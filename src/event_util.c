#include "event_util.h"

void event_log_callback(int s, const char *msg)
{
    switch(s)
    {
        case _EVENT_LOG_DEBUG:
            log_debug(msg);
            break;
        case _EVENT_LOG_MSG:
            log_info(msg);
            break;
        case _EVENT_LOG_WARN:
            log_warn(msg);
            break;
        case _EVENT_LOG_ERR:
            log_err(msg);
            break;
        default:
            log_debug(msg);
            break;
    }
}
