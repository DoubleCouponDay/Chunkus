#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"

FILE* logfile = 0;
const char* LOG_PATH = "log.txt";
const char* RELEASE_BUILD_VAR = "releasebuild";

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

void clear_logfile() {
    if(logfile) {
        close_log();
    }
    open_log(LOG_PATH);
}

void logger(const char* tag, const char* message, ...) {
    if (!logfile)
    {
        open_log(LOG_PATH);
    }

    va_list args;
    va_start(args, message);

    time_t now;
    struct tm * timeinfo;
    char time_buffer[100];

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, 100, "%b %e %T", timeinfo);
    char* possiblerelease = getenv(RELEASE_BUILD_VAR);

    if(possiblerelease == NULL) {
        printf("'%s' not found in environment!\n", RELEASE_BUILD_VAR);
        exit(ASSUMPTION_WRONG);
    }
    bool isdebug = strcmp(possiblerelease, "false") == 0;

    if(isdebug) { //print to terminal
        printf("%s [%s]: ", time_buffer, tag);
        vprintf(message, args);
        printf("\n");
    }

    else { //print to log file
        fprintf(logfile, "%s [%s]: ", time_buffer, tag);
        vfprintf(logfile, message, args); 
        fprintf(logfile, "\n");        
    }
    fflush(logfile);
    va_end(args);
}
