#include "config.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <iostream>

#include <err.h>
#include <event.h>
#include <evhttp.h>

#include "leveldb/db.h"
#include "utils.h"

#define BUFFER_SIZE 4096

//this settings reference to memcached settings
struct settings {
    size_t maxbytes;
    int maxconns;
    int port;
    int udpport;
    struct in_addr interf;
    int verbose;
    bool managed;          /* if 1, a tracker manages virtual buckets */
    int evict_to_free;
    char *socketpath;   /* path to unix socket if using local socket */
    double factor;          /* chunk size growth factor */
    int chunk_size;
    int num_threads;        /* number of libevent threads to run */
    char prefix_delimiter;  /* character that marks a key prefix (for stats) */
    int detail_enabled;     /* nonzero if we're collecting detailed stats */
};


/* A record info. */
typedef struct iteminfo iteminfo;
struct iteminfo {
	    char *key;
    	unsigned int flags;
		time_t exptime;
		int data_length;	
		char *data;
		char *errorinf;
};

/*state: ALL_DATA_READ*/
enum message_read_status {
	READ_LINE = 3,
	ALL_DATA_READ = 2,
	MORE_DATA_EXPECTED = 1,
	DATA_CORRUPTED = -1,
	DATA_TOO_LONG = -2
};
typedef struct bufferq buffer_q;
struct bufferq {
	/*当前数据信息*/
    iteminfo item;
    /*需要读取字节*/
    int needbyte;
    /*已读取字节*/
    int readbyte;
	message_read_status state;
    /* 链表结构。 */
    TAILQ_ENTRY(bufferq) entries;
};

typedef struct evconnect ev_connect;
struct evconnect {
  int fd;
  struct bufferevent *buf_ev;
  TAILQ_HEAD(, bufferq) readq;
  struct event_base *base;    /* libevent handle this thread uses */
};


static leveldb::DB* lldb;

static leveldb::Options lloptions;

extern char *llserver_settings_dataname;

/* 缓存数据,默认为100mb*/
extern int llserver_settings_cache;

extern struct settings settings;

extern struct event_base *main_base;

void start_mc_server();

leveldb::Status opendb();

leveldb::Status put(const iteminfo item);

const iteminfo get(const char *key);

leveldb::Status clear(const char *key);

leveldb::Status clearall();

void llserver_http_handler(struct evhttp_request *req, void *arg);

void llserver_read_handler(struct bufferevent *bev, void *arg);

void llserver_write_handler(struct bufferevent *bev, void *arg);

void llserver_error_handler(struct bufferevent *bev, short what, void *arg);

