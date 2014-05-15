#include <malloc.h>

#include "client.h"

static unsigned int client_hash(const void *key)
{
    return dictGenHashFunction((unsigned char*)key, sdslen((sds)key));
}

static int client_key_compare(void *privdata, const void *key1, const void *key2)
{   
    DICT_NOTUSED(privdata);
    return strcmp(key1, key2) == 0;
}

static dictType client_dict_type = {
    client_hash,            /* hash function */
    NULL,                   /* key dup */
    NULL,                   /* val dup */
    client_key_compare,     /* key compare */
    NULL,                   /* key destructor */
    NULL                    /* val destructor */
};

void client_dict_init()
{
    server.client_dict = dictCreate(&client_dict_type, NULL);
}

int client_dict_add(sds name, struct client *c)
{
    int res = dictAdd(server.client_dict, name, c);

    if(res != DICT_OK)
        log_err("Client: cannot added client [%s] into dictionary.", name);
    else
        log_debug("Client: added client[%s] into dictionary.", name);

    return res;

}

void client_dict_rm(sds name)
{
    dictDelete(server.client_dict, name);
}

int client_dict_size()
{
    return dictSize(server.client_dict);
}

struct client* client_dict_get(sds name)
{
    dictEntry *de = dictFind(server.client_dict, name);
    if(!de)
        return NULL;

    return (struct client*)dictGetVal(de);
}

struct client* client_alloc(evutil_socket_t sock, struct sockaddr_in *addr)
{
    struct client *c = malloc(sizeof(struct client));
    rcounter_init(&c->counter);

    memcpy(&c->addr, addr, sizeof(struct sockaddr_in));

//    evutil_make_socket_nonblocking(sock);
    c->sock = sock;
    c->ip = sdsnew(inet_ntoa(addr->sin_addr));
    c->port = ntohs(addr->sin_port);
    c->name = sdscatprintf(sdsempty(), "%s:%d", c->ip, c->port);
    c->c_time = time(NULL);
    c->msg = msg_alloc();

    client_dict_add(c->name, c);

    //c->bufev = bufferevent_socket_new(server.evbase, sock, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    c->bufev = bufferevent_socket_new(server.evbase, sock, BEV_OPT_CLOSE_ON_FREE);
    c->bufin = bufferevent_get_input(c->bufev);
    c->bufout = bufferevent_get_output(c->bufev);

    //bufferevent_setcb(c->bufev, client_event_read, client_event_write, client_event_error, c);
    bufferevent_setcb(c->bufev, client_event_read, NULL, client_event_error, c);
    bufferevent_enable(c->bufev, EV_READ);

    server.client_current++;

    return c;
}

void client_close(struct client *c)
{
    close(c->sock);
}

void client_free(struct client *c)
{
    log_info("Client: removing client %s:%d", c->ip, c->port);
    client_dict_rm(c->name);
    bufferevent_free(c->bufev);
    rcounter_destory(&c->counter);
    sdsfree(c->ip);
    sdsfree(c->name);
    msg_free(c->msg);
    free(c);
    server.client_current--;
    log_info("Client: current connected cliends %d", server.client_current);
    malloc_trim(0);
}

void client_event_error(struct bufferevent *bufev, short events, void *arg)
{
    struct client *c = (struct client*) arg;
    log_debug("Event: error from %s:%d", c->ip, c->port);

    if (events & BEV_EVENT_EOF)
    {
        log_info("Event: EOF recieved, closed clinet socket %s:%d", c->ip, c->port);
        client_close(c);
        client_free(c);

    } else if (events & BEV_EVENT_ERROR)
    {
        log_debug("- Error is BEV_EVENT_ERROR");
        log_info("Event: received error from socket, %s(%d)", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), EVUTIL_SOCKET_ERROR());
        client_close(c);
        client_free(c);

    } else if (events & BEV_EVENT_TIMEOUT)
    {
        log_debug("- Error is BEV_EVENT_TIMEOUT");
    } else if (events & BEV_EVENT_CONNECTED)
    {
        log_debug("- Error is BEV_EVENT_CONNECTED");
    }else if (events & BEV_EVENT_READING)
    {
        log_debug("- Error is BEV_EVENT_READING");
    } else if (events & BEV_EVENT_WRITING)
    {
        log_debug("- Error is BEV_EVENT_WRITING");
    }

}

extern int _msg_row_data_alloc;

void client_event_read(struct bufferevent *bufev, void *arg)
{   
    struct client *c = (struct client*) arg;
    log_debug("Event: read from %s:%d", c->ip, c->port);

    char *line;
    size_t line_len;
    
    while((line = evbuffer_readln(c->bufin, &line_len, EVBUFFER_EOL_CRLF)) != NULL)
    {
        log_debug("Message [%.*s]", line_len, line);
        client_readline(c, line, line_len);
    }
}

void client_event_write(struct bufferevent *bufev, void *arg)
{   
    struct client *c = (struct client*) arg;
    log_debug("Event: write from %s:%d", c->ip, c->port);
}

void client_write_cmd(struct client *c, struct msg *msg)
{
    if(!msg || !msg->first)
        return;

    struct msg_row *row = msg->first;

    char *wbp = c->wbuf;

    while(row)
    {
        evbuffer_add(c->bufout, row->data, row->len);
        if(!row->inc_nl)
        {
            evbuffer_add(c->bufout, ZR_MSG_NL, ZR_MSG_NL_LEN);
        }
        row = row->next;
    }
    evbuffer_add(c->bufout, ZR_MSG_NL, ZR_MSG_NL_LEN);
    evbuffer_write(c->bufout, c->sock);

    return;
}

int client_execute_cmd(struct client *c)
{
    if(log_enabled_debug()) msg_print(LOG_DEBUG, c->msg);
    enum command_result res;
    struct msg *rep = NULL;
    struct msg_cmd *mc = NULL;
    struct command *cmd = NULL;
    struct msg *msg = c->msg;

    if(!msg_validate(msg))
    {
        rep = msg_init_error(ZR_ERR_INVALID_CMD_NO, ZR_ERR_INVALID_CMD);
        goto freemsg;
    }

    mc = msg_get_cmd(msg);
    if(!mc)
    {
        rep = msg_init_error(ZR_ERR_INVALID_CMD_NO, ZR_ERR_INVALID_CMD);
        goto freemsg;
    }


    sds cmd_name = sdsnewlen(mc->name, mc->len);
    cmd = cmd_get(cmd_name);
    sdsfree(cmd_name);
    if(!cmd)
    {
        rep = msg_init_error(ZR_ERR_UNKNOWN_CMD_NO, ZR_ERR_UNKNOWN_CMD);
        goto freemsg;
    }

    if(cmd->status == ZR_CMD_INACT)
    {
        rep = msg_init_error(ZR_ERR_INVALID_CMD_FORMAT_NO, ZR_ERR_INVALID_CMD_FORMAT);
        goto freemsg;
    }

    if(cmd->min_argc > msg->argc || cmd->max_argc < msg->argc)
    {
        rep = msg_init_error(ZR_ERR_INVALID_ARGUMENT_CNT_NO, ZR_ERR_INVALID_ARGUMENT_CNT);
        goto freemsg;
    }

    res = cmd->func(cmd, c, msg, NULL);

    if(res == ZR_CMD_SUCCESS)
    {
        rcounter_tic_ok(&c->counter);
        rcounter_tic_ok(&server.counter);
        log_debug("Client: %s command success.", cmd->name);
    }else
    {
        rcounter_tic_err(&c->counter);
        rcounter_tic_err(&server.counter);
        log_warn("Client: %s command faild.", cmd->name);
    }

    msg_trunk(c->msg);
    //if(((rcounter_ok(&server.counter)+rcounter_err(&server.counter))%1000) == 0)
    //    malloc_trim(0);

    return 1;

freemsg:
    client_write_cmd(c, rep);
    rcounter_tic_err(&c->counter);
    rcounter_tic_err(&c->counter);
    rcounter_tic_err(&server.counter);
    msg_free(rep);
    msg_trunk(c->msg);
    return 1;
}

void client_readline(struct client *c, char *line, size_t line_len)
{
    if(line_len == 0)
    {
        if(c->msg)
        {
            client_execute_cmd(c);
        }

        // for last empty line
        free(line);
        return;
    }

    if(!c->msg->argc)
        c->msg->t = utime_time();

    msg_append(c->msg, line, line_len);
}

