#ifndef _RLCLIENT_H_
#define _RLCLIENT_H_

#include "zoodisr.h"

#define ZR_REQ_LOG_BUF_LEN  4096

extern struct server server;

struct rlclient
{
    evutil_socket_t     sock;
    struct sockaddr_un  addr;

    struct bufferevent  *bufev;
    struct evbuffer     *bufin;
    struct evbuffer     *bufout;
    
    struct rlclient     *prev;
    struct rlclient     *next;
};

struct rlclient_list
{
    int                 count;
    struct rlclient     *first;
    struct rlclient     *last;
};

struct rlclient_list* rlclient_list_alloc();
struct rlclient* rlclient_alloc(evutil_socket_t sock, struct sockaddr_un *addr);
void rlclient_close(struct rlclient *r);
void rlclient_free(struct rlclient *r);
void rlclient_event_error(struct bufferevent *bufev, short events, void *arg);
int rlclient_write(struct client *c, const char *str, ...);
void rlclient_event_read(struct bufferevent *bufev, void *arg);
int rlclient_enabled();


#endif // _RLCLIENT_H_


