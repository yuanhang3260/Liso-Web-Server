#ifndef __LOGGER_H_
#define __LOGGER_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

/** @brief Class Logger */
class Logger
{
public:
	/** @brief log level */
	enum LogLevel {
	    LOG_DEBUG = 1,
	    LOG_PROD = 2,
	};

	/** constructor */
	Logger(string _logFileName): logFileName(_logFileName)
	{
		logfile = fopen(_logFileName.c_str(), "w+");
    	if(logfile == NULL) {
        	fprintf(stderr, "Open log file %s failed\n", _logFileName.c_str());
    		exit(1);
    	}
	}

	/** log */
	static void log(const char* format, ...);
	/** end log */
	void endLog();

private:
	string logFileName;
	static FILE* logfile;
};



#endif /* __LOGGER_H_ */
