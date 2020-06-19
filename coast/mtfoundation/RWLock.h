/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef RWLOCK_H_
#define RWLOCK_H_

#include "ITOStorage.h"
#include "ITOString.h"
#include "SystemAPI.h"

class RWLock {
public:
	enum eLockMode { eReading = 1, eWriting = 2 };
	/*! creates system dependent read/write lock
	  \param name a name to identify the mutex when tracing locking problems
	  \param a allocator used to allocate the storage for the name */
	RWLock(const char *name, Allocator *a = coast::storage::Global());
	//! deletes system dependent read/write lock
	~RWLock();
	//! locks system dependent lock for reading or writing according to the reading flag
	void Lock(eLockMode mode = eReading) const;
	//! releases system dependent lock for reading or writing according to the reading flag
	void Unlock(eLockMode mode = eReading) const;
	//! tries to lock system dependent lock for reading or writing according to the reading flag; bails out returning false
	//! without blocking if not possible
	bool TryLock(eLockMode mode = eReading) const;
	//! access the internal system dependent representation of the read/write lock
	RWLOCKPTR GetInternal() const { return GETRWLOCKPTR(fLock); }

private:
	//! handle to system dependent read/write lock
	RWLOCK fLock;
	//! the name of the read/write lock to ease debugging locking problems
	String fName;

	//! standard constructor prohibited
	RWLock();
	//! standard copy constructor prohibited
	RWLock(const RWLock &);
	//! standard assignment operator prohibited
	void operator=(const RWLock &);
};

#endif /* RWLOCK_H_ */
