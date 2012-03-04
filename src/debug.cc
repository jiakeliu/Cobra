#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "debug.h"


static int g_debug_level = 0;

void
set_debug_level(int level)
{
    g_debug_level = level;
}

int
log(int level, const char *file, int line, const char *fmt, ...)
{
    int ret = 0;
    bool error = (IS_ERROR(level) == ERROR_BIT);
    FILE* std = (error) ? stderr : stdout;
    time_t now;
    va_list va;

    /* if the log level is greater than this event, return */
    if (LEVEL(level) < LEVEL(g_debug_level))
        return -1;

    /* if g_debug_level is an error, we only print errors, regardless of level */
    if (!error && IS_ERROR(g_debug_level))
        return -1;

    now = time(NULL);

    /* print the file and line if it exists. */
    if (file && line > 0)
        ret += fprintf(std, LOG_FMT, (unsigned long)now, file, line, (error)?"!!!":":");
    else if (error)
        fprintf(std, "!!!");

    va_start(va, fmt);
    ret += vfprintf(std, fmt, va);
    va_end(va);

    fflush(std);
    return ret;
}


