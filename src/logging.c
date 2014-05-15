#include <stdio.h>
#include <string.h>

#include "logging.h"

#ifdef THREAD_SAFE
static struct logging _LOG = {LOG_DEFAULT, NULL, NULL, PTHREAD_MUTEX_INITIALIZER};
#else // THREAD_SAFE
static struct logging _LOG = {LOG_DEFAULT, NULL, NULL};
#endif // THREAD_SAFE

void log_init(int level)
{
    _LOG.level = level;
    _LOG.out = stdout;
    _LOG.err = stderr;
}

void log_set_out(FILE *out)
{
    if(out != NULL) 
        _LOG.out = out;
}

FILE* log_get_out()
{
    return _LOG.out;
}

void log_set_err(FILE *err)
{
    if(err != NULL) 
        _LOG.err = err;
}

FILE* log_get_err()
{
    return _LOG.err;
}

void log_set_level(int level)
{
    if(level != 0) _LOG.level = level;
}

int log_get_level()
{
    return _LOG.level;
}


void log_str(int level, int nl, const char *file, const int line, const char *level_str, const char *format, ...)
{
    if(_LOG.level > level)
        return;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    LOG_LOCK();

#ifdef LOG_FILELINE
    fprintf(_LOG.out, "%d-%02d-%02d %02d:%02d:%02d [%s] %s(%d) ",
                (tm->tm_year) + 1900, (tm->tm_mon) + 1, tm->tm_mday, tm->tm_hour,
                tm->tm_min, tm->tm_sec, level_str, file, line);
#else // LOG_FILELINE
    fprintf(_LOG.out, "%d-%02d-%02d %02d:%02d:%02d [%s] ", (tm->tm_year) +
                1900, (tm->tm_mon) + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
                tm->tm_sec, level_str);
#endif // LOG_FILELINE

    va_list argptr;
    va_start(argptr, format);
    vfprintf(_LOG.out, format, argptr);
    va_end(argptr);
    if(nl) fprintf(_LOG.out, "\n");
    fflush(_LOG.out);

    LOG_UNLOCK();
}

void log_estr(int level, int nl, const char *file, const int line, const char *level_str, const char *format, ...)
{
    if(_LOG.level > level)
        return;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    LOG_LOCK();

#ifdef LOG_FILELINE
    fprintf(_LOG.err, "%d-%02d-%02d %02d:%02d:%02d [%s] %s(%d) ",
                (tm->tm_year) + 1900, (tm->tm_mon) + 1, tm->tm_mday, tm->tm_hour,
                tm->tm_min, tm->tm_sec, level_str, file, line);
#else // LOG_FILELINE
    fprintf(_LOG.err, "%d-%02d-%02d %02d:%02d:%02d [%s] ", (tm->tm_year) +
                1900, (tm->tm_mon) + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
                tm->tm_sec, level_str);
#endif // LOG_FILELINE

    va_list argptr;
    va_start(argptr, format);
    vfprintf(_LOG.err, format, argptr);
    va_end(argptr);
    if(nl) fprintf(_LOG.err, "\n");
    fflush(_LOG.err);

    LOG_UNLOCK();
}

int log_get_level_str(const char *str)
{
    if(!str || strlen(str) == 0)
        return LOG_DEFAULT;

    if(!strncasecmp(str, "debug", strlen("debug")))
        return LOG_DEBUG;

    if(!strncasecmp(str, "verbose", strlen("verbose")))
        return LOG_VERBOSE;

    if(!strncasecmp(str, "info", strlen("info")))
        return LOG_INFO;

    if(!strncasecmp(str, "warn", strlen("warn")) ||
        !strncasecmp(str, "warning", strlen("warning")))
        return LOG_WARN;

    if( !strncasecmp(str, "error", strlen("error")) ||
        !strncasecmp(str, "err", strlen("err")))
        return LOG_ERR;

    if(!strncasecmp(str, "msg", strlen("msg")) ||
        !strncasecmp(str, "message", strlen("message")))
        return LOG_MSG;

    return LOG_DEFAULT;
}
