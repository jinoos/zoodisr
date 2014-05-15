#ifndef _RCOUNTER_H_
#define _RCOUNTER_H_

#include <stdint.h>

#include "countque.h"

// request counter
struct rcounter
{
    uint64_t    ok;
    uint64_t    err;

    // recent counter
    struct cq   rok;
    struct cq   rerr;

    time_t      last_requested_time;
    time_t      uptime;
};

#include "zoodisr.h"

struct rcounter* rcounter_init(struct rcounter *counter);
void rcounter_destory(struct rcounter *counter);
void rcounter_tic_ok(struct rcounter *c);
void rcounter_tic_err(struct rcounter *c);

uint64_t rcounter_ok(struct rcounter *c);
uint64_t rcounter_rok(struct rcounter *c);
uint64_t rcounter_err(struct rcounter *c);
uint64_t rcounter_rerr(struct rcounter *c);

#endif // _RCOUNTER_H_


