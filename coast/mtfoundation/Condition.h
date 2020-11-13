/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef CONDITION_H_
#define CONDITION_H_

#include "SystemAPI.h"

class Mutex;
class Condition {
public:
	typedef Mutex MutexType;
	//! creates a system dependent condition variable
	Condition();
	//! destroys the system dependent condition variable
	~Condition();

	/*! waits for the condition to be signaled atomicly releasing the mutex while waiting and reaquiring it when returning from
	  the wait
	  \param m the mutex which is used with the condition; it must be locked, since it will be unlocked
	  \return system dependent return value; zero if call was ok */
	int Wait(MutexType &m);
	/*! waits for the condition to be signaled at most secs seconds and nanosecs nanoseconds if available on the plattform
	  \param m the mutex which is used with the condition; it must be locked, since it will be unlocked
	  \param secs timeout to wait in seconds
	  \param nanosecs timeout to wait in nanoseconds (10e-9)
	  \return system dependent return value; zero if call was ok */
	int TimedWait(MutexType &m, long secs, long nanosecs = 0);
	//! access the system dependent handle to the condition variable
	CONDPTR GetInternal() { return GETCONDPTR(fCondition); }
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
	COND fCondition;
};

#endif /* CONDITION_H_ */
