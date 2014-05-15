#ifndef _MSG_H_
#define _MSG_H_

#include "zoodisr.h"

#define ZR_MSG_NL                   "\r\n"
#define ZR_MSG_NL_LEN               2
#define ZR_MSG_DEF_CMD_BUFFER       64  
#define ZR_MSG_MAX_CMD_LEN          (ZR_MSG_DEF_CMD_BUFFER-3-10)

#define ZR_MSG_LAST_ERR_BUF_LEN     1024
#define ZR_MSG_STR_BUF_LEN          1024*1024

struct msg_row
{
    char            *data;
    size_t          len;
    int             inc_nl;
    struct msg_row  *next;
};

struct msg_cmd
{
    char            *name;
    size_t          len;
};

struct msg
{
    int             argc;
    struct msg_row  *first;
    struct msg_row  *last;

    int             is_valid;
    int             validated;
    struct msg_cmd  cmd;
    uint64_t        t;
};

void msg_set_last_error_empty();
char* msg_set_last_error(const char *err);
char* msg_last_error();

// @return
//      1 : include newline chars (\r\n)
//      0 : not include newline chars in end of the string.
int msg_check_newline(const char *str, size_t len);

struct msg* msg_alloc();
struct msg* msg_alloc_with_line();
struct msg_row* msg_append(struct msg *msg, char *line, size_t len);
struct msg_row* msg_append_printf(struct msg *msg, const char *fmt, ...);
struct msg_row* msg_append_dup(struct msg *msg, const char *line, size_t len);
struct msg_cmd* msg_get_cmd(struct msg *msg);
int msg_validate(struct msg *msg);
struct msg_row* msg_argv(struct msg *msg, int num);
int msg_done(struct msg *msg);
void msg_trunk(struct msg *msg);
void msg_free(struct msg *msg);
struct msg* msg_init_pong();
struct msg* msg_init_error(int no, const char *err);

//
void msg_print(int level, struct msg *msg);
#endif // _MSG_H_


