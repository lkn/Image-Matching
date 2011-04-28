//
// Also, automatically prints to file
// TODO: implement logging level, threadsafety
//
#ifndef LOGGER_H__
#define LOGGER_H__

#include <fstream>
#include <string>

// TODO: actually implement levels
enum LogLevel {
	ERR,
	WARNING,
	INFO,
	VERBOSE
};

class Logger {
public:
	Logger();
	Logger(std::string fileName);
	~Logger();

	void Init(std::string fileName);
	void Log(LogLevel level, const char *format, ...);

private:
	std::ofstream *logFile_;
	LogLevel 	   loggerLevel_;
};

#endif