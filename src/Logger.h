#ifndef __LOGGER_H_
#define __LOGGER_H_ 

#define CUR_LOG_LEVEL LogDebug

/** @brief log level */
enum LogLevel{
    LogDebug=1,
    LogProd=2,
};


/** init logger */
int initLogger(char *logFileName);
/** logger */
void logger(enum LogLevel, const char *format, ...);
/** get logger file */
FILE* getLogger();
/** end log */
void endLog();


#endif /* __LOGGER_H_ */
