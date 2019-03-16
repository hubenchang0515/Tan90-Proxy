/****************************************************************************************
MIT License

Copyright (c) 2019 Plan C

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

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
    #ifdef DEBUG
        #define log_printf(level, fmt, ...)  _log_printf(level, " <%s : %d> " fmt, __FILE__, __LINE__, ##__VA_ARGS__ )
    #else
        #define log_printf(level, fmt, ...)  _log_printf(level, fmt, ##__VA_ARGS__ )
    #endif
#endif

#endif