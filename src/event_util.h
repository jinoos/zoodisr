#ifndef _EVENT_UTIL_H_
#define _EVENT_UTIL_H_

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "logging.h"

void event_log_callback(int s, const char *msg);

#endif // _EVENT_UTIL_H_
