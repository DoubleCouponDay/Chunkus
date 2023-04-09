#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"

FILE* logfile = NULL;
const char* LOG_PATH = "log.txt";

void open_log(const char* filename)
{
    if (logfile != NULL)
        fclose(logfile);
    
    logfile = fopen(filename, "w");
}

void close_log()
{
    if (logfile)
        fclose(logfile);
    logfile = NULL;
}

void clear_logfile() {
    if(logfile) {
        close_log();
    }
    open_log(LOG_PATH);
}

#ifndef NDEBUG
void logger(const char* tag, const char* message, ...) {
    if (!logfile)
    {
        open_log(LOG_PATH);
    }

    va_list args;
    va_start(args, message);

    time_t now;
    struct tm timeinfo;
    char time_buffer[100];

    time(&now);
    int err = localtime_s(&timeinfo, &now);
    if (!err)
        return;
    strftime(time_buffer, 100, "%b %e %T", &timeinfo);

    //print to log file
    fprintf(logfile, "%s [%s]: ", time_buffer, tag);
    vfprintf(logfile, message, args); 
    fprintf(logfile, "\n");        

    fflush(logfile);
    va_end(args);
}
#else
void logger(const char* tag, const char* message, ...) {}
#endif
