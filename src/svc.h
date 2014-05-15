#ifndef _SVC_H_
#define _SVC_H_

#include <stdint.h>

#include "zoodisr.h"

struct svc
{
    //
    struct z_conn_info  *z_conn;

    // service name what is configured in conf
    sds                 name;
    int8_t              key;

    // number of seconds of recent request counter
    struct rcounter     counter;

    Ketama              *khash;

    // node(single linked list)
    int                 node_count;
    struct node         *node;
};

struct svc* svc_alloc(const char *name, size_t name_len, int8_t key);

#endif // _SVC_H_

