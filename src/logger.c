#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

static bool log_intiialized = false;
static FILE *log_file;
static const char *log_file_path = "log.txt";

void init_logger(const char *log_file_path)
{
    log_file = fopen(log_file_path, "w");
    if (log_file == NULL)
    {
        printf("Failed to open log file\n");
        exit(EXIT_FAILURE);
    }
    log_intiialized = true;
}

void logger(const char *tag, const char *format, ...)
{
    if (!log_intiialized)
    {
        init_logger(log_file_path);
    }
    time_t now;
    time(&now);
    fprintf(log_file, "\n%s [%s]: ", ctime(&now), tag);
    // printf("%s [%s]: ", ctime(&now), tag);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    // vprintf(format, args);
    va_end(args);

    fflush(log_file);
}
