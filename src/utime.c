#include "utime.h"

inline const uint64_t utime_time()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return ((uint64_t)(t.tv_sec*1000000) + (uint64_t)t.tv_usec);
}

// @brief Get cpu time
inline uint64_t utime_cpu()
{
	unsigned int lo,hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)hi << 32) | lo;
}
