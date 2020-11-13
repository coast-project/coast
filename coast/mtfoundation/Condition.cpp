/*
 * Copyright (c) 2020, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Condition.h"

#include "Mutex.h"
#include "SystemLog.h"
#include "Threads.h"
#include "Tracer.h"

Condition::Condition() {
	if (!CREATECOND(fCondition)) {
		SystemLog::Error("CREATECOND failed");
	}
}

Condition::~Condition() {
	if (!DELETECOND(fCondition)) {
		SystemLog::Error("DELETECOND failed");
	}
}

int Condition::Wait(Mutex &m) {
	StatTrace(Condition.Wait, "releasing Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(),
			  coast::storage::Current());
	m.ReleaseForWait();
	if (!CONDWAIT(fCondition, m.GetInternal())) {
		SystemLog::Error("CONDWAIT failed");
	}
	StatTrace(Condition.Wait, "reacquired Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(),
			  coast::storage::Current());
	m.AcquireAfterWait();
	return 0;
}

int Condition::TimedWait(Mutex &m, long secs, long nanosecs) {
	StatTrace(Condition.TimedWait,
			  "releasing Mutex[" << m.fName << "] secs:" << secs << " nano:" << nanosecs << " Id: " << GetId()
								 << " CallId:" << Thread::MyId(),
			  coast::storage::Current());
	m.ReleaseForWait();
	int ret = CONDTIMEDWAIT(fCondition, m.GetInternal(), secs, nanosecs);
	StatTrace(Condition.Wait, "reacquired Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(),
			  coast::storage::Current());
	m.AcquireAfterWait();
	return ret;
}

int Condition::Signal() {
	StatTrace(Condition.Signal, "Id: " << GetId() << " CallId: " << Thread::MyId(), coast::storage::Current());
	return SIGNALCOND(fCondition);
}

int Condition::BroadCast() {
	StatTrace(Condition.BroadCast, "Id: " << GetId() << " CallId: " << Thread::MyId(), coast::storage::Current());
	return BROADCASTCOND(fCondition);
}

long Condition::GetId() {
	long lId = (long)GetInternal();
	StatTrace(Condition.GetId, lId, coast::storage::Current());
	return lId;
}
