#ifndef _UTIME_H_
#define _UTIME_H_

#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>

inline const uint64_t utime_time();

// @brief Get cpu time
inline uint64_t utime_cpu();

#endif // _UTIME_H_
