#include "rlclient.h"

static char write_buf[ZR_REQ_LOG_BUF_LEN];

struct rlclient_list* rlclient_list_alloc()
{
    struct rlclient_list *rlist = malloc(sizeof(struct rlclient_list));
    rlist->count = 0;
    rlist->first = rlist->last = NULL;
    server.rlc_list = rlist;
    return rlist;
}

struct rlclient* rlclient_alloc(evutil_socket_t sock, struct sockaddr_un *addr)
{
    struct rlclient_list *l = server.rlc_list;
    struct rlclient *r = malloc(sizeof(struct rlclient));
    memcpy(&r->addr, addr, sizeof(struct sockaddr_un));
    r->next = r->prev = NULL;
    r->sock = sock;

    r->bufev = bufferevent_socket_new(server.evbase, sock, BEV_OPT_CLOSE_ON_FREE);
    r->bufin = bufferevent_get_input(r->bufev);
    r->bufout = bufferevent_get_output(r->bufev);

    bufferevent_setcb(r->bufev, rlclient_event_read, NULL, rlclient_event_error, r);
    bufferevent_enable(r->bufev, EV_READ);

    if(l->count == 0)
    {
        l->first = l->last = r;
    }else
    {
        l->last->next = r;
        r->prev = l->last;
        l->last = r;
    }

    l->count++;

    return r;
}

void rlclient_close(struct rlclient *r)
{
    close(r->sock);
}

void rlclient_free(struct rlclient *r)
{
    struct rlclient_list *l = server.rlc_list;
    bufferevent_free(r->bufev);
    if(l->count == 1)
    {
        l->first = l->last = NULL;
    }else
    {
        if(r->next)
        {
            r->next->prev = r->prev;
        }else
        {
            l->last = r->prev;
            r->prev->next = NULL;
        }

        if(r->prev)
        {
            r->prev->next = r->next;
        }else
        {
            l->first = r->next;
            r->next->prev = NULL;
        }
    }
    l->count--;
    free(r);
}

void rlclient_event_error(struct bufferevent *bufev, short events, void *arg)
{
    struct rlclient *r = (struct rlclient*) arg;
    log_debug("RL: error event. socket[%d]", r->sock);

    if (events & BEV_EVENT_EOF)
    {
        log_info("RL: EOF recieved, closed clinet socket [%d]", r->sock);
        rlclient_close(r);
        rlclient_free(r);

    } else if (events & BEV_EVENT_ERROR)
    {
        log_info("RL: received error, %s(%d)", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), EVUTIL_SOCKET_ERROR());
        rlclient_close(r);
        rlclient_free(r);

    } else if (events & BEV_EVENT_TIMEOUT)
    {
        log_debug("RL: error is BEV_EVENT_TIMEOUT");
    } else if (events & BEV_EVENT_CONNECTED)
    {
        log_debug("RL: error is BEV_EVENT_CONNECTED");
    }else if (events & BEV_EVENT_READING)
    {
        log_debug("RL: error is BEV_EVENT_READING");
    } else if (events & BEV_EVENT_WRITING)
    {
        log_debug("RL: error is BEV_EVENT_WRITING");
    }
}

int rlclient_enabled()
{
    return server.rlc_list->count;
}

int rlclient_write(struct client *c, const char *fmt, ...)
{
    int len;
    char *bufp = write_buf;
    struct rlclient_list *l = server.rlc_list;

    if(l->count == 0)
        return 0;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);


    len = snprintf(bufp, ZR_REQ_LOG_BUF_LEN, "%d-%02d-%02d %02d:%02d:%02d %s ",
                                            (tm->tm_year) + 1900, 
                                            (tm->tm_mon) + 1,
                                            tm->tm_mday,
                                            tm->tm_hour,
                                            tm->tm_min,
                                            tm->tm_sec,
                                            c->ip);

    bufp += len;

    va_list ap;
    va_start(ap, fmt);
    len = vsnprintf(bufp, ZR_REQ_LOG_BUF_LEN-(bufp-write_buf), fmt, ap);
    va_end(ap);

    bufp += len;

    len = snprintf(bufp, ZR_REQ_LOG_BUF_LEN-(bufp-write_buf), "%s", "\n");
    bufp[len] = 0;

    struct rlclient *r = l->first;

    while(r)
    {
        send(r->sock, write_buf, strlen(write_buf), 0);
        r = r->next;
    }

    return l->count;
}

void rlclient_event_read(struct bufferevent *bufev, void *arg)
{
    struct rlclient *r = (struct rlclient*) arg;
    char *line;
    size_t line_len;

    while((line = evbuffer_readln(r->bufin, &line_len, EVBUFFER_EOL_CRLF)) != NULL)
    {
        free(line);
    }
}
