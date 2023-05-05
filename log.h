#ifndef LOG_H
#define LOG_H

enum LogLevel
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERR
};

void dlog(enum LogLevel l, char fmt[], ...);

#endif // LOG_H
