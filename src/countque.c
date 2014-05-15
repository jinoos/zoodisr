#include "countque.h"

uint64_t cq_tic_time(struct cq *cq, time_t t)
{
    time_t base_time = get_base_time_by_time(cq->tic_per_sec, t);

	struct cq_tic *tic;
	if(cq->last == NULL)
	{
		tic = cq_tic_init(cq);
		tic->time = base_time;
		tic->count++;

		cq->first = cq->last = tic;

	}else
	{
		if(cq->last->time == base_time)
		{
			cq->last->count++;
		}else
		{
			tic = cq_tic_init(cq);
			tic->time = base_time;
			tic->count++;
			cq->last->newer = tic;
			cq->last = tic;
		}
    }

	cq->count++;
    cq_adjust(cq, base_time);

	return cq->count;
}

uint64_t cq_tic(struct cq *cq)
{
    time_t t = time(NULL);
    return cq_tic_time(cq, t);
}

struct cq* cq_init(struct cq *cq, time_t tic_per_sec, uint32_t tic_max_cnt)
{
    cq->count = 0;
    cq->tic_per_sec = tic_per_sec;
    cq->tic_max_cnt = tic_max_cnt;
    cq->first = cq->last = NULL;
    return cq;
}

struct cq* cq_alloc(time_t tic_per_sec, uint32_t tic_max_cnt)
{
	struct cq *cq = (struct cq*) calloc(sizeof(struct cq), 1);
    cq_init(cq, tic_per_sec, tic_max_cnt);
    return cq;
}

uint64_t cq_count(struct cq *cq)
{
    time_t base_time = get_base_time(cq->tic_per_sec);
    cq_adjust(cq, base_time);
    return cq->count;
}

time_t cq_tic_per_seconds(struct cq *cq)
{
    return cq->tic_per_sec;
}
uint32_t cq_tic_max_count(struct cq *cq)
{
    return cq->tic_max_cnt;
}

void cq_flush(struct cq *cq)
{
    struct cq_tic *tic = cq->first;
    while(tic != NULL)
    {
        cq->first = tic->newer;
        free(tic);
        tic = cq->first;
    }

    cq->count = 0;

    return;
}

void cq_free(struct cq *cq)
{
    cq_flush(cq);
    free(cq);
    return;
}

/////////////////////////////////////////////////////////////////////

time_t get_base_time_by_time(time_t term, time_t t)
{
	return t-(t%term);
}

time_t get_base_time(time_t term)
{
    time_t t = time(NULL);
    return get_base_time_by_time(term, t);
}

time_t get_ttl_time(time_t base_time, time_t tic_per_sec, int tic_max_cnt)
{
	return (time_t)base_time - (tic_per_sec * (time_t)tic_max_cnt);
}

struct cq_tic* cq_tic_init(struct cq *cq)
{
	struct cq_tic *tic = (struct cq_tic*) calloc(sizeof(struct cq_tic), 1);
	tic->count = 0;
	tic->newer = NULL;
	return tic;
}

void cq_adjust(struct cq *cq, time_t base_time)
{
	time_t ttl = get_ttl_time(base_time, cq->tic_per_sec, cq->tic_max_cnt);
	struct cq_tic *tic = cq->first;
	while(tic != NULL && tic->time <= ttl)
	{
		cq->first = tic->newer;
		cq->count -= tic->count;
        free(tic);
		tic = cq->first;
	}

    if(tic == NULL)
    {
        cq->first = cq->last = NULL;
    }

    return;
}


