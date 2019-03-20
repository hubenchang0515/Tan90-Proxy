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

#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
    #include "leakcheck.h"
#endif

static FILE* log_file = NULL;

static void log_info(const char* fmt, va_list args);
static void log_warning(const char* fmt, va_list args);
static void log_error(const char* fmt, va_list args);

int set_log_file(const char* fp)
{
    if(log_file != NULL)
    {
        log_printf(LOG_ERROR, "Log file had been set before.");
        return EXIT_FAILURE;
    }

    log_file = fopen(fp, "a+");
    if(log_file == NULL)
    {
        log_printf(LOG_ERROR, "Open log file failure : %s.", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int close_log_file(void)
{
    if(log_file == NULL || 0 == fclose(log_file))
    {
        return EXIT_SUCCESS;
    }
    else
    {
        log_printf(LOG_ERROR, "Close log file failure : %s.", strerror(errno));
        return EXIT_FAILURE;
    }
    
}

void _log_printf(enum LogLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    switch(level)
    {
    case LOG_INFO : 
        log_info(fmt, args);
        break;

    case LOG_WARNING : 
        log_warning(fmt, args);
        break;

    case LOG_ERROR : 
        log_error(fmt, args);
        break;
    }
    va_end(args);
}


static void log_info(const char* fmt, va_list args)
{
    fprintf(stdout, "[INFO]    : ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    if(log_file != NULL)
    {
        fprintf(log_file, "[INFO]    : ");
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

static void log_warning(const char* fmt, va_list args)
{
    fprintf(stdout, "[WARNING] : ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    if(log_file != NULL)
    {
        fprintf(log_file, "[WARNING] : ");
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

static void log_error(const char* fmt, va_list args)
{
    fprintf(stderr, "[ERROR]   : ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    if(log_file != NULL)
    {
        fprintf(log_file, "[ERROR]   : ");
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}