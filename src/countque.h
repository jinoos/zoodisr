#ifndef _COUNTERQUE_H_
#define _COUNTERQUE_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct cq_tic
{
	uint32_t			count;
	time_t				time;
	struct cq_tic		*newer;
};

struct cq
{
	uint64_t			count;

    time_t              tic_per_sec;
    uint32_t            tic_max_cnt;

	struct cq_tic       *first;
	struct cq_tic       *last;
};

uint64_t cq_tic_time(struct cq *cq, time_t t);
uint64_t cq_tic(struct cq *cq);
struct cq* cq_init(struct cq *cq, time_t tic_per_sec, uint32_t tic_max_cnt);
struct cq* cq_alloc(time_t tic_per_sec, uint32_t tic_max_cnt);
uint64_t cq_count(struct cq *cq);
time_t cq_tic_per_seconds(struct cq *cq);
uint32_t cq_tic_max_count(struct cq *cq);
void cq_flush(struct cq *cq);
void cq_free(struct cq *cq);

/////////////////////////////////////////////////////////////////////

time_t get_base_time_by_time(time_t term, time_t t);
time_t get_base_time(time_t term);
time_t get_ttl_time(time_t base_time, time_t tic_per_sec, int tic_max_cnt);
struct cq_tic* cq_tic_init(struct cq *cq);
void cq_tic_free(struct cq *cq, struct cq_tic *tic);
void cq_adjust(struct cq *cq, time_t base_time);

#endif // _COUNTERQUE_H_
