/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SysLogTest.h"
#include "SystemLog.h"
#include "SystemBase.h"
#include "TestSuite.h"
#include "Tracer.h"

void SysLogTest::setUp() {
	StartTrace(SysLogTest.setUp);
	SystemLog::Init("SysLogTest");
}

void SysLogTest::tearDown() {
	StartTrace(SysLogTest.tearDown);
	SystemLog::Terminate();
}

void SysLogTest::TestFlags() {
	StartTrace(SysLogTest.TestFlags);
	t_assertm(SystemLog::getSysLog() != NULL, " expected creation of fgSysLog");
}

namespace {
	const char * const MessagePrefix = "SYSLOGTEST: ";
	const time_t refTime = 1234567890;
	String getTimeStampPrefixString() {
		return coast::system::GenTimeStamp("%Y%m%d%H%M%S", true, refTime).Append(' ');
	}
	String formatMessageFixedTimeStamp(const char *level, const char *msg) {
		String finalMessage(128L);
		finalMessage.Append(getTimeStampPrefixString());
		finalMessage.Append(MessagePrefix).Append(msg).Append('\n');
		return finalMessage;
	}
}

struct TestSysLog: public SystemLog {
	String fMessage;
	virtual void DoTraceLevel(const char *level, const char *msg) {
		fMessage = DoFormatTraceLevelMessage(level, msg);
	}
	virtual void DoLogTrace(eLogLevel, const char *logMsg) {
		DoTraceLevel(MessagePrefix, logMsg);
	}
	virtual void DoSystemLevelLog(SystemLog::eLogLevel, const char*) {
	}
};

void SysLogTest::TestFormatter() {
	StartTrace(SysLogTest.TestFormatter);
	String logMessage("Message as error");
	SystemLog::Error(logMessage);
	SystemLog::SystemLogPtr mySysLog = SystemLog::SystemLogPtr(new TestSysLog());
	SystemLog::SystemLogPtr oldLogger = SystemLog::replaceSysLog(mySysLog);
	SystemLog::Error(logMessage);
	assertCharPtrEqual(String(MessagePrefix).Append(logMessage).Append('\n').cstr(), static_cast<TestSysLog*>(mySysLog.get())->fMessage.cstr());
	SystemLog::messageFormatterFunctionType oldFormatter = SystemLog::replaceMessageFormatter(&coast::system::log::formatMessageTimeStamp);
	SystemLog::Error(logMessage);
	assertCharPtrEqualm(String(MessagePrefix).Append(logMessage).Append('\n').cstr(), static_cast<TestSysLog*>(mySysLog.get())->fMessage.SubString(14+1).cstr(), "expected message at offset 15 as it is prefixed by a timestamp");
	SystemLog::replaceMessageFormatter(&formatMessageFixedTimeStamp);
	SystemLog::Error(logMessage);
	assertCharPtrEqual(getTimeStampPrefixString().Append(MessagePrefix).Append(logMessage).Append('\n').cstr(), static_cast<TestSysLog*>(mySysLog.get())->fMessage.cstr());
	SystemLog::replaceSysLog(oldLogger);
	SystemLog::Error(logMessage);
	SystemLog::replaceMessageFormatter(oldFormatter);
}

void SysLogTest::TestFormatterChangeByEnv() {
	StartTrace(SysLogTest.TestFormatter);
	String logMessage("Message as error");
	// prepare setting up environment
	String envVarSetting(coast::system::log::envnameLogonCerrWithTimestamp);
	envVarSetting.Append("=1");
	putenv(const_cast<char*>(envVarSetting.cstr()));
	// re-init syslog to re-evaluate environment variables
	SystemLog::Init("blub");
	// need to replace syslog instance to catch generated message
	SystemLog::SystemLogPtr mySysLog = SystemLog::SystemLogPtr(new TestSysLog());
	SystemLog::SystemLogPtr oldLogger = SystemLog::replaceSysLog(mySysLog);
	SystemLog::Error(logMessage);
	assertCharPtrEqualm(String(MessagePrefix).Append(logMessage).Append('\n').cstr(), static_cast<TestSysLog*>(mySysLog.get())->fMessage.SubString(14+1).cstr(), "expected message at offset 15 as it is prefixed by a timestamp");
	unsetenv(coast::system::log::envnameLogonCerrWithTimestamp);
	SystemLog::Init(0);
	oldLogger = SystemLog::replaceSysLog(mySysLog);
	SystemLog::Error(logMessage);
	assertCharPtrEqual(String(MessagePrefix).Append(logMessage).Append('\n').cstr(), static_cast<TestSysLog*>(mySysLog.get())->fMessage.cstr());
	SystemLog::replaceSysLog(oldLogger);
}

// builds up a suite of testcases, add a line for each testmethod
Test *SysLogTest::suite() {
	StartTrace(SysLogTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SysLogTest, TestFlags);
	ADD_CASE(testSuite, SysLogTest, TestFormatter);
	ADD_CASE(testSuite, SysLogTest, TestFormatterChangeByEnv);
	return testSuite;
}
