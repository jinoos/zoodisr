#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <inttypes.h>

#include "config.h"

#ifdef THREAD_SAFE
#include <pthread.h>
#endif // THREAD_SAFE

#define LOG_DEBUG      1 
#define LOG_VERBOSE    2
#define LOG_INFO       3
#define LOG_WARN       4
#define LOG_ERR        5
#define LOG_MSG        99
#define LOG_DEFAULT LOG_WARN

#define LOG_SDEBUG     "DEBUG"
#define LOG_SVERBOSE   "VBOSE"
#define LOG_SINFO      "INFO "
#define LOG_SWARN      "WARN "
#define LOG_SERR       "ERROR"
#define LOG_SMSG       "MSG  "

#define log_debug(F, ...)   do{log_str(LOG_DEBUG, 1, __FILE__, __LINE__, LOG_SDEBUG, F, ##__VA_ARGS__);}while(0)
#define log_verbose(F, ...) do{log_str(LOG_VERBOSE, 1, __FILE__, __LINE__, LOG_SVERBOSE, F, ##__VA_ARGS__);}while(0)
#define log_info(F, ...)    do{log_str(LOG_INFO, 1, __FILE__, __LINE__, LOG_SINFO, F, ##__VA_ARGS__);}while(0)
#define log_warn(F, ...)    do{log_str(LOG_WARN, 1, __FILE__, __LINE__, LOG_SWARN, F, ##__VA_ARGS__);}while(0)
#define log_err(F, ...)     do{log_str(LOG_ERR, 1, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_error(F, ...)   do{log_str(LOG_ERR, 1, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_msg(F, ...)     do{log_str(LOG_MSG, 1, __FILE__, __LINE__, LOG_SMSG, F, ##__VA_ARGS__);}while(0)

#define log_debug_n(F, ...)   do{log_str(LOG_DEBUG, 0, __FILE__, __LINE__, LOG_SDEBUG, F, ##__VA_ARGS__);}while(0)
#define log_verbose_n(F, ...) do{log_str(LOG_VERBOSE, 0, __FILE__, __LINE__, LOG_SVERBOSE, F, ##__VA_ARGS__);}while(0)
#define log_info_n(F, ...)    do{log_str(LOG_INFO, 0, __FILE__, __LINE__, LOG_SINFO, F, ##__VA_ARGS__);}while(0)
#define log_warn_n(F, ...)    do{log_str(LOG_WARN, 0, __FILE__, __LINE__, LOG_SWARN, F, ##__VA_ARGS__);}while(0)
#define log_err_n(F, ...)     do{log_str(LOG_ERR, 0, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_error_n(F, ...)   do{log_str(LOG_ERR, 0, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_msg_n(F, ...)     do{log_str(LOG_MSG, 0, __FILE__, __LINE__, LOG_SMSG, F, ##__VA_ARGS__);}while(0)

#define log_print(l,F, ...) do{                                                             \
                                if(l ==LOG_DEBUG) log_debug(F, ##__VA_ARGS__);              \
                                else if(l == LOG_DEBUG) log_debug(F, ##__VA_ARGS__);        \
                                else if(l == LOG_VERBOSE) log_verbose(F, ##__VA_ARGS__);    \
                                else if(l == LOG_INFO) log_info(F, ##__VA_ARGS__);          \
                                else if(l == LOG_WARN) log_warn(F, ##__VA_ARGS__);          \
                                else if(l == LOG_ERR) log_err(F, ##__VA_ARGS__);            \
                                else if(l == LOG_MSG) log_msg(F, ##__VA_ARGS__);            \
                            }while(0)
#define log_print_n(l,F, ...) do{                                                             \
                                if(l ==LOG_DEBUG) log_debug_n(F, ##__VA_ARGS__);              \
                                else if(l == LOG_DEBUG) log_debug_n(F, ##__VA_ARGS__);        \
                                else if(l == LOG_VERBOSE) log_verbose_n(F, ##__VA_ARGS__);    \
                                else if(l == LOG_INFO) log_info_n(F, ##__VA_ARGS__);          \
                                else if(l == LOG_WARN) log_warn_n(F, ##__VA_ARGS__);          \
                                else if(l == LOG_ERR) log_err_n(F, ##__VA_ARGS__);            \
                                else if(l == LOG_MSG) log_msg_n(F, ##__VA_ARGS__);            \
                            }while(0)

#define log_edebug(F, ...)   do{log_estr(LOG_DEBUG, 1, __FILE__, __LINE__, LOG_SDEBUG, F, ##__VA_ARGS__);}while(0)
#define log_everbose(F, ...) do{log_estr(LOG_VERBOSE, 1, __FILE__, __LINE__, LOG_SVERBOSE, F, ##__VA_ARGS__);}while(0)
#define log_einfo(F, ...)    do{log_estr(LOG_INFO, 1, __FILE__, __LINE__, LOG_SINFO, F, ##__VA_ARGS__);}while(0)
#define log_ewarn(F, ...)    do{log_estr(LOG_WARN, 1, __FILE__, __LINE__, LOG_SWARN, F, ##__VA_ARGS__);}while(0)
#define log_eerr(F, ...)     do{log_estr(LOG_ERR, 1, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_eerror(F, ...)   do{log_estr(LOG_ERR, 1, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_emsg(F, ...)     do{log_estr(LOG_MSG, 1, __FILE__, __LINE__, LOG_SMSG, F, ##__VA_ARGS__);}while(0)

#define log_edebug_n(F, ...)   do{log_estr(LOG_DEBUG, 0, __FILE__, __LINE__, LOG_SDEBUG, F, ##__VA_ARGS__);}while(0)
#define log_everbose_n(F, ...) do{log_estr(LOG_VERBOSE, 0, __FILE__, __LINE__, LOG_SVERBOSE, F, ##__VA_ARGS__);}while(0)
#define log_einfo_n(F, ...)    do{log_estr(LOG_INFO, 0, __FILE__, __LINE__, LOG_SINFO, F, ##__VA_ARGS__);}while(0)
#define log_ewarn_n(F, ...)    do{log_estr(LOG_WARN, 0, __FILE__, __LINE__, LOG_SWARN, F, ##__VA_ARGS__);}while(0)
#define log_eerr_n(F, ...)     do{log_estr(LOG_ERR, 0, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_eerror_n(F, ...)   do{log_estr(LOG_ERR, 0, __FILE__, __LINE__, LOG_SERR, F, ##__VA_ARGS__);}while(0)
#define log_emsg_n(F, ...)     do{log_estr(LOG_MSG, 0, __FILE__, __LINE__, LOG_SMSG, F, ##__VA_ARGS__);}while(0)

#define log_eprint(l,F, ...) do{                                                            \
                                if(l ==LOG_DEBUG) log_edebug(F, ##__VA_ARGS__);             \
                                else if(l == LOG_DEBUG) log_edebug(F, ##__VA_ARGS__);       \
                                else if(l == LOG_VERBOSE) log_everbose(F, ##__VA_ARGS__);   \
                                else if(l == LOG_INFO) log_einfo(F, ##__VA_ARGS__);         \
                                else if(l == LOG_WARN) log_ewarn(F, ##__VA_ARGS__);         \
                                else if(l == LOG_ERR) log_eerr(F, ##__VA_ARGS__);           \
                                else if(l == LOG_MSG) log_emsg(F, ##__VA_ARGS__);           \
                            }while(0)

#define log_eprint_n(l,F, ...) do{                                                            \
                                if(l ==LOG_DEBUG) log_edebug_n(F, ##__VA_ARGS__);             \
                                else if(l == LOG_DEBUG) log_edebug_n(F, ##__VA_ARGS__);       \
                                else if(l == LOG_VERBOSE) log_everbose_n(F, ##__VA_ARGS__);   \
                                else if(l == LOG_INFO) log_einfo_n(F, ##__VA_ARGS__);         \
                                else if(l == LOG_WARN) log_ewarn_n(F, ##__VA_ARGS__);         \
                                else if(l == LOG_ERR) log_eerr_n(F, ##__VA_ARGS__);           \
                                else if(l == LOG_MSG) log_emsg_n(F, ##__VA_ARGS__);           \
                            }while(0)

#ifdef THREAD_SAFE
#define LOG_LOCK()      do{pthread_mutex_lock(&_LOG.lock);}while(0)
#define LOG_UNLOCK()    do{pthread_mutex_unlock(&_LOG.lock);}while(0)
#else // THREAD_SAFE
#define LOG_LOCK()
#define LOG_UNLOCK()
#endif // THREAD_SAFE

#define log_set_level_debug()       (log_set_level(LOG_DEFAULT))
#define log_set_level_verbose()     (log_set_level(LOG_VERBOSE))
#define log_set_level_info()        (log_set_level(LOG_INFO))
#define log_set_level_warn()        (log_set_level(LOG_WARN))
#define log_set_level_err()         (log_set_level(LOG_ERR))
#define log_set_level_msg()         (log_set_level(LOG_MSG))


#define log_printable(x)        (log_get_level() <= x ? 1 : 0)
#define log_enabled_debug()      (log_printable(LOG_DEFAULT))
#define log_enabled_verbose()    (log_printable(LOG_VERBOSE))
#define log_enabled_info()       (log_printable(LOG_INFO))
#define log_enabled_warn()       (log_printable(LOG_WARN))
#define log_enabled_err()        (log_printable(LOG_ERR))
#define log_enabled_msg()        (log_printable(LOG_MSG))

struct logging
{
    int level;
    FILE *out;
    FILE *err;
#ifdef THREAD_SAFE
    pthread_mutex_t lock;
#endif // THREAD_SAFE
};

void log_init();
void log_set_out(FILE *out);
FILE* log_get_out();
void log_set_err(FILE *err);
FILE* log_get_err();
void log_set_level(int level);
int log_get_level();
int log_get_level_str(const char *str);

void log_str(int level, int nl, const char *file, const int line, const char *level_str, const char *format, ...);
void log_estr(int level, int nl, const char *file, const int line, const char *level_str, const char *format, ...);

#endif // _LOGGING_H_
