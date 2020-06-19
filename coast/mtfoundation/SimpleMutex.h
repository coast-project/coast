/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SIMPLEMUTEX_H_
#define SIMPLEMUTEX_H_

#include "ITOString.h"
#include "SimpleCondition.h"
#include "SystemAPI.h"

class SimpleMutex {
	friend class SimpleCondition;

public:
	typedef SimpleCondition ConditionType;
	/*! create mutex with names to ease debugging of locking problems
	  \param name a name to identify the mutex when tracing locking problems
	  \param a allocator used to allocate the storage for the name */
	SimpleMutex(const char *name, Allocator *a);
	~SimpleMutex();

	/*! tries to acquire the mutex for this thread, blocks the caller if already locked (by another thread)
	  acquires the mutex for the calling thread; this is NOT recursive, if you call lock twice for the same thread it deadlocks
	*/
	void Lock();

	/*! releases the mutex */
	void Unlock();

	/*! tries to acquire the mutex if possible; bails out without locking if already locked
	  \return false when already locked */
	bool TryLock();

	/*! returns native reprentation of mutex hidden by macro MUTEXPTR */
	MUTEXPTR GetInternal() const { return GETMUTEXPTR(fMutex); }

private:
	//! internal representation of mutex
	MUTEX fMutex;

	//! the mutexs name, this is handy for tracing locking problems
	String fName;

	//! standard constructor prohibited
	SimpleMutex();
	//! standard copy constructor prohibited
	SimpleMutex(const SimpleMutex &);
	//! standard assignement operator prohibited
	void operator=(const SimpleMutex &);
};

#endif /* SIMPLEMUTEX_H_ */
