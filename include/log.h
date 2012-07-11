#ifndef MLLIB_LOG_H_
#define MLLIB_LOG_H_

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

namespace mlplus 
{

#define LOG(fmt, str...) Logger::instance()->printLog(fmt, ##str)
#define WARN(fmt, str...) Logger::instance()->printWarning(fmt, ##str)
#define ERROR(fmt, str...) Logger::instance()->printError(fmt, ##str)

#ifdef DEBUG
#define MSG(fmt, str...) Logger::instance()->logDebug(__FILE__, __LINE__, __func__, fmt, ##str)
#else
#define MSG(fmt, str...)  
#endif

#ifdef DEBUG
#define DEBUG_PURE(fmt, str...) Logger::instance()->logPure(fmt, ##str)
#else
#define DEBUG_PURE(fmt, str...)  
#endif

class Logger 
{

public:
	typedef enum 
    {
		LOG_LEVEL = 0,
		WARN_LEVEL = 1,
		ERROR_LEVEL = 2
	}LogType;

	static Logger* instance();
	static Logger* instance(LogType level);

	LogType setLogLevel(LogType level);
	bool setLogHandler(const char* file = NULL);
	FILE* setLogHandler(FILE* file_p = stderr);
	FILE* getLogHandler();
	void logPure(const char* fmt, ...);
	void logDebug(const char* file, int line, const char* func, const char* fmt, ...);
	void printLog(const char* fmt, ...);
	void printWarning(const char* fmt, ...);
	void printError(const char* fmt, ...);

private:
	Logger();
	~Logger();
	Logger(const Logger&);  // Prevent from copying a Singleton
	Logger& operator =(const Logger&);  // Prevent from copying a Singleton

	void log_trace(const char* file, int line, int level, const char* func, const char* fmt, va_list vap);
	void log_trace_pure(int level, const char* fmt, va_list vap);
	void trace_pure(const char* fmt, va_list vap);

	static void releaseHandler();
	static void get_cur_time(char cur[]);

	int set_log_handler(const char* file);
	//int set_log_level(int level);

	static Logger* minstance;
	static FILE* mfp_log;
	static LogType mlog_level;
	static pthread_mutex_t mlog_mutex;	
	static const char* msg_psLevelName[];

};
}

#endif
