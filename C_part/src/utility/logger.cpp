#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

struct loggy_boi
{
    loggy_boi(char* file) : logfile(fopen(file, "w")) {}
    ~loggy_boi() { if (logfile) fclose(logfile); }

    FILE* logfile;
};

loggy_boi boi = loggy_boi("log.txt");

void logger(const char* tag, const char* message, ...) {
    if (!boi.logfile)
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

    fprintf(boi.logfile, "%s [%s]: ", time_buffer, tag);
    printf("%s [%s]: ", time_buffer, tag);
    vfprintf(boi.logfile, message, args);
    vprintf(message, args);
    fprintf(boi.logfile, "\n");
    printf("\n");
    fflush(boi.logfile);

    va_end(args);
}

void logger_noline(const char* msg, ...)
{
    if (!boi.logfile)
    {
        printf("Make sure open_log is called at the beginning of the program!!!\n");
        return;
    }

    va_list args;
    va_start(args, msg);

    time_t now;
    struct tm * timeinfo;
    char time_buffer[100];

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, 100, "%b %e %T", timeinfo);

    vfprintf(boi.logfile, msg, args);
    vprintf(msg, args);

    va_end(args);
}
