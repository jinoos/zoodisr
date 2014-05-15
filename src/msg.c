#include "msg.h"

static char _msg_last_err[ZR_MSG_LAST_ERR_BUF_LEN];
static char *_msg_last_err_ptr = NULL;
static char _msg_buf[ZR_MSG_STR_BUF_LEN];

// @return
//      1 : include newline chars (\r\n)
//      0 : not include newline chars in end of the string.
int msg_check_newline(const char *str, size_t len)
{
    if(str[len-2] != '\r' || str[len-1] != '\n')
        return 0;
    else
        return 1;
}

void msg_set_last_error_empty()
{
    _msg_last_err_ptr = NULL;
}

char* msg_set_last_error(const char *err)
{
    if(strlen(err) > (ZR_MSG_LAST_ERR_BUF_LEN-1))
    {
        strncpy(_msg_last_err, err, ZR_MSG_LAST_ERR_BUF_LEN-1);
    }else
    {
        strncpy(_msg_last_err, err, strlen(err));
    }

    _msg_last_err_ptr = _msg_last_err;

    return _msg_last_err_ptr;
}

char* msg_last_error()
{
    return _msg_last_err_ptr;
}

struct msg* msg_alloc()
{
    return (struct msg*) calloc(sizeof(struct msg), 1);
}

struct msg* msg_alloc_with_line(const char *line, size_t len)
{
    struct msg *m = msg_alloc();
    msg_append_dup(m, line, len);
    return m;
}

struct msg_row* msg_append(struct msg *msg, char *line, size_t len)
{
    struct msg_row *row = calloc(sizeof(struct msg_row), 1);
    row->data = line;
    row->len = len;
    row->inc_nl = msg_check_newline(line, len);

    if(msg->last == NULL)
    {
        msg->first = msg->last = row;
    }else
    {
        msg->last->next = row;
        msg->last = row;
    }
    msg->argc++;

    return row;
}

struct msg_row* msg_append_printf(struct msg *msg, const char *fmt, ...)
{
    int len;
    va_list ap;
    struct msg_row *row = NULL;
    va_start(ap, fmt);
    if((len = vsnprintf(_msg_buf, ZR_MSG_STR_BUF_LEN, fmt, ap)) >= 0)
    {
        row = msg_append_dup(msg, _msg_buf, len);
    }
    va_end(ap);

    return row;
}

struct msg_row* msg_append_dup(struct msg *msg, const char *line, size_t len)
{
    char *data = malloc(len+1);
    data[len] = '\0';
    memcpy(data, line, len);
    return msg_append(msg, data, len);
}

struct msg_cmd* msg_get_cmd(struct msg *msg)
{
    if(!msg->validated)
        msg_validate(msg);

    if(!msg->is_valid)
        return NULL;

    return &msg->cmd;
}

int msg_validate(struct msg *msg)
{
    if(msg->validated)
    {
        return msg->is_valid;
    }

    msg->validated = 1;

    if(!msg->first)
        goto invalid;

    if(msg->first->data[0] != ZR_CMD_REQ_PREFIX && 
            msg->first->data[0] != ZR_CMD_REP_PREFIX )
        goto invalid;

    if(strpbrk(&msg->first->data[1], " !@#$%^&*()+-=|\\~`[]{}:;\"'<>?/.,"))
        goto invalid;

    int limit  = 3 - (msg->first->inc_nl * 2);

    if(strlen(msg->first->data) <= limit)
        goto invalid;

    msg->cmd.name = msg->first->data;
    msg->cmd.len = msg->first->len - (msg->first->inc_nl * 2);

    msg->is_valid = 1;

    return msg->is_valid;

invalid:
    msg->is_valid = 0;
    return 0;
}

int msg_done(struct msg *msg)
{
    msg->t = utime_time();
    return msg->argc;
}

void msg_trunk(struct msg *msg)
{
    struct msg_row *nrow, *row = msg->first;
    while(row)
    {
        nrow = row->next;
        free(row->data);
        free(row);
        row = nrow;
    }
    msg->argc = 0;
    msg->first = msg->last = NULL;
    msg->is_valid = msg->validated = 0;
    msg->cmd.name = NULL;
    msg->cmd.len = 0;
    msg->t = 0;
}

void msg_free(struct msg *msg)
{
    msg_trunk(msg);
    free(msg);
}

struct msg_row* msg_argv(struct msg *msg, int num)
{
    if(msg->argc <= num)
        return NULL;

    int i = num;
    struct msg_row *row = msg->first;
    while(i--)
        row = row->next;

    return row;
}

struct msg* msg_init_pong()
{
    struct msg *msg = msg_alloc();
    msg_append_dup(msg, ZR_CMD_REP_PONG, strlen(ZR_CMD_REP_PONG));
    return msg;
}

struct msg* msg_init_error(int no, const char *err)
{
    struct msg *msg = msg_alloc();
    char buf[11];
    sprintf(buf, "%d", no);

    msg_append_dup(msg, ZR_CMD_VAR(Error).name, strlen(ZR_CMD_VAR(Error).name));
    msg_append_dup(msg, buf, strlen(buf));
    msg_append_dup(msg, err, strlen(err));
    return msg;
}

//
//
//
//
void msg_print(int level, struct msg *msg)
{
    struct msg_row *row = msg->first;
    log_print(level, "- Print message ----------------------------------");
    while(row)
    {
        if(row->inc_nl)
            log_print_n(level, "%s", row->data);
        else
            log_print(level, "%s", row->data);

        row = row->next;
    }
    log_print(level, "- End printing message ---------------------------");
}
