/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SIMPLECONDITION_H_
#define SIMPLECONDITION_H_

#include "SystemAPI.h"

class SimpleMutex;
class SimpleCondition {
public:
	typedef SimpleMutex MutexType;
	//! creates a system dependent condition variable
	SimpleCondition();
	//! destroys the system dependent condition variable
	~SimpleCondition();

	/*! waits for the condition to be signaled atomicly releasing the mutex while waiting and reaquiring it when returning from
	  the wait
	  \param m the mutex which is used with the condition; it must be locked, since it will be unlocked
	  \return system dependent return value; zero if call was ok */
	int Wait(MutexType &m);

	/*! waits for the condition to be signaled at most secs seconds and nanosecs nanoseconds if available on the plattform.
	  After this time we block on the SimpleMutex until we can lock it before leaving the function
	  \param m the mutex which is used with the condition; it must be locked, since it will be unlocked
	  \param secs timeout to wait in seconds
	  \param nanosecs timeout to wait in nanoseconds (10e-9)
	  \return system dependent return value; zero if call was ok */
	int TimedWait(MutexType &m, long secs, long nanosecs = 0);
	//! access the system dependent handle to the condition variable
	CONDPTR GetInternal() { return GETCONDPTR(fSimpleCondition); }
	//! signal the condition; do it while protected through the associated mutex, then unlock the mutex; zero or one waiting
	//! thread will be woken up
	int Signal();
	//! broadcasts the condition; do it while protected through the associated mutex, then unlock the mutex; all waiting thread
	//! will eventually be woken up
	int BroadCast();

private:
	//! get internal 'Id' mainly for Tracing
	long GetId();

	//! the system dependent condition variable handle
	COND fSimpleCondition;
};

#endif /* SIMPLECONDITION_H_ */
