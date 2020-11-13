/*
 * Copyright (c) 2020, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SimpleCondition.h"

#include "SystemLog.h"
#include "Threads.h"
#include "Tracer.h"

SimpleCondition::SimpleCondition() {
	StatTrace(SimpleCondition.SimpleCondition, "", coast::storage::Current());
	if (!CREATECOND(fSimpleCondition)) {
		SystemLog::Error("CREATECOND failed");
	}
}

SimpleCondition::~SimpleCondition() {
	StatTrace(SimpleCondition.~SimpleCondition, "", coast::storage::Current());
	if (!DELETECOND(fSimpleCondition)) {
		SystemLog::Error("DELETECOND failed");
	}
}

int SimpleCondition::Wait(SimpleMutex &m) {
	StatTrace(SimpleCondition.Wait, "releasing SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(),
			  coast::storage::Current());
	if (!CONDWAIT(fSimpleCondition, m.GetInternal())) {
		SystemLog::Error("CONDWAIT failed");
	}
	StatTrace(SimpleCondition.Wait, "reacquired SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(),
			  coast::storage::Current());
	return 0;
}

int SimpleCondition::TimedWait(SimpleMutex &m, long secs, long nanosecs) {
	StatTrace(SimpleCondition.TimedWait,
			  "releasing SimpleMutex[" << m.fName << "] secs:" << secs << " nano:" << nanosecs << " CallId:" << Thread::MyId(),
			  coast::storage::Current());
	int iRet = CONDTIMEDWAIT(fSimpleCondition, m.GetInternal(), secs, nanosecs);
	StatTrace(SimpleCondition.TimedWait, "reacquired SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(),
			  coast::storage::Current());
	return iRet;
}

int SimpleCondition::Signal() {
	StatTrace(SimpleCondition.Signal, "", coast::storage::Current());
	return SIGNALCOND(fSimpleCondition);
}

int SimpleCondition::BroadCast() {
	StatTrace(SimpleCondition.BroadCast, "", coast::storage::Current());
	return BROADCASTCOND(fSimpleCondition);
}

long SimpleCondition::GetId() {
	long lId = (long)GetInternal();
	StatTrace(SimpleCondition.GetId, lId, coast::storage::Current());
	return lId;
}
