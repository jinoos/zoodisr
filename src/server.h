#ifndef _SERVER_H_
#define _SERVER_H_

#include "zoodisr.h"

struct server_bind
{
    sds                     addr_str;
    struct sockaddr_in      addr;
    struct evconnlistener   *listener;
};

struct server_bind_rl
{
    struct sockaddr_un      addr;
    struct evconnlistener   *listener;
};

struct server
{
    sds                 name;

    int                 port;

    sds                 req_log_uds;

    int                 client_max;
    int                 tcp_backlog;

    int                 log_level;
    sds                 log_file;

    int                 z_health;
    struct z_conn_info  *z_health_conn;
    sds                 z_health_node;
    int                 zoo_timeout;

    // service bucket
    uint8_t             svc_count;
    struct svc*         svc_arr[ZR_MAX_SVC_COUNT];

    //
    // event
    //
    struct event_base       *evbase;
    struct event_config     *evconf;

    // Server socket and event values
    int                 bind_addr_count;
    struct server_bind  bind_arr[ZR_MAX_BIND_ADDR];

    // Request log socket and event values
    struct server_bind_rl  bind_rl;

    // request counter
    struct rcounter     counter;

    // client connections
    int                 client_current;
    uint64_t            client_connected;

    struct rlclient_list    *rlc_list;
    uint64_t            log_client_connected;

    // for commands
    dict                *cmd_dict;
    dict                *client_dict;

};

void server_init_default();
void* server_zoo_run(void *data);
void* svc_zoo_run(void *data);
void server_run_prepare();
void server_evbase_init();
void server_binding();
void server_rl_binding();
void server_accept_client(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr);
void server_accept_log_client(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr);
void server_run();

#endif // _SERVER_H_
