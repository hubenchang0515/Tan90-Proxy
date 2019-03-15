#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static FILE* log_file = NULL;

static void log_info(const char* fmt, va_list args);
static void log_warning(const char* fmt, va_list args);
static void log_error(const char* fmt, va_list args);

int set_log_file(const char* fp)
{
    if(log_file != NULL)
    {
        log_printf(LOG_ERROR, "log file had been set before.");
        return EXIT_FAILURE;
    }

    log_file = fopen(fp, "a+");
    if(log_file == NULL)
    {
        log_printf(LOG_ERROR, "open log file failure : %s.", strerror(errno));
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
        log_printf(LOG_ERROR, "close log file failure : %s.", strerror(errno));
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
    fprintf(stdout, "[INFO]");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    if(log_file != NULL)
    {
        fprintf(log_file, "[INFO]");
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

static void log_warning(const char* fmt, va_list args)
{
    fprintf(stdout, "[WARNING]");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    if(log_file != NULL)
    {
        fprintf(log_file, "[WARNING]");
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

static void log_error(const char* fmt, va_list args)
{
    fprintf(stderr, "[ERROR]");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    if(log_file != NULL)
    {
        fprintf(log_file, "[ERROR]");
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}