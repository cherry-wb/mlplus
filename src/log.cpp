#include "log.h"
using namespace std;

namespace mlplus 
{
Logger* Logger::minstance = NULL;

FILE* Logger::mfp_log = stdout;

Logger::LogType Logger::mlog_level = LOG_LEVEL;

pthread_mutex_t Logger::mlog_mutex = PTHREAD_MUTEX_INITIALIZER;

const char* Logger::msg_psLevelName[] = { "LOG", "WARN", "ERROR"};

Logger::Logger() 
{
}

Logger::~Logger()
{
	pthread_mutex_lock(&mlog_mutex);
	if (mfp_log != NULL && mfp_log != stderr && mfp_log != stdout) 
    {
		fclose(mfp_log);
	}
	pthread_mutex_unlock(&mlog_mutex);
}

Logger* Logger::instance() 
{
	pthread_mutex_lock(&mlog_mutex);
	if (minstance == NULL) 
    {
		minstance = new Logger();
		atexit(&releaseHandler);
	}
	pthread_mutex_unlock(&mlog_mutex);
	
	return minstance;
}

Logger* Logger::instance(LogType level) 
{
	pthread_mutex_lock(&mlog_mutex);
	if (minstance == NULL) 
    {
		minstance = new Logger();
		atexit(&releaseHandler);
	}
	mlog_level = level;
	pthread_mutex_unlock(&mlog_mutex);

	return minstance;
}


void Logger::releaseHandler() 
{
	//pthread_mutex_lock(&mlog_mutex);
	//if (minstance != NULL) 
    //{
	delete minstance;
	//	minstance = NULL;
	//}
	//pthread_mutex_unlock(&mlog_mutex);
}

Logger::LogType Logger::setLogLevel(LogType level) 
{
	LogType lt = mlog_level;

	pthread_mutex_lock(&mlog_mutex);
	mlog_level = level;
	pthread_mutex_unlock(&mlog_mutex);

	return lt;
}

bool Logger::setLogHandler(const char* file) 
{
	pthread_mutex_lock(&mlog_mutex);

	if (file != NULL) 
    {
		if (mfp_log != NULL && mfp_log != stdout && mfp_log != stderr) 
        {
			fclose(mfp_log);
		}
		if (set_log_handler(file) == -1) 
        {
			pthread_mutex_unlock(&mlog_mutex);
			return false;
		}
	} 
    else 
    {
		if (mfp_log != stderr && mfp_log != stdout) 
        {
			fclose(mfp_log);
			mfp_log = stdout;
		}
	}

	pthread_mutex_unlock(&mlog_mutex);

	return true;
}

FILE* Logger::getLogHandler()
{
	return mfp_log;
}	

FILE* Logger::setLogHandler(FILE* file_p) 
{
	FILE* fp = mfp_log;

	//cerr << "setLH FILE" << endl;
	pthread_mutex_lock(&mlog_mutex);

	if (file_p == NULL || file_p == stderr) 
    {
		pthread_mutex_unlock(&mlog_mutex);
		return NULL;
	} 
    else if (file_p != mfp_log) 
    {
		//if (mfp_log != stderr && mfp_log != stdout) 
        //{
		//	fclose(mfp_log);
		//}
		mfp_log = file_p;
		pthread_mutex_unlock(&mlog_mutex);
		return fp;
	}

	pthread_mutex_unlock(&mlog_mutex);
	return NULL;
}

int Logger::set_log_handler(const char* file) 
{
	FILE* fp = fopen(file, "a+");
	if(fp == NULL) 
    {
		return -1;
	}

	mfp_log = fp;
	return 0;
}

void Logger::logDebug(const char* file, int line, const char* func, const char* fmt, ...) 
{
	va_list vap;
	
	pthread_mutex_lock(&mlog_mutex);

	va_start(vap, fmt);
	log_trace(file, line, (int)mlog_level, func, fmt, vap);
	va_end(vap);

	pthread_mutex_unlock(&mlog_mutex);
	
}

void Logger::logPure(const char* fmt, ...) 
{	
	va_list vap;

	pthread_mutex_lock(&mlog_mutex);

	va_start(vap, fmt);
	log_trace_pure((int)mlog_level, fmt, vap);
	va_end(vap);

	pthread_mutex_unlock(&mlog_mutex);
	
}

void Logger::printLog(const char* fmt, ...) 
{	
	va_list vap;

	pthread_mutex_lock(&mlog_mutex);

	va_start(vap, fmt);
	log_trace_pure((int)LOG_LEVEL, fmt, vap);
	va_end(vap);

	pthread_mutex_unlock(&mlog_mutex);
	
}

void Logger::printWarning(const char* fmt, ...) 
{	
	va_list vap;

	pthread_mutex_lock(&mlog_mutex);

	va_start(vap, fmt);
	log_trace_pure((int)WARN_LEVEL, fmt, vap);
	va_end(vap);

	pthread_mutex_unlock(&mlog_mutex);
	
}

void Logger::printError(const char* fmt, ...) 
{	
	va_list vap;

	pthread_mutex_lock(&mlog_mutex);

	va_start(vap, fmt);
	log_trace_pure((int)ERROR_LEVEL, fmt, vap);
	va_end(vap);

	pthread_mutex_unlock(&mlog_mutex);
	
}

void Logger::get_cur_time(char cur[]) 
{
	time_t n = time(NULL);
	struct tm* p = localtime(&n);
	sprintf(cur, "%04d-%02d-%02d %02d:%02d:%02d", 
			p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
			p->tm_hour, p->tm_min, p->tm_sec);
}

void Logger::log_trace(const char* file, int line, int level, const char* func, const char* fmt, va_list vap) 
{
	if(level < 0 || level < (int)mlog_level || level >= (int)(sizeof(msg_psLevelName)) / (int)sizeof(const char*)) 
    {
		return;
	}
	char buffer[1024];
	char cur[20];
	get_cur_time(cur);

	int len = snprintf(buffer, 1024, "%s:%d %s() %s (%d/%X) [%s] ", file, line, func, cur, getpid(), (int)pthread_self(), msg_psLevelName[level]);
	if(len >= 1024)
    {
		buffer[1023] = '\n';
		len = 1024;
		//fwrite(buffer, 1, len, mfp_log);
		if (level == ERROR_LEVEL) 
        {
			fwrite(buffer, 1, len, mfp_log);
			if (mfp_log != stderr && mfp_log != stdout) 
            {
				fwrite(buffer, 1, len, stderr);
			}
		} 
        else 
        {
			fwrite(buffer, 1, len, mfp_log);
		}

		fflush(mfp_log);
		return ;
	}
	len += vsnprintf(buffer + len, 1024 - len, fmt, vap);
	if(len < 1023) 
    {
		buffer[len++] = '\n';
		buffer[len] = 0;
	} 
    else 
    {
		buffer[1024 - 1] = '\n';
		len = 1024;
	}
	//fwrite(buffer, 1, len, mfp_log);
	if (level == ERROR_LEVEL) 
    {
		fwrite(buffer, 1, len, mfp_log);
		if (mfp_log != stderr && mfp_log != stdout) 
        {
			fwrite(buffer, 1, len, stderr);
		}
	} 
    else 
    {
		fwrite(buffer, 1, len, mfp_log);
	}
	fflush(mfp_log);
}

void Logger::log_trace_pure(int level, const char* fmt, va_list vap) 
{
	if (level < 0 || level < (int)mlog_level || level >= (int)(sizeof(msg_psLevelName)) / (int)sizeof(const char*)) 
    {
		return;
	}

	char buffer[1024];
	char cur[20];
	get_cur_time(cur);
	static int abcdef = 0;
	abcdef++;
	int len = snprintf(buffer, 1024, "[%s] [%s] %5d ", cur, msg_psLevelName[level],  getpid());
	if(len >= 1024)
    {
		buffer[1023] = '\n';
		len = 1024;
		//fwrite(buffer, 1, len, mfp_log);
		if (level == ERROR_LEVEL) 
        {
			fwrite(buffer, 1, len, mfp_log);
			if (mfp_log != stderr && mfp_log != stdout) 
            {
				fwrite(buffer, 1, len, stderr);
			}
		} 
        else 
        {
			fwrite(buffer, 1, len, mfp_log);
		}

		fflush(mfp_log);
		return;
	}
	len += vsnprintf(buffer + len, 1024 - len, fmt, vap);
	if (len < 1023) 
    {
		buffer[len++] = '\n';
		buffer[len] = 0;
	} 
    else 
    {
		buffer[1024 - 1] = '\n';
		len = 1024;
	}
	//fwrite(buffer, 1, len, mfp_log);
	if (level == ERROR_LEVEL) 
    {
		fwrite(buffer, 1, len, mfp_log);
		if (mfp_log != stderr && mfp_log != stdout) 
        {
			fwrite(buffer, 1, len, stderr);
		}
	} 
    else 
    {
		fwrite(buffer, 1, len, mfp_log);
	}

	fflush(mfp_log);
}

void Logger::trace_pure(const char* fmt, va_list vap) 
{
	char buffer[1024];
	int len = 0;

	len = vsnprintf(buffer, 1024, fmt, vap);
	if(len < 1023) 
    {
		buffer[len++] = '\n';
		buffer[len] = 0;
	} 
    else 
    {
		buffer[1024 - 1] = '\n';
		len = 1024;
	}
	fwrite(buffer, 1, len, mfp_log);
	fflush(mfp_log);
}

}
