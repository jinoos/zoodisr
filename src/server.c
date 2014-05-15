#include "server.h"

struct server server;

void server_init_default()
{
    server.name = NULL;
    server.bind_addr_count = 0;

    server.port = 0;

    // unix domain socket path
    server.req_log_uds = NULL;

    server.client_max = 0;
    server.tcp_backlog = 0;
    server.log_level = LOG_DEFAULT;
    server.log_file = NULL;

    server.z_health = 0;
    server.z_health_conn = NULL;
    server.zoo_timeout = 0;

    server.svc_count = 0;

    rcounter_init(&server.counter);
    cmd_dict_init();
    client_dict_init();
    rlclient_list_alloc();

}

void* server_zoo_run(void *data)
{

    while(1)
    {
        sleep(1);
    }

    return NULL;
}

void* svc_zoo_run(void *data)
{
    struct svc *svc = (struct svc*) data;

    log_info("Starting SVC [%s] zookeeper watcher..", svc->name);

    z_svc_connect(svc);

    while(1)
    {
        sleep(1);
    }
    log_err("svc_end");

    return NULL;
}


void server_run_prepare()
{
    int i;

    for(i = 0; i < server.svc_count; i++)
    {
        struct svc *svc = server.svc_arr[i];
        z_svc_connect(svc);
    }

    // test code
    log_info("Starting server zookeeper watcher..");
    z_server_connect();

    return;

}

void server_evbase_init()
{
    server.evconf = event_config_new();
    event_config_avoid_method(server.evconf, "select");
    event_config_avoid_method(server.evconf, "poll");
    server.evbase = event_base_new_with_config(server.evconf);
}

void server_binding()
{
    char *err = NULL;
    int i;
    for(i = 0; i < server.bind_addr_count; i++)
    {
        struct server_bind *bind = &server.bind_arr[i];
        bind->addr.sin_port = htons(server.port);
        bind->listener = evconnlistener_new_bind(server.evbase,
                                                server_accept_client,
                                                (void*) bind,
                                                LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_CLOSE_ON_EXEC,
                                                server.tcp_backlog,
                                                (struct sockaddr*) &bind->addr,
                                                sizeof(struct sockaddr_in));
        
        if(!bind->listener)
        {
            err = sdscatprintf(sdsempty(), "Cann't create listener with %s", bind->addr_str);
            goto binderr;
        }

        log_msg("Listening for client [%s:%d]", bind->addr_str, server.port);
    }

    return;
binderr:
    log_eerr("NETWORK ERROR.");
    log_eerr(">> %s", err);
    sdsfree(err);
    exit(1);
}

void server_rl_binding()
{
    char *err = NULL;

    if(!server.req_log_uds)
        return;

    unlink(server.req_log_uds);
    struct server_bind_rl *bind = &server.bind_rl;
    bind->addr.sun_family = AF_UNIX;
    strcpy(bind->addr.sun_path, server.req_log_uds);
    bind->listener = evconnlistener_new_bind(server.evbase,
                                                server_accept_log_client,
                                                (void*) bind,
                                                LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_CLOSE_ON_EXEC,
                                                -1,
                                                (struct sockaddr*) &bind->addr,
                                                sizeof(struct sockaddr_un));
    if(!bind->listener)
    {
        err = sdscatprintf(sdsempty(), "Cann't create request log domain socket on %s", server.req_log_uds);
        goto binderr;
    }

    log_msg("Listening for request log [%s]", server.req_log_uds);

    return;
binderr:
    log_eerr("ERROR.");
    log_eerr("%s", err);
    sdsfree(err);
    exit(1);
}

void server_accept_client(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr)
{
    int port = ntohs(((struct sockaddr_in*)addr)->sin_port);
    sds ip = sdsnew(inet_ntoa(((struct sockaddr_in*)addr)->sin_addr));

    server.client_connected++;

    log_info("Accepted client socket from %s:%d, current[%d], max[%d], total[%"PRIu64"]", ip, port, server.client_current + 1, server.client_max, server.client_connected);
    if(server.client_current >= server.client_max)
    {
        ssize_t wl;
        log_warn("Reached max connection: %d, client will be closed.", server.client_current + 1);
        sds msg = sdscatprintf(sdsempty(), "%s 1%sMax connection error.", ZR_CMD_REP_ERR, ZR_MSG_NL);
        wl = write(sock, msg, sdslen(msg));
        sdsfree(msg);
        close(sock);
        return;
    }


    //struct client *c = client_alloc(sock, (struct sockaddr_in*)addr);
    client_alloc(sock, (struct sockaddr_in*)addr);
}

void server_accept_log_client(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr)
{
    int nlen = len - offsetof(struct sockaddr_un, sun_path);
    struct sockaddr_un caddr;
    caddr.sun_path[nlen] = '\0';

    server.log_client_connected++;

    log_info("Accepted log client, current[%d] max[%d] total[%"PRIu64"]", server.rlc_list->count + 1, ZR_MAX_LOG_CONN, server.log_client_connected);
    if(server.rlc_list->count >= ZR_MAX_LOG_CONN)
    {
        ssize_t wl;
        log_warn("Reached max log client connection: %d, current client will be closed.", server.rlc_list->count + 1);
        sds msg = sdscatprintf(sdsempty(), "%s 1%sMax log client connection error. [%d]", ZR_CMD_REP_ERR, ZR_MSG_NL, server.rlc_list->count);
        wl = write(sock, msg, sdslen(msg));
        sdsfree(msg);
        close(sock);
        return;
    }

    rlclient_alloc(sock, &caddr);

}

void server_run()
{
    event_base_dispatch(server.evbase);
}
