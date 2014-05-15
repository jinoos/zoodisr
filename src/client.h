#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "zoodisr.h"

#define CLIENT_WBUF_LEN    1024*64

struct client
{
    sds                     name;
    evutil_socket_t         sock;
    struct sockaddr_in      addr;
    sds                     ip;
    int                     port;
    struct bufferevent      *bufev;
    struct evbuffer         *bufin;
    struct evbuffer         *bufout;

    struct rcounter         counter;

    struct msg              *msg;

    time_t                  c_time;

    char                    wbuf[CLIENT_WBUF_LEN];
};

void client_dict_init();
int client_dict_add(sds name, struct client *c);
struct client* client_dict_get(sds name);
int client_dict_size();
void client_dict_rm(sds name);

struct client* client_alloc();
void client_ready_sock(struct client*c, evutil_socket_t sock, struct sockaddr_in *addr);
void client_free(struct client *c);
void client_close(struct client *c);
void client_event_error(struct bufferevent *bufev, short events, void *arg);
void client_event_read(struct bufferevent *bufev, void *arg);
void client_event_write(struct bufferevent *bufev, void *arg);
void client_write_cmd(struct client *c, struct msg *msg);
int client_execute_cmd(struct client *c);
void client_readline(struct client *c, char *line, size_t line_len);

#endif // _CLIENT_H_
