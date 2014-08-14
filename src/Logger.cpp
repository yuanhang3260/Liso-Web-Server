#include <errno.h>
#include <sys/stat.h>
#include "Utility.h"
#include "Logger.h"

FILE* Logger::logfile;

/** @brief log function
 *
 *  @param leve log level
 *  @param format args passed to print function
 *  @return void
 */
void Logger::log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(logfile, format, args);
    fflush(logfile);
    va_end(args);

}


/** @brief end log */
void Logger::endLog() 
{
    int re = fclose(logfile);
    if (re < 0) {
        fprintf(stderr, "Close log file failed\n");
    }
}