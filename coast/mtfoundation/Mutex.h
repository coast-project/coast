/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include "Condition.h"
#include "ITOStorage.h"
#include "ITOString.h"
#include "SystemAPI.h"

class Mutex {
	friend class Condition;
	friend class MutexInitializer;
	friend class ThreadsTest;
	friend class ContextTest;

public:
	typedef Condition ConditionType;
	/*! create mutex with names to ease debugging of locking problems
	  \param name a name to identify the mutex when tracing locking problems
	  \param a allocator used to allocate the storage for the name */
	Mutex(const char *name, Allocator *a = coast::storage::Global());
	~Mutex();

	/*! tries to acquire the mutex for this thread, blocks the caller if already locked (by another thread)
	  acquires the mutex for the calling thread; this is recursive, if you call lock twice for the same thread
	  a count is incremented
	  \note you should call unlock as many times as you called lock */
	void Lock();

	/*! releases the mutex */
	void Unlock();

	/*! tries to acquire the mutex if possible; bails out without locking if already locked
	  \return false when already locked */
	bool TryLock();
	/*! return status of lock without locking overhead, useful for
	  releasing, and reaquiring when lock */
	bool IsLockedByMe() { return GetCount() > 0; }
	/*! returns native representation of mutex hidden by macro MUTEXPTR
	  \return native representation of mutex */
	MUTEXPTR GetInternal() const { return GETMUTEXPTR(fMutex); }

	//! resets thread id before Wait on a condition is called
	void ReleaseForWait();
	//! restores thread id after returning from a wait call
	void AcquireAfterWait();

protected:
	//! returns thread specific mutex lock count
	long GetCount();
	//! sets thread specific mutex lock count
	bool SetCount(long);

	//! get mutex 'Id' (name) mainly for Tracing
	const String &GetId();

	//! this mutex unique id
	String fMutexId;
#if defined(WIN32) && defined(TRACE_LOCK_UNLOCK)
	String fMutexIdTL;
#endif

private:
	//! internal representation of mutex
	MUTEX fMutex;
	//! thread id of the lock holder
	long fLocker;

	//! the mutexs name, this is handy for tracing locking problems
	String fName;

	//! standard constructor prohibited
	Mutex();
	//! standard copy constructor prohibited
	Mutex(const Mutex &);
	//! standard assignement operator prohibited
	void operator=(const Mutex &);
};

#endif /* MUTEX_H_ */
