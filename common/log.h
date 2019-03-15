#ifndef TAN90_PROXY_LOG_H
#define TAN90_PROXY_LOG_H

#include <stdarg.h>

enum LogLevel
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

int set_log_file(const char* fp);
int close_log_file(void);

void _log_printf(enum LogLevel level, const char* fmt, ...);
void log_printf(enum LogLevel level, const char* fmt, ...);

#ifndef log_printf
#define log_printf(level, fmt, ...)  _log_printf(level, " <%s : %d> " fmt, __FILE__, __LINE__, ##__VA_ARGS__ )
#endif

#endif