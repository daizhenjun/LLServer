#include "config.h"
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

#include "llserver.h"
#include "llthread.h"

//下面代码部分引用自memcached中多线程lievent实例启动代码
/* listening socket */
static int l_socket = 0;

struct event_base *main_base;

static int new_socket(const bool is_udp) {
    int sfd;
    int flags;

    if ((sfd = socket(AF_INET, is_udp ? SOCK_DGRAM : SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        return -1;
    }

    if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("setting O_NONBLOCK");
        close(sfd);
        return -1;
    }
    return sfd;
}


static int server_socket(const int port, const bool is_udp) {
    int sfd;
    struct linger ling = {0, 0};
    struct sockaddr_in addr;
    int flags =1;

    if ((sfd = new_socket(is_udp)) == -1) {
        return -1;
    }

    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    if (is_udp) {
        ;//maximize_sndbuf(sfd);
    } else {
        setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
        setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
        setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    }

    /*
     * the memset call clears nonstandard fields in some impementations
     * that otherwise mess things up.
     */
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = settings.interf;
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind()");
        close(sfd);
        return -1;
    }
    if (!is_udp && listen(sfd, 1024) == -1) {
        perror("listen()");
        close(sfd);
        return -1;
    }
    return sfd;
}


void start_mc_server()
{
	/* create the listening socket and bind it */
    if (settings.socketpath == NULL) {
        l_socket = server_socket(settings.port, 0);
        if (l_socket == -1) {
            fprintf(stderr, "failed to listen\n");
            exit(EXIT_FAILURE);
        }
    }

	/* initialize main thread libevent instance */
    main_base = event_init();
	/* create the initial listening connection */
    /*if (!(listen_conn = conn_new(l_socket, conn_listening,
                                 EV_READ | EV_PERSIST, 1, false, main_base))) {
        fprintf(stderr, "failed to create listening connection");
        exit(EXIT_FAILURE);
    }*/

	struct event ev;
	event_set(&ev, l_socket, EV_READ | EV_PERSIST, event_handler, (void *)&l_socket);	//函数创建新的事件结构
    event_base_set(main_base, &ev);
	//event_add使通过event_set()设置的事件在事件匹配或超时时(如果设置了超时)被执行
	if (event_add(&ev, 0) == -1) {
		//close(l_socket);
	}

	thread_init(settings.num_threads, main_base);/* llthread.c */
    /* enter the event loop */
    event_base_loop(main_base, 0);    
}
