#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "../../test/debug.h"

FILE* logfile = 0;
const char* LOG_PATH = "log.txt";

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
        open_log(LOG_PATH);
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
    vfprintf(logfile, message, args);
    fprintf(logfile, "\n");
    fflush(logfile);

#if defined(_DEBUG) || defined(DEBUG)
    printf("%s [%s]: ", time_buffer, tag);
    vprintf(message, args);
    printf("\n");
#endif

    va_end(args);
}
