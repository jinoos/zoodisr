#include <regex.h>
#include "zoo_util.h"

#define REGEX_ZOO_CONN_STR              "^(zoo://)([0-9\\.,:]+)(/.*)$"
#define REGEX_ZOO_CONN_STR_WU           "^(zoo://)([^:]+)([:]*)([^@]*)(@)([0-9\\.,:]+)(/.*)$"
#define REGEX_ZOO_SVC_NAME              "^([^:]+)(:)([0-9]+)$"

#define REGEX_ZOO_CONN_STR_NMATCH       4
#define REGEX_ZOO_CONN_STR_WU_NMATCH    8
#define REGEX_ZOO_SVC_NAME_NMATCH       4

static int regex_compiled = 0;
static regex_t regex_zoo_conn_string;
static regex_t regex_zoo_conn_string_wu;
static regex_t regex_zoo_svc_name;

void z_set_log_level(int level)
{
    switch(level)
    {
        case LOG_DEBUG:
            zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
        case LOG_VERBOSE:
            zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
            break;
        case LOG_INFO:
            zoo_set_debug_level(ZOO_LOG_LEVEL_INFO);
            break;
        case LOG_WARN:
            zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
            break;
        case LOG_ERR:
            zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
            break;
        default:
            zoo_set_debug_level(ZOO_LOG_LEVEL_INFO);
            break;
    }
}


void z_regex_compile()
{
    int res;

    res = regcomp(&regex_zoo_conn_string, REGEX_ZOO_CONN_STR, REG_EXTENDED);
    if(res != 0)
    {
        log_err("Cannot compile pattern %s", REGEX_ZOO_CONN_STR);
        exit(1);
    }

    res = regcomp(&regex_zoo_conn_string_wu, REGEX_ZOO_CONN_STR_WU, REG_EXTENDED);
    if(res != 0)
    {
        log_err("Cannot compile pattern %s", REGEX_ZOO_CONN_STR_WU);
        exit(1);
    }

    res = regcomp(&regex_zoo_svc_name, REGEX_ZOO_SVC_NAME, REG_EXTENDED);

    regex_compiled = 1;
}


struct z_conn_info* z_parse_conn_string(const char *str)
{
    if(regex_compiled == 0)
        z_regex_compile();

    int res;
    sds uri, user, pass, hosts, node;

    regmatch_t pmatch[REGEX_ZOO_CONN_STR_NMATCH];
    regmatch_t pmatch_wu[REGEX_ZOO_CONN_STR_WU_NMATCH];

    struct z_conn_info *cinfo = calloc(sizeof(struct z_conn_info), 1);
    cinfo->with_auth = 0;
    cinfo->user = NULL;
    cinfo->pass = NULL;
    cinfo->zh = NULL;
    cinfo->zstat = ZOO_CONNECTING_STATE;
    cinfo->ztid = 0;

    // check with connection string with user info pattern first, 
    // if not matched with user info, then try to check no user info pattern.
    res = regexec(&regex_zoo_conn_string_wu, str, REGEX_ZOO_CONN_STR_WU_NMATCH, pmatch_wu, 0);

    if(res == 0)
    {
        uri = sdsnewlen(&str, strlen(str));
        user = sdsnewlen(&str[pmatch_wu[2].rm_so], (pmatch_wu[2].rm_eo - pmatch_wu[2].rm_so));
        pass = sdsnewlen(&str[pmatch_wu[4].rm_so], (pmatch_wu[4].rm_eo - pmatch_wu[4].rm_so));
        hosts = sdsnewlen(&str[pmatch_wu[6].rm_so], (pmatch_wu[6].rm_eo - pmatch_wu[6].rm_so));
        node = sdsnewlen(&str[pmatch_wu[7].rm_so], (pmatch_wu[7].rm_eo - pmatch_wu[7].rm_so));

        struct z_conn_info *cinfo = calloc(sizeof(struct z_conn_info), 1);
        cinfo->uri = uri;
        cinfo->with_auth = 1;
        cinfo->user = user;
        cinfo->pass = pass;
        cinfo->hosts = hosts;
        cinfo->node = node;

        return cinfo;
    }

    res = regexec(&regex_zoo_conn_string, str, REGEX_ZOO_CONN_STR_NMATCH, pmatch, 0);

    if(res == 0)
    {
        uri = sdsnewlen(&str, strlen(str));
        hosts = sdsnewlen(&str[pmatch[2].rm_so], (pmatch[2].rm_eo - pmatch[2].rm_so));
        node = sdsnewlen(&str[pmatch[3].rm_so], (pmatch[3].rm_eo - pmatch[3].rm_so));

        cinfo->uri = uri;
        cinfo->hosts = hosts;
        cinfo->node = node;

        return cinfo;
    }

    free(cinfo);
    return NULL;
}

void z_server_connect()
{
    struct z_conn_info *cinfo = server.z_health_conn;
    cinfo->zh = zookeeper_init(cinfo->hosts, z_server_watcher, server.zoo_timeout, NULL, &server, 0); 
}

void z_server_watcher(zhandle_t *zh, int type, int state, const char *path, void *data)
{
    struct server *s = (struct server*) data;

    if(!s->z_health)
        return;

    struct z_conn_info *cinfo = s->z_health_conn;

    int res;

    log_debug("ZOO: Caught Server event, connection status has been changed. type:%d state:%d path:%s", type, state, path);
    if(log_enabled_debug())
    {
        z_print_type(LOG_DEBUG, type);
        z_print_state(LOG_DEBUG, state);
    }

    // Related connection.
    if(type == ZOO_SESSION_EVENT)
    {
        cinfo->zstat = state;
        if(state == ZOO_CONNECTED_STATE)
        {
            if(cinfo->with_auth)
            {
                sds test = sdscatprintf(sdsempty(), "%s:%s", server.name, server.name);
                res = zoo_add_auth(cinfo->zh, "digest", test, sdslen(cinfo->pass), 0, 0);
                sdsfree(test);
            }

            sds ndata = sdsempty();
            int i;
            for(i = 0; i < s->bind_addr_count; i++)
            {
                ndata = sdscatprintf(ndata, "%s:%d\n", s->bind_arr[i].addr_str,s->port);
            }
            ndata = sdstrim(ndata, "\n");
            log_info("ZOO: Server health check connected");

            res = zoo_create(cinfo->zh, s->z_health_node, ndata, sdslen(ndata), &ZOO_READ_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
            if(res != ZOK)
            {
                if(res == ZNODEEXISTS)
                {
                    log_info("ZOO: Node already [%s], does not create it.", s->z_health_node);
                }else
                {
                    log_err("ZOO: %s [%s]", zerror(res), s->z_health_node);
                }
            }
            sdsfree(ndata);
        }else if(state == ZOO_EXPIRED_SESSION_STATE)
        {
            log_err("ZOO: Server health check session expired, try to reconnect after 5 seconds...");
            zookeeper_close(cinfo->zh);
            sleep(5);
            z_server_connect();
            //cinfo->zh = zookeeper_init(cinfo->hosts, z_server_watcher, server.zoo_timeout, NULL, s, 0);
        }else if(state == ZOO_AUTH_FAILED_STATE)
        {
            log_err("ZOO: Server health check Auth failed, check your scheme [%s], try to reconnect after 5 seconds...", cinfo->user);
            zookeeper_close(cinfo->zh);
            sleep(5);
            z_server_connect();
            //cinfo->zh = zookeeper_init(cinfo->hosts, z_server_watcher, server.zoo_timeout, NULL, s, 0);
        }
    }
}

void z_svc_connect(struct svc *svc)
{
    struct z_conn_info *cinfo = svc->z_conn;
    cinfo->zh = zookeeper_init(cinfo->hosts, z_svc_watcher, server.zoo_timeout, NULL, svc, 0); 
}

void z_svc_read_set_ketama(struct svc *svc, const struct String_vector *childs)
{
    int res, i, buf_len, ncnt = 0;
    char buf[100];
    Ketama *k = Ketama_new();
    struct node *node_hdr = NULL, *node_lst = NULL;

    for(i = 0; i < childs->count; i++)
    {
        sds ip, tsds;
        int port, weight;
        char *node = childs->data[i];

        regmatch_t pmatch[REGEX_ZOO_SVC_NAME_NMATCH];

        res = regexec(&regex_zoo_svc_name, node, REGEX_ZOO_SVC_NAME_NMATCH, pmatch, 0);

        if(res != 0)
        {
            log_err("ZOO: SVC node name is not available, [%s/%s]", svc->z_conn->node, childs->data[i]);
            continue;
        }else
        {
            ip = sdsnewlen(&node[pmatch[1].rm_so], (pmatch[1].rm_eo - pmatch[1].rm_so));
            tsds = sdsnewlen(&node[pmatch[3].rm_so], (pmatch[3].rm_eo - pmatch[3].rm_so));
            port = atoi(tsds);
            sdsfree(tsds);
        }

        sds child = sdscatprintf(sdsnew(svc->z_conn->node), "/%s", childs->data[i]);

        res = zoo_get(svc->z_conn->zh, child, 0, buf, &buf_len, NULL);
        if(res != ZOK)
        {
            log_err("ZOO: Cannot read SVC node [%s], %s", child, zerror(res));
            sdsfree(child);
            continue;
        }
        log_debug("ZOO: Read node [%s] data [%.*s]", child, buf_len, buf);
        sdsfree(child);

        tsds = sdsnewlen(buf, buf_len);
        weight = atoi(sdstrim(tsds, " \n\r\t"));
        sdsfree(tsds);

        log_debug("ZOO: SVC node [%s:%d] weight [%d]", ip, port, weight);

        // first node
        if(!node_hdr)
        {
            node_hdr = node_lst = node_alloc(ip, port, weight);
        }else
        {
            node_lst->next = node_alloc(ip, port, weight);
            node_lst = node_lst->next;
        }

        Ketama_add_server(k, ip, port, weight);

        ncnt++;
    }

    Ketama *old_k = svc->khash;
    struct node *old_n = svc->node;

    if(ncnt == 0)
    {
        log_warn("ZOO: No available SVC childres in [%s]", svc->z_conn->node);
        if(svc->khash)
        {
            svc->khash = NULL;
            svc->node = NULL;
            svc->node_count = 0;
            Ketama_free(k);
        }
    }else
    {
        log_info("ZOO: Create ketama countinum for [%d] nodes", ncnt);
        Ketama_create_continuum(k);
        svc->khash = k;
        svc->node = node_hdr;
        svc->node_count = node_count(node_hdr);
    }

    log_msg("SVC [%s] has [%d] available nodes.", svc->name, ncnt);

    struct node *tnode = node_hdr;
    while(tnode != NULL)
    {
        log_debug("SVC [%s] node %s:%d w:%d", svc->name, tnode->ip, tnode->port, tnode->weight);
        tnode = tnode->next;
    }

    usleep(10);
    if(old_k) Ketama_free(old_k);
    if(old_n) node_free_all(old_n);

    return;
}

void z_svc_read_children(int rc, const struct String_vector *childs, const void *data)
{
    struct svc *svc = (struct svc*) data;
    struct z_conn_info *cinfo = svc->z_conn;

    log_debug("ZOO: SVC [%s] reading childrens.", cinfo->node);

    if(regex_compiled == 0) z_regex_compile();

    switch(rc)
    {
        case ZOK:
            z_svc_read_set_ketama(svc, childs);
            break;
        default:
            log_err("ZOO: SVC [%s] reading children: %s", zerror(rc));
    }

    return;
}

void z_svc_watcher(zhandle_t *zh, int type, int state, const char *path, void *data)
{
    struct svc *svc = (struct svc*) data;
    struct z_conn_info *cinfo = svc->z_conn;

    int res;

    log_debug("ZOO: Caught SVC event, connection status has been changed. type:%d state:%d path:%s", type, state, path);
    if(log_enabled_debug())
    {
        z_print_type(LOG_DEBUG, type);
        z_print_state(LOG_DEBUG, state);
    }

    // Related connection.
    if(type == ZOO_SESSION_EVENT)
    {
        cinfo->zstat = state;
        if(state == ZOO_CONNECTED_STATE)
        {
            if(cinfo->with_auth)
            {
                sds test = sdscatprintf(sdsempty(), "%s:%s", server.name, server.name);
                res = zoo_add_auth(cinfo->zh, "digest", test, sdslen(cinfo->pass), 0, 0);
                sdsfree(test);
            }

            log_info("ZOO: SVC [%s] connected.", svc->name);

            res = zoo_awget_children(cinfo->zh, cinfo->node, z_svc_watcher, svc, z_svc_read_children, svc);

            if(res != ZOK)
            {
                log_err("ZOO: Failed SVC node [%s] childres reading. %s", cinfo->node, zerror(res));
            }

        }else if(state == ZOO_EXPIRED_SESSION_STATE)
        {
            log_err("ZOO: SVC watcher session expired, try to reconnect after 5 seconds...");
            zookeeper_close(cinfo->zh);
            sleep(5);
            z_svc_connect(svc);
        }else if(state == ZOO_AUTH_FAILED_STATE)
        {
            log_err("ZOO: SVC watcher Auth failed, check your scheme [%s], try to reconnect after 5 seconds...", cinfo->user);
            zookeeper_close(cinfo->zh);
            sleep(5);
            z_svc_connect(svc);
        }
    
    // Related with SVC node changes
    }else if(type == ZOO_CHILD_EVENT)
    {
        log_msg("ZOO: SVC[%s]  node has been changed.", svc->name);
        res = zoo_awget_children(cinfo->zh, cinfo->node, z_svc_watcher, svc, z_svc_read_children, svc);
        if(res != ZOK)
        {
            log_err("ZOO: Failed SVC node [%s] childres reading. %s", cinfo->node, zerror(res));
        }
    }

    return;
}

void z_print_state(int ll, const int state)
{
    if(state == ZOO_EXPIRED_SESSION_STATE)
        log_print(ll, "ZOO: Status ZOO_EXPIRED_SESSION_STATE");
    else if(state == ZOO_AUTH_FAILED_STATE)
        log_print(ll, "ZOO: Status ZOO_AUTH_FAILED_STATE");
    else if(state == ZOO_CONNECTING_STATE)
        log_print(ll, "ZOO: Status ZOO_CONNECTING_STATE");
    else if(state == ZOO_ASSOCIATING_STATE)
        log_print(ll, "ZOO: Status ZOO_ASSOCIATING_STATE");
    else if(state == ZOO_CONNECTED_STATE)
        log_print(ll, "ZOO: Status ZOO_CONNECTED_STATE");
    else
        log_print(ll, "ZOO: Status unknown: %d", state);

    return;
}

void z_print_type(int ll, const int type)
{
    if(type == ZOO_CREATED_EVENT)
        log_print(ll, "ZOO: Type ZOO_CREATED_EVENT");
    else if(type == ZOO_DELETED_EVENT)
        log_print(ll, "ZOO: Type ZOO_DELETED_EVENT");
    else if(type == ZOO_CHANGED_EVENT)
        log_print(ll, "ZOO: Type ZOO_CHANGED_EVENT");
    else if(type == ZOO_CHILD_EVENT)
        log_print(ll, "ZOO: Type ZOO_CHILD_EVENT");
    else if(type == ZOO_SESSION_EVENT)
        log_print(ll, "ZOO: Type ZOO_SESSION_EVENT");
    else if(type == ZOO_NOTWATCHING_EVENT)
        log_print(ll, "ZOO: Type ZOO_NOTWATCHING_EVENT");
    else
        log_print(ll, "ZOO: Type unknown: %d", type);

    return;
}

