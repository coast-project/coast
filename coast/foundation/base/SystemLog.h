/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SystemLog_H
#define _SystemLog_H

#include "ITOString.h"

namespace coast {
	namespace system {
		namespace log {
			//!< <tt>COAST_DOLOG</tt> controls the level of severities shown in the syslog. See below for possible values.
			const char * const envnameDoLog = "COAST_DOLOG";
			//!< <tt>COAST_LOGONCERR</tt> controls the level of severities shown on the console. See below for possible values.
			const char * const envnameLogonCerr = "COAST_LOGONCERR";
			//!< Enabling <tt>COAST_LOGONCERR_WITH_TIMESTAMP</tt> will prepend a timestamp to all messages logged to the console.
			const char * const envnameLogonCerrWithTimestamp = "COAST_LOGONCERR_WITH_TIMESTAMP";
			String formatMessage(const char *level, const char *msg);
			String formatMessageTimeStamp(const char *level, const char *msg);
		}
	}
}

/*! <b>API for syslog access</b>
This is the Coast system logging API. It is used for system-level and application-level logging.
The values of <tt>COAST_LOGONCERR</tt> and <tt>COAST_DOLOG</tt> control the level of severities shown either on the console or in the syslog. Possible values are:
<pre>
1: DEBUG
2: INFO
3: WARNING
4: ERROR
5: ALERT
</pre>
All messages with a severity above or equal the specified value will log onto the appropriate channel.<br>
The loggers default behavior is to write ALERT messages into syslog and ERROR and ALERT messages to the console.

Enabling <tt>COAST_LOGONCERR_WITH_TIMESTAMP</tt> will prepend a timestamp to all messages logged to the console.
Even more control over console message formatting is possible by defining your own formatter function and replacing it
in your code using SystemLog::replaceMessageFormatter(&myFormatterFunction). Check the SysLogTests to get an idea.
*/
class SystemLog {
	friend class SysLogTest;
public:
	typedef String (*messageFormatterFunctionType)(const char *, const char *);
	typedef boost_or_std::shared_ptr<SystemLog> SystemLogPtr;

	//! module initialization
	static void Init(const char *appId);

	//! module termination
	static void Terminate();

	static SystemLog* getSysLog();

	/*! define importancy levels in increasing order for easier */
	enum eLogLevel {
		eNone,
		eDEBUG,
		eINFO,
		eWARNING,
		eERR,
		eALERT,
		eLast
	};

	//! bottleneck routine used by the other methods, you can use severity levels directly
	static void Log(eLogLevel level, const char *msg);

	//! severity eDEBUG for tracing server activity during development and deployment
	static void Debug(const char *msg);

	//! severity eINFO for general information log general useful information about server activity
	static void Info(const char *msg);

	//! severity eWARNING	for information about inconsistent state or potential dangerous situation
	static void Warning(const char *msg);

	//! severity eERR for outright errors during operation of the server without fatal results
	static void Error(const char *msg);

	//! severity eALERT for fatal errors this call triggers also an alert on  the operator console
	static void Alert(const char *msg);

	//! for use by the Assert() macro defined in foundation.h
	static int LogAssert(const char *file, long line, const char *assertion);

	//! provides error messages as strings for errno values
	//! const char *SysErrorMsg(long errnum) maps the error number in errnum
	//! to an error  message  string,  and returns a pointer to that string.
	//! SysErrorMsg(long errnum) uses the same set of error messages as perror().
	//! The returned string should not be overwritten.
	static String SysErrorMsg(long errnum);

	// provides an error message for the last call that has set errno
	// const char *LastSysError() returns the error message retrieved with errno
	// from SysErrorMsg(errno)
	static String LastSysError();

	//!wrapper to write(2,...) on Unix systems to get things written to stderr
	//!needed, because some iostream implementations aren't thread safe
	static void WriteToStderr(const char *msg, long length = -1);
	static void WriteToStderr(const String &msg);

	//!wrapper to write(1,...) on Unix systems to get things written to stdout
	//!needed, because some iostream implementations aren't thread safe
	static void WriteToStdout(const char *msg, long length = -1);
	static void WriteToStdout(const String &msg);

	static SystemLog::messageFormatterFunctionType replaceMessageFormatter(SystemLog::messageFormatterFunctionType newFormatter);

protected:
	SystemLog() {
	}
	virtual ~SystemLog() {
	}

	//!template method
	virtual void DoLog(eLogLevel level, const char *msg);

	//!do a system dependent log call
	virtual void DoSystemLevelLog(eLogLevel level, const char *msg) = 0;

	//!write log messages to cerr with a level a string indicating the level if preprocessor flags are set accordingly
	virtual void DoLogTrace(eLogLevel level, const char *msg);

	//!write log messages to cerr if preprocessor flags are set accordingly
	virtual void DoTraceLevel(const char *level, const char *msg);

	//!format the message which we later log to the console
	virtual String DoFormatTraceLevelMessage(const char *level, const char *msg);

	static SystemLogPtr replaceSysLog(SystemLogPtr newLogger);
};

#define	SYSDEBUG(msg) \
	{ String strFileLineMsg(coast::storage::Current());\
	strFileLineMsg << "( " << __FILE__ << ":" << static_cast<long>(__LINE__) << " ) " << msg;\
	SystemLog::Debug(strFileLineMsg); }
#define	SYSINFO(msg) \
	{ String strFileLineMsg(coast::storage::Current());\
	strFileLineMsg << "( " << __FILE__ << ":" << static_cast<long>(__LINE__) << " ) " << msg;\
	SystemLog::Info(strFileLineMsg); }
#define	SYSWARNING(msg) \
	{ String strFileLineMsg(coast::storage::Current());\
	strFileLineMsg << "( " << __FILE__ << ":" << static_cast<long>(__LINE__) << " ) " << msg;\
	SystemLog::Warning(strFileLineMsg); }
#define	SYSERROR(msg) \
	{ String strFileLineMsg(coast::storage::Current());\
	strFileLineMsg << "( " << __FILE__ << ":" << static_cast<long>(__LINE__) << " ) " << msg;\
	SystemLog::Error(strFileLineMsg); }
#define	SYSALERT(msg) \
	{ String strFileLineMsg(coast::storage::Current());\
	strFileLineMsg << "( " << __FILE__ << ":" << static_cast<long>(__LINE__) << " ) " << msg;\
	SystemLog::Alert(strFileLineMsg); }


#endif		//not defined _SystemLog_H
