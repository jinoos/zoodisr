#include "zoodisr.h"

int main(void)
{

    log_init(LOG_DEFAULT);

    //struct msg *msg = msg_alloc("+PINPAG", 2);
    struct msg *msg = msg_alloc_load("+PINPAG 9");
    log_err("%s", msg_last_error());
    msg_add_arg(msg, "ARG1");
    msg_add_arg(msg, "ARG2");
    msg_add_arg(msg, "ARG3");
    msg_add_arg(msg, "ARG4");
    msg_add_arg(msg, "ARG5");
    msg_add_arg(msg, "ARG6");
    msg_add_arg(msg, "ARG7");
    msg_add_arg(msg, "ARG8");
    msg_add_arg(msg, "ARG9");
    msg_add_arg(msg, "ARG10");
    log_err("%s", msg_last_error());

    msg_print(LOG_ERR, msg);


    return 0;
}

