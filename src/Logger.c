#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include "Utility.h"
#include "Logger.h"

/** log file */
static FILE* logFILE;

/** @brief init logger
 *  @param logFile log file
 *  @return int
 */
int initLogger(char *logFileName)
{
    logFILE = fopen(logFileName, "w+");
    if(logFILE != NULL) {
        return 0;
    } else {
        fprintf(stderr, "Open Log file %s failed\n", logFileName);
        exit(1);
    }
}


/** @brief log function
 *
 *  @param leve log level
 *  @param format args passed to print function
 *  @return void
 */
void logger(enum LogLevel level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if(level >= CUR_LOG_LEVEL) 
    {
        vfprintf(logFILE, format, args);
        fflush(logFILE);
    }
    va_end(args);

}


/** @brief get log file
 *  @return log FILE*
 */
FILE* getLogger() 
{
    return logFILE;
}


/** @brief end log */
void endLog() 
{
    int re = fclose(logFILE);
    if (re < 0) {
        fprintf(stderr, "Close log file failed\n");
    }
}