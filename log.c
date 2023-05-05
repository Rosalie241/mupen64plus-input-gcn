#include "log.h"
#include "main.h"

#include <stdio.h>
#include <stdarg.h>

void dlog(enum LogLevel l, char fmt[], ...)
{
    int m64p_loglevel;

    switch (l)
    {
    case LOG_ERR:
        m64p_loglevel = M64MSG_ERROR;
        break;
    case LOG_WARN:
        m64p_loglevel = M64MSG_WARNING;
        break;
    default:
        m64p_loglevel = M64MSG_INFO;
        break;
    }

    va_list argv;
    va_start(argv, fmt);

    char msg[1024];
    vsnprintf(msg, 1024, fmt, argv);

    va_end(argv);

    if (debug_callback != NULL)
    {
        debug_callback(debug_callback_context, m64p_loglevel, msg);
    }
}
