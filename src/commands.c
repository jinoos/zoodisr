#include "commands.h"

extern struct server server;

//
// Request command
//
ZR_CMD_REQ_INIT(Ping,                   ZR_CMD_ACT,      1,1);
ZR_CMD_REQ_INIT(GetStats,               ZR_CMD_ACT,      1,1);
ZR_CMD_REQ_INIT(GetServiceStats,        ZR_CMD_ACT,      2,2);
ZR_CMD_REQ_INIT(GetServiceList,         ZR_CMD_ACT,      1,1);
ZR_CMD_REQ_INIT(GetActiveServiceList,   ZR_CMD_ACT,      1,1);
ZR_CMD_REQ_INIT(GetNode,                ZR_CMD_ACT,      3,3);
ZR_CMD_REQ_INIT(GetNodeList,            ZR_CMD_INACT,    2,2);
ZR_CMD_REQ_INIT(GetActiveNodeList,      ZR_CMD_INACT,    2,2);
ZR_CMD_REQ_INIT(GetNodeStats,           ZR_CMD_INACT,    3,3);
ZR_CMD_REQ_INIT(GetClientStats,         ZR_CMD_ACT,      1,1);

//
// Response command
//
ZR_CMD_REP_INIT(Error,                  ZR_CMD_ACT,     3,3);
ZR_CMD_REP_INIT(Pong,                   ZR_CMD_ACT,     1,1);
ZR_CMD_REP_INIT(Stats,                  ZR_CMD_ACT,     1,UINT16_MAX);
ZR_CMD_REP_INIT(ServiceStats,           ZR_CMD_ACT,     1,UINT16_MAX);
ZR_CMD_REP_INIT(ServiceList,            ZR_CMD_ACT,     1,UINT16_MAX);
ZR_CMD_REP_INIT(NodeList,               ZR_CMD_ACT,     1,UINT16_MAX);
ZR_CMD_REP_INIT(Node,                   ZR_CMD_ACT,     2,2);
ZR_CMD_REP_INIT(NodeStats,              ZR_CMD_ACT,     1,UINT16_MAX);
ZR_CMD_REP_INIT(ClientStats,            ZR_CMD_ACT,     1,UINT16_MAX);

static unsigned int cmd_case_hash(const void *key)
{
    return dictGenCaseHashFunction((unsigned char*)key, strlen((char*)key));
}

static int cmd_key_case_compare(void *privdata, const void *key1, const void *key2)
{   
    DICT_NOTUSED(privdata);
    return strcasecmp(key1, key2) == 0;
}

static dictType cmd_dict_type = {
    cmd_case_hash,          /* hash function */
    NULL,           /* key dup */
    NULL,                   /* val dup */
    cmd_key_case_compare,   /* key compare */
    NULL,                   /* key destructor */
    NULL                    /* val destructor */
};

void cmd_dict_init()
{
    if(server.cmd_dict != NULL)
        return;

    server.cmd_dict = dictCreate(&cmd_dict_type, NULL);
    dictAdd(server.cmd_dict, ZR_CMD_VAR(Ping).name, &ZR_CMD_VAR(Ping));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetStats).name, &ZR_CMD_VAR(GetStats));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetServiceStats).name, &ZR_CMD_VAR(GetServiceStats));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetServiceList).name, &ZR_CMD_VAR(GetServiceList));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetActiveServiceList).name, &ZR_CMD_VAR(GetActiveServiceList));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetNode).name, &ZR_CMD_VAR(GetNode));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetNodeList).name, &ZR_CMD_VAR(GetNodeList));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetActiveNodeList).name, &ZR_CMD_VAR(GetActiveNodeList));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(GetNodeStats).name, &ZR_CMD_VAR(GetNodeStats));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(Error).name, &ZR_CMD_VAR(Error));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(Pong).name, &ZR_CMD_VAR(Pong));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(Stats).name, &ZR_CMD_VAR(Stats));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(ServiceStats).name, &ZR_CMD_VAR(ServiceStats));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(ServiceList).name, &ZR_CMD_VAR(ServiceList));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(NodeList).name, &ZR_CMD_VAR(NodeList));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(Node).name, &ZR_CMD_VAR(Node));
    dictAdd(server.cmd_dict, ZR_CMD_VAR(NodeStats).name, &ZR_CMD_VAR(NodeStats));
}

struct command* cmd_get(const char *cmd)
{
    struct dictEntry *de;
    de = dictFind(server.cmd_dict, cmd);
    if(de == NULL)
        return NULL;

    return dictGetVal(de);
}

//
// request command func
//
ZR_CMD_FUNC(Ping)
{
    struct msg *rmsg = msg_init_pong();
    client_write_cmd(client, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s", t - msg->t, cmd->name);
    }

    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetStats)
{
    int i;
    sds rs;

    struct msg *rmsg = msg_alloc_with_line(ZR_CMD_VAR(Stats).name, strlen(ZR_CMD_VAR(Stats).name));
    msg_append_printf(rmsg, "name: %s", server.name);
    msg_append_printf(rmsg, "port: %d", server.port);

    rs = sdscat(sdsempty(), "bind:");
    for(i = 0; i < server.bind_addr_count; i++)
    {
        rs = sdscatprintf(rs, " %s", server.bind_arr[i].addr_str);
    }
    msg_append_dup(rmsg, rs, sdslen(rs));
    sdsfree(rs);

    msg_append_printf(rmsg, "client-max: %d", server.client_max);
    msg_append_printf(rmsg, "client-current: %d", server.client_current);
    msg_append_printf(rmsg, "client-connected: %"PRIu64, server.client_connected);
    msg_append_printf(rmsg, "log-client-max: %d", ZR_MAX_LOG_CONN);
    msg_append_printf(rmsg, "log-client-current: %d", server.rlc_list->count);
    msg_append_printf(rmsg, "log-client-connected: %"PRIu64, server.log_client_connected);
    msg_append_printf(rmsg, "tcp-backlog: %d", server.tcp_backlog);
    msg_append_printf(rmsg, "zookeeper-health: %d", server.z_health);

    if(server.z_health) rmsg = cmd_zookeeper_state(rmsg, "zookeeper-health", server.z_health_conn);

    msg_append_printf(rmsg,"zookeeper-timeout: %d", server.zoo_timeout);
    msg_append_printf(rmsg,"service-count: %d", server.svc_count);
    msg_append_printf(rmsg,"request-log-uds: %s", server.req_log_uds);
    rmsg = cmd_rcounter_state(rmsg, "requests", &server.counter);
    {
        int secs = time(NULL) - server.counter.uptime;
        int h = secs / 3600;
        int m = (secs % 3600) / 60;
        int s = (secs % 3600) % 60;
        msg_append_printf(rmsg,"uptime: %dh %dm %ds", h, m, s);
    }
    client_write_cmd(client, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s", t - msg->t, cmd->name);
    }

    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetServiceStats)
{
    int i;
    sds rs;

    struct msg *rmsg = msg_alloc_with_line(ZR_CMD_VAR(ServiceStats).name, strlen(ZR_CMD_VAR(ServiceStats).name));
    msg_append_printf(rmsg, "name: %s", server.name);
    msg_append_printf(rmsg, "port: %d", server.port);

    rs = sdscat(sdsempty(), "bind:");
    for(i = 0; i < server.bind_addr_count; i++)
    {
        rs = sdscatprintf(rs, " %s", server.bind_arr[i].addr_str);
    }
    msg_append_dup(rmsg, rs, sdslen(rs));
    sdsfree(rs);

    msg_append_printf(rmsg, "client-max: %d", server.client_max);
    msg_append_printf(rmsg, "client-current: %d", server.client_current);
    msg_append_printf(rmsg, "client-connected: %"PRIu64, server.client_connected);
    msg_append_printf(rmsg, "log-client-max: %d", ZR_MAX_LOG_CONN);
    msg_append_printf(rmsg, "log-client-current: %d", server.rlc_list->count);
    msg_append_printf(rmsg, "log-client-connected: %"PRIu64, server.log_client_connected);
    msg_append_printf(rmsg, "tcp-backlog: %d", server.tcp_backlog);
    msg_append_printf(rmsg, "zookeeper-health: %d", server.z_health);

    if(server.z_health) rmsg = cmd_zookeeper_state(rmsg, "zookeeper-health", server.z_health_conn);

    msg_append_printf(rmsg,"zookeeper-timeout: %d", server.zoo_timeout);
    msg_append_printf(rmsg,"service-count: %d", server.svc_count);
    msg_append_printf(rmsg,"request-log-uds: %s", server.req_log_uds);
    msg_append_printf(rmsg,"request-log-uds: %s", server.req_log_uds);
    rmsg = cmd_rcounter_state(rmsg, "requests", &server.counter);
    {
        int secs = time(NULL) - server.counter.uptime;
        int h = secs / 3600;
        int m = (secs % 3600) / 60;
        int s = (secs % 3600) % 60;
        msg_append_printf(rmsg,"uptime: %dh %dm %ds", h, m, s);
    }
    client_write_cmd(client, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s", t - msg->t, cmd->name);
    }

    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetServiceList)
{
    int i;
    struct msg *rmsg = msg_alloc_with_line(ZR_CMD_VAR(ServiceList).name, strlen(ZR_CMD_VAR(ServiceList).name));

    for(i = 0; i < server.svc_count; i++)
    {
        struct svc *s = server.svc_arr[i];
        msg_append_printf(rmsg, "%s: %d", s->name, i);
    }
    client_write_cmd(client, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s SVC_COUNT %d", t - msg->t, cmd->name, server.svc_count);
    }

    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetActiveServiceList)
{
    int i, cnt = 0;
    struct msg *rmsg = msg_alloc_with_line(ZR_CMD_VAR(ServiceList).name, strlen(ZR_CMD_VAR(ServiceList).name));

    for(i = 0; i < server.svc_count; i++)
    {
        struct svc *s = server.svc_arr[i];
        if(s->node_count <= 0)
            continue;

        msg_append_printf(rmsg, "%s: %d", s->name, i);
        cnt++;
    }
    client_write_cmd(client, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s SVC_COUNT %d", t - msg->t, cmd->name, cnt);
    }

    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetNode)
{
    struct msg *rmsg = NULL;
    struct msg_row *svc_row, *key_row;
    struct svc *svc;
    int svc_no, key_len;

    svc_row = msg_argv(msg, 1);
    key_row = msg_argv(msg, 2);

    key_len = key_row->inc_nl ? key_row->len-2 : key_row->len;

    svc_no = strtol(svc_row->data, NULL, 0);

    if(errno == ERANGE)
    {
        rmsg = msg_init_error(ZR_ERR_SVC_OUTOFBOUND_NO, ZR_ERR_SVC_OUTOFBOUND);
        client_write_cmd(client, rmsg);
        msg_free(rmsg);

        // request log
        if(rlclient_enabled())
        {
            uint64_t t = utime_time();
            rlclient_write(client, "%"PRIu64"us ERR %s SVC_OUTOFBOUND SVC:%s KEY:%.*s", t - msg->t, cmd->name, svc_row->data, svc_no, key_len, key_row->data);
        }

        return ZR_CMD_FAILURE;
    }

    if(svc_no < 0 || svc_no >= server.svc_count)
    {
        rmsg = msg_init_error(ZR_ERR_WRONG_SVC_NO_NO, ZR_ERR_WRONG_SVC_NO);
        client_write_cmd(client, rmsg);
        msg_free(rmsg);

        // request log
        if(rlclient_enabled())
        {
            uint64_t t = utime_time();
            rlclient_write(client, "%"PRIu64"us ERR %s WRONG_SVC_NO SVC:%d KEY:%.*s", t - msg->t, cmd->name, svc_no, key_len, key_row->data);
        }

        return ZR_CMD_FAILURE;
    }

    svc = server.svc_arr[svc_no];
    rmsg = msg_alloc_with_line(ZR_CMD_VAR(Node).name, strlen(ZR_CMD_VAR(Node).name));

    if(svc->node_count == 0)
    {
        client_write_cmd(client, rmsg);
        msg_free(rmsg);

        // request log
        if(rlclient_enabled())
        {
            uint64_t t = utime_time();
            rlclient_write(client, "%"PRIu64"us OK %s ZERO_NODE SVC:%s(%d) KEY:%.*s", t - msg->t, cmd->name, svc->name, svc_no, key_len, key_row->data);
        }

        return ZR_CMD_SUCCESS;
    }

    int ordinal = Ketama_get_server_ordinal(svc->khash, key_row->data, key_len);
    char *node_name = Ketama_get_server_address(svc->khash, ordinal);
    msg_append_dup(rmsg, node_name, strlen(node_name));
    client_write_cmd(client, rmsg);
    msg_print(LOG_DEBUG, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s SVC:%s(%d) KEY:%.*s NODE:%s", t - msg->t, cmd->name, svc->name, svc_no, key_len, key_row->data, node_name);
    }

    return ZR_CMD_SUCCESS;
}


ZR_CMD_FUNC(GetClientStats)
{
    int i;
    sds rs;

    struct msg *rmsg = msg_alloc_with_line(ZR_CMD_VAR(Stats).name, strlen(ZR_CMD_VAR(Stats).name));
    msg_append_printf(rmsg, "name: %s", server.name);
    msg_append_printf(rmsg, "port: %d", server.port);

    rs = sdscat(sdsempty(), "bind:");
    for(i = 0; i < server.bind_addr_count; i++)
    {
        rs = sdscatprintf(rs, " %s", server.bind_arr[i].addr_str);
    }
    msg_append_dup(rmsg, rs, sdslen(rs));
    sdsfree(rs);

    msg_append_printf(rmsg, "client-max: %d", server.client_max);
    msg_append_printf(rmsg, "client-current: %d", server.client_current);
    msg_append_printf(rmsg, "client-connected: %"PRIu64, server.client_connected);
    msg_append_printf(rmsg, "log-client-max: %d", ZR_MAX_LOG_CONN);
    msg_append_printf(rmsg, "log-client-current: %d", server.rlc_list->count);
    msg_append_printf(rmsg, "log-client-connected: %"PRIu64, server.log_client_connected);
    msg_append_printf(rmsg, "tcp-backlog: %d", server.tcp_backlog);
    msg_append_printf(rmsg, "zookeeper-health: %d", server.z_health);

    if(server.z_health) rmsg = cmd_zookeeper_state(rmsg, "zookeeper-health", server.z_health_conn);

    msg_append_printf(rmsg,"zookeeper-timeout: %d", server.zoo_timeout);
    msg_append_printf(rmsg,"service-count: %d", server.svc_count);
    msg_append_printf(rmsg,"request-log-uds: %s", server.req_log_uds);
    msg_append_printf(rmsg,"request-log-uds: %s", server.req_log_uds);
    rmsg = cmd_rcounter_state(rmsg, "requests", &server.counter);
    {
        int secs = time(NULL) - server.counter.uptime;
        int h = secs / 3600;
        int m = (secs % 3600) / 60;
        int s = (secs % 3600) % 60;
        msg_append_printf(rmsg,"uptime: %dh %dm %ds", h, m, s);
    }
    client_write_cmd(client, rmsg);
    msg_free(rmsg);

    // request log
    if(rlclient_enabled())
    {
        uint64_t t = utime_time();
        rlclient_write(client, "%"PRIu64"us OK %s", t - msg->t, cmd->name);
    }

    return ZR_CMD_SUCCESS;
}


// not use
// @todo
ZR_CMD_FUNC(GetNodeStats)
{
    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetNodeList)
{
    return ZR_CMD_SUCCESS;
}

ZR_CMD_FUNC(GetActiveNodeList)
{
    return ZR_CMD_SUCCESS;
}


struct msg* cmd_zookeeper_state(struct msg *m, const char *prefix, struct z_conn_info *z_info)
{
    if(z_info->with_auth)
        msg_append_printf(m, "%s-uri: zoo://%s:****@%s%s", prefix, z_info->user, z_info->hosts, z_info->node);
    else
        msg_append_printf(m, "%s-uri: zoo://%s%s", prefix, z_info->hosts, z_info->node);

    if(!z_info->zh)
    {
        msg_append_printf(m, "%s-state: not connected.", prefix);

    }else
    {
        int stat = zoo_state(z_info->zh);

        if(stat == ZOO_CONNECTED_STATE)
            msg_append_printf(m, "%s-state: connected."ZR_MSG_NL, prefix);
        else if(stat == ZOO_EXPIRED_SESSION_STATE)
            msg_append_printf(m, "%s-state: session expired."ZR_MSG_NL, prefix);
        else if(stat == ZOO_AUTH_FAILED_STATE)
            msg_append_printf(m, "%s-state: auth failed."ZR_MSG_NL, prefix);
        else if(stat == ZOO_CONNECTING_STATE)
            msg_append_printf(m, "%s-state: connecting."ZR_MSG_NL, prefix);
        else if(stat == ZOO_ASSOCIATING_STATE)
            msg_append_printf(m, "%s-state: associating."ZR_MSG_NL, prefix);
        else
            msg_append_printf(m, "%s-state: unknown state."ZR_MSG_NL, prefix);
    }

    return m;
}

struct msg* cmd_rcounter_state(struct msg *m, const char *prefix, struct rcounter *rc)
{
    msg_append_printf(m, "%s: %"PRIu64, prefix, rc->ok + rc->err);
    msg_append_printf(m, "%s-ok: %"PRIu64, prefix, rc->ok);
    msg_append_printf(m, "%s-err: %"PRIu64, prefix, rc->err);
    msg_append_printf(m, "%s-qps: %.1f", prefix, (double)(cq_count(&rc->rok) + cq_count(&rc->rerr))/(uint64_t)cq_tic_max_count(&rc->rok));
    msg_append_printf(m, "%s-qps-ok: %.1f", prefix, (double)cq_count(&rc->rok)/(uint64_t)cq_tic_max_count(&rc->rok));
    msg_append_printf(m, "%s-qps->err: %.1f", prefix, (double)cq_count(&rc->rerr)/(uint64_t)cq_tic_max_count(&rc->rok));
    msg_append_printf(m, "%s-qps-seconds: %d", prefix, cq_tic_max_count(&rc->rok));

    return m;
}
