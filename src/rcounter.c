#include "rcounter.h"

struct rcounter* rcounter_init(struct rcounter *c)
{
    cq_init(&c->rok, 1, ZR_RR_COUNTER_BUFFER);
    cq_init(&c->rerr, 1, ZR_RR_COUNTER_BUFFER);
    c->last_requested_time = 0;
    c->uptime = time(NULL);

    return c;
}

void rcounter_destory(struct rcounter *c)
{
    cq_flush(&c->rok);
    cq_flush(&c->rerr);
}

void rcounter_tic_ok(struct rcounter *c)
{
    c->ok++;
    cq_tic(&c->rok);
    c->last_requested_time = time(NULL);
}

void rcounter_tic_err(struct rcounter *c)
{
    c->err++;
    cq_tic(&c->rerr);
    c->last_requested_time = time(NULL);
}

uint64_t rcounter_ok(struct rcounter *c)
{
    return c->ok;
}

uint64_t rcounter_rok(struct rcounter *c)
{
    return cq_count(&c->rok);
}

uint64_t rcounter_err(struct rcounter *c)
{
    return c->ok;
}

uint64_t rcounter_rerr(struct rcounter *c)
{
    return cq_count(&c->rerr);
}
