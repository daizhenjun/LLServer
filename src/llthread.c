#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/signal.h>
#include <sys/uio.h>
#include <pwd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

#include <string.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/queue.h>

#include "config.h"
#include "utils.h"
#include "llserver.h"
#include "llthread.h"

//下面代码部分引用自memcached中多线程lievent实例
#define ITEMS_PER_ALLOC 64

/* An item in the connection queue. */
typedef struct conn_queue_item CQ_ITEM;
struct conn_queue_item {
    int     sfd;
    int     init_state;
    int     event_flags;
    int     read_buffer_size;
    int     is_udp;
    CQ_ITEM *next;
};

/* A connection queue. */
typedef struct conn_queue CQ;
struct conn_queue {
    CQ_ITEM *head;
    CQ_ITEM *tail;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
};

/* Lock for connection freelist */
static pthread_mutex_t conn_lock;
/* Lock for global stats */
static pthread_mutex_t stats_lock;
/* Free list of CQ_ITEM structs */
static CQ_ITEM *cqi_freelist;
static pthread_mutex_t cqi_freelist_lock;

/*
 * Each libevent instance has a wakeup pipe, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
typedef struct {
    pthread_t thread_id;        /* unique ID of this thread */
    struct event_base *base;    /* libevent handle this thread uses */
    struct event notify_event;  /* listen event for notify pipe */
    int notify_receive_fd;      /* receiving end of notify pipe */
    int notify_send_fd;         /* sending end of notify pipe */
    CQ  new_conn_queue;         /* queue of new connections to handle */
} LIBEVENT_THREAD;

static LIBEVENT_THREAD *threads;

/*
 * Number of threads that have finished setting themselves up.
 */
static int init_count = 0;
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;

/*
 * Initializes a connection queue.
 */
static void cq_init(CQ *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    pthread_cond_init(&cq->cond, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/*
 * Waits for work on a connection queue.
 */
static CQ_ITEM *cq_pop(CQ *cq) {
    CQ_ITEM *item;

    pthread_mutex_lock(&cq->lock);
    while (NULL == cq->head)
        pthread_cond_wait(&cq->cond, &cq->lock);
    item = cq->head;
    cq->head = item->next;
    if (NULL == cq->head)
        cq->tail = NULL;
    pthread_mutex_unlock(&cq->lock);

    return item;
}

/*
 * Looks for an item on a connection queue, but doesn't block if there isn't
 * one.
 * Returns the item, or NULL if no item is available
 */
static CQ_ITEM *cq_peek(CQ *cq) {
    CQ_ITEM *item;

    pthread_mutex_lock(&cq->lock);
    item = cq->head;
    if (NULL != item) {
        cq->head = item->next;
        if (NULL == cq->head)
            cq->tail = NULL;
    }
    pthread_mutex_unlock(&cq->lock);

    return item;
}

/*
 * Adds an item to a connection queue.
 */
static void cq_push(CQ *cq, CQ_ITEM *item) {
    item->next = NULL;

    pthread_mutex_lock(&cq->lock);
    if (NULL == cq->tail)
        cq->head = item;
    else
        cq->tail->next = item;
    cq->tail = item;
    pthread_cond_signal(&cq->cond);
    pthread_mutex_unlock(&cq->lock);
}

/*
 * Returns a fresh connection queue item.
 */
static CQ_ITEM *cqi_new() {
    CQ_ITEM *item = NULL;
    pthread_mutex_lock(&cqi_freelist_lock);
    if (cqi_freelist) {
        item = cqi_freelist;
        cqi_freelist = item->next;
    }
    pthread_mutex_unlock(&cqi_freelist_lock);

    if (NULL == item) {
        int i;

        /* Allocate a bunch of items at once to reduce fragmentation */
        item =(CQ_ITEM*) malloc(sizeof(CQ_ITEM) * ITEMS_PER_ALLOC);
        if (NULL == item)
            return NULL;

        /*
         * Link together all the new items except the first one
         * (which we'll return to the caller) for placement on
         * the freelist.
         */
        for (i = 2; i < ITEMS_PER_ALLOC; i++)
            item[i - 1].next = &item[i];

        pthread_mutex_lock(&cqi_freelist_lock);
        item[ITEMS_PER_ALLOC - 1].next = cqi_freelist;
        cqi_freelist = &item[1];
        pthread_mutex_unlock(&cqi_freelist_lock);
    }

    return item;
}
//
//
/*
 * Frees a connection queue item (adds it to the freelist.)
 */
static void cqi_free(CQ_ITEM *item) {
    pthread_mutex_lock(&cqi_freelist_lock);
    item->next = cqi_freelist;
    cqi_freelist = item;
    pthread_mutex_unlock(&cqi_freelist_lock);
}

/* Which thread we assigned a connection to most recently. */
static int last_thread = -1;

void dispatch_conn_new(int sfd, /*int init_state,*/ int event_flags,
                       int read_buffer_size, int is_udp) {
    CQ_ITEM *item = cqi_new();
    int thread = (last_thread + 1) % settings.num_threads;

    last_thread = thread;

    item->sfd = sfd;
    //item->init_state = init_state;
    item->event_flags = event_flags;
    item->read_buffer_size = read_buffer_size;
    item->is_udp = is_udp;

    cq_push(&threads[thread].new_conn_queue, item);
	//MEMCACHED_CONN_DISPATCH(sfd, threads[thread].thread_id);  
    if (write(threads[thread].notify_send_fd, "", 1) != 1) {
        perror("Writing to thread notify pipe");
    }
}

void event_handler(const int fd, const short which, void *arg) {
    int *e;
    e = (int *)arg;
    //assert(e != NULL);

    /* Accept a new connection. */
    socklen_t addrlen;
    struct sockaddr addr;
    int sfd;

	if ((sfd = accept(fd, &addr, &addrlen)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* these are transient, so don't log anything */
            close(fd);
        } else if (errno == EMFILE) {
            if (settings.verbose > 0)
                fprintf(stderr, "Too many open connections\n");
            close(fd);
        } else {
            perror("accept()");
            close(fd);
        }
        return;
    }
	//解析数据
    int flags = 1;
	if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
                fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
                perror("setting O_NONBLOCK");
                close(sfd);
                return; 
    }
    //调度(指派)新的链接
    dispatch_conn_new(sfd, /*conn_read,*/ EV_READ | EV_PERSIST, 2048, false);
}

/*
 * Processes an incoming "handle a new connection" item. This is called when
 * input arrives on the libevent wakeup pipe.
 */
static void thread_libevent_process(int fd, short which, void *arg) {
    LIBEVENT_THREAD *me = (LIBEVENT_THREAD *)arg;
    CQ_ITEM *item;
    char buf[1];
    //从注册pipe读端获取信息
    if (read(fd, buf, 1) != 1)
        if (settings.verbose > 0)
            fprintf(stderr, "Can't read from libevent pipe\n");

    item = cq_peek(&me->new_conn_queue);

    if (NULL != item) {
        ev_connect *evc =(ev_connect*) malloc(sizeof(ev_connect));
        if (evc == NULL){
           perror("malloc readclient failed");
		   return;
	    }
	    evc->fd = item->sfd;
		evc->base = me->base;
		evc->buf_ev = bufferevent_socket_new(me->base, item->sfd, 0);	
		bufferevent_setcb(evc->buf_ev, llserver_read_handler, llserver_write_handler, llserver_error_handler, evc);
        bufferevent_enable(evc->buf_ev, EV_READ);
	    bufferevent_setwatermark(evc->buf_ev, EV_READ, 0/*low*/, BUFFER_SIZE/*high*/);
		//bufferevent_set_timeouts(evc->bufev, &tv, &tv);    
    
		struct bufferq *bufferque;
	    bufferque = (bufferq*)calloc(1, sizeof(bufferq));
	    if (bufferque == NULL)
            err(1, "calloc faild");
        bufferque->needbyte = bufferque->readbyte = 0;
        bufferque->state = READ_LINE;
	    /* 初始化客户端写队列。 */
        TAILQ_INIT(&evc->readq);
        TAILQ_INSERT_TAIL(&evc->readq, bufferque, entries);

        cqi_free(item);
	}
}


/*
 * Set up a thread's information.
 */
static void setup_thread(LIBEVENT_THREAD *me) {
    if (! me->base) {
        me->base = event_init();
        if (! me->base) {
            fprintf(stderr, "Can't allocate event base\n");
            exit(1);
        }
    }

    /* Listen for notifications from other threads */
    event_set(&me->notify_event, me->notify_receive_fd,
              EV_READ | EV_PERSIST, thread_libevent_process, me);
    event_base_set(me->base, &me->notify_event);

    if (event_add(&me->notify_event, 0) == -1) {
        fprintf(stderr, "Can't monitor libevent notify pipe\n");
        exit(1);
    }

    cq_init(&me->new_conn_queue);
}

static void *worker_libevent(void *arg) {
    LIBEVENT_THREAD *me = (LIBEVENT_THREAD *) arg;

    /* Any per-thread setup can happen here; thread_init() will block until
     * all threads have finished initializing.
     */

    pthread_mutex_lock(&init_lock);
    init_count++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

    return (void*) event_base_loop(me->base, 0);
}

/*
 * Creates a worker thread.
 */
static void create_worker(void *(*func)(void *), void *arg) {
    pthread_t       thread;
    pthread_attr_t  attr;
    int             ret;

    pthread_attr_init(&attr);

    if ((ret = pthread_create(&thread, &attr, func, arg)) != 0) {
        fprintf(stderr, "Can't create thread: %s\n",
                strerror(ret));
        exit(1);
    }
}

/*
 * Initializes the thread subsystem, creating various worker threads.
 *
 * nthreads  Number of event handler threads to spawn
 * main_base Event base for main thread
 */
void thread_init(int nthreads, struct event_base *main_base) {
    int         i;

    //pthread_mutex_init(&conn_lock, NULL);
    //pthread_mutex_init(&stats_lock, NULL);

    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

    pthread_mutex_init(&cqi_freelist_lock, NULL);
    cqi_freelist = NULL;

    threads = (LIBEVENT_THREAD *) malloc(sizeof(LIBEVENT_THREAD) * nthreads);
    if (! threads) {
        perror("Can't allocate thread descriptors");
        exit(1);
    }

    threads[0].base = main_base;
    threads[0].thread_id = pthread_self();

    for (i = 0; i < nthreads; i++) {
        int fds[2];
        if (pipe(fds)) {
            perror("Can't create notify pipe");
            exit(1);
        }

        threads[i].notify_receive_fd = fds[0];
        threads[i].notify_send_fd = fds[1];

        setup_thread(&threads[i]);
    }

    /* Create threads after we've done all the libevent setup. */
    for (i = 1; i < nthreads; i++) {
        create_worker(worker_libevent, &threads[i]);
    }

    /* Wait for all the threads to set themselves up before returning. */
    pthread_mutex_lock(&init_lock);
    init_count++; // main thread
    while (init_count < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);
}

//struct event_base *main_base;
