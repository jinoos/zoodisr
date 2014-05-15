#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "zoodisr.h"

#define ZR_CMD_REQ_PREFIX           '+'
#define ZR_CMD_REP_PREFIX           '-'

#define ZR_CMD_REP_ERR              "-ERROR"
#define ZR_CMD_REP_PONG             "-PONG"
#define ZR_CMD_REP_STATS            "-Stats"
#define ZR_CMD_REP_SERVICESTATS     "-ServiceStats"
#define ZR_CMD_REP_SERVICELIST      "-ServiceList"
#define ZR_CMD_REP_NODELIST         "-NodeList"
#define ZR_CMD_REP_NODE             "-Node"
#define ZR_CMD_REP_NODESTATS        "-NodeStats"

enum command_type
{
    ZR_CMD_REQ,
    ZR_CMD_REP
};

enum command_status
{
    ZR_CMD_ACT,
    ZR_CMD_INACT
};

enum command_result
{
    ZR_CMD_SUCCESS,
    ZR_CMD_FAILURE,
};

void cmd_dict_init();
struct command* cmd_get(const char *cmd);

struct client;

struct command
{
    char                *name;
    enum command_type   type;
    enum command_status status;
    uint16_t            min_argc;
    uint16_t            max_argc;
    enum command_result (*func)(struct command *cmd, struct client *client, struct msg *msg, void *data);
};

#define ZR_CMD_FUNC(x)      enum command_result zr_cmd_func_##x(struct command *cmd, struct client *client, struct msg *msg, void *data)
#define ZR_CMD_FUNC_PTR(x)  &zr_cmd_func_##x

#define ZR_CMD_REQ_INIT(F,st,mn,mx)     struct command cmd##F = {"+"#F, ZR_CMD_REQ, st, mn, mx, ZR_CMD_FUNC_PTR(F)}
#define ZR_CMD_REP_INIT(F,st,mn,mx)     struct command cmd##F = {"-"#F, ZR_CMD_REP, st, mn, mx, NULL}
#define ZR_CMD_VAR_EXTERN(F)            extern struct command cmd##F
#define ZR_CMD_VAR(F)                   (cmd##F)

// request command func
ZR_CMD_FUNC(Ping);
ZR_CMD_FUNC(GetStats);
ZR_CMD_FUNC(GetServiceStats);
ZR_CMD_FUNC(GetServiceList);
ZR_CMD_FUNC(GetActiveServiceList);
ZR_CMD_FUNC(GetNode);
ZR_CMD_FUNC(GetClientStats);

// @todo
// not use
ZR_CMD_FUNC(GetNodeStats);
ZR_CMD_FUNC(GetNodeList);
ZR_CMD_FUNC(GetActiveNodeList);

ZR_CMD_VAR_EXTERN(Ping);
ZR_CMD_VAR_EXTERN(GetStats);
ZR_CMD_VAR_EXTERN(GetServiceStats);
ZR_CMD_VAR_EXTERN(GetServiceList);
ZR_CMD_VAR_EXTERN(GetActiveServiceList);
ZR_CMD_VAR_EXTERN(GetNode);
ZR_CMD_VAR_EXTERN(GetNodeList);
ZR_CMD_VAR_EXTERN(GetActiveNodeList);;
ZR_CMD_VAR_EXTERN(GetNodeStats);
ZR_CMD_VAR_EXTERN(Error);
ZR_CMD_VAR_EXTERN(Pong);
ZR_CMD_VAR_EXTERN(Stats);
ZR_CMD_VAR_EXTERN(ServiceStats);
ZR_CMD_VAR_EXTERN(ServiceList);
ZR_CMD_VAR_EXTERN(NodeList);
ZR_CMD_VAR_EXTERN(Node);
ZR_CMD_VAR_EXTERN(NodeStats);
ZR_CMD_VAR_EXTERN(ClientStats);

struct msg* cmd_zookeeper_state(struct msg *m, const char *prefix, struct z_conn_info *z_info);
struct msg* cmd_rcounter_state(struct msg *m, const char *prefix, struct rcounter *rc);

#endif // _COMMANDS_H_
