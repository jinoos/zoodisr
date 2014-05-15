#ifndef _ZOODIS_ROUTER_H_
#define _ZOODIS_ROUTER_H_

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <libgen.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <zookeeper/zookeeper.h>

#define ZR_MAX_SVC_COUNT        INT8_MAX
#define ZR_MAX_SVC_NAME         INT8_MAX-1

#define ZR_MAX_BIND_ADDR        8
#define ZR_MAX_HOSTNAME_LEN     INT8_MAX-1

#define ZR_MAX_LOG_CONN         5

#define ZR_PORT                 7758
#define ZR_CONN_MAX             1000*10
#define ZR_TCP_BACKLOG          500
#define ZR_RR_COUNTER_BUFFER    10

#define ZR_ZOO_TIMEOUT          5000
#define ZR_DEFAULT_REQ_LOG_UDS  "/tmp/zoodis-req-log"

struct svc;
struct rcounter;

#include "countque.h"
#include "rcounter.h"
#include "sds.h"
#include "zoo_util.h"

#include "logging.h"
#include "dict.h"
#include "ketama.h"
#include "svc.h"
#include "server.h"
#include "conf.h"
#include "event_util.h"
#include "node.h"
#include "client.h"
#include "msg.h"
#include "commands.h"
#include "err_msg.h"
#include "rlclient.h"
#include "utime.h"

extern struct server server;

//
// functions
//
#endif // _ZOODIS_ROUTER_H_
