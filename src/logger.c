#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

FILE* logfile = 0;

void open_log(char* filename)
{
    if (logfile)
        fclose(logfile);
    
    logfile = fopen(filename, "w");
}

void close_log()
{
    if (logfile)
        fclose(logfile);
    logfile = 0;
}

void logger(const char* tag, const char* message, ...) {
    if (!logfile)
    {
        printf("Make sure open_log is called at the beginning of the program!!!\n");
        return;
    }

    va_list args;
    va_start(args, message);

    time_t now;
    struct tm * timeinfo;
    char time_buffer[100];

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, 100, "%b %e %T", timeinfo);

    fprintf(logfile, "%s [%s]: ", time_buffer, tag);
    printf("%s [%s]: ", time_buffer, tag);
    vfprintf(logfile, message, args);
    vprintf(message, args);
    fprintf(logfile, "\n");
    printf("\n");
    fflush(logfile);

    va_end(args);
}