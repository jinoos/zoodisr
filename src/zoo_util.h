#ifndef _ZOO_UTIL_H_
#define _ZOO_UTIL_H_

#include "zoodisr.h"

// zookeeper connection infomation
struct z_conn_info
{
    sds uri;
    int with_auth;
    sds user;
    sds pass;
    sds hosts;
    sds node;

    zhandle_t   *zh;
    int         zstat;

    pthread_t   zthread;
    int         ztid;
};

void z_set_log_level(int level);
struct z_conn_info* z_parse_conn_string(const char *str);

void z_server_connect();
void z_server_watcher(zhandle_t *zh, int type, int state, const char *path, void *data);

void z_svc_connect(struct svc *svc);
void z_svc_read_set_ketama(struct svc *svc, const struct String_vector *childs);
void z_svc_read_children(int rc, const struct String_vector *childs, const void *data);
void z_svc_watcher(zhandle_t *zh, int type, int state, const char *path, void *data);

void z_print_state(int ll, const int state);
void z_print_type(int ll, const int type);

#endif // _ZOO_UTIL_H_
