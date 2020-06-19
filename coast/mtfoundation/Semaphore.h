/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "SystemAPI.h"

class Semaphore {
public:
	Semaphore(unsigned i_nCount);
	~Semaphore();

	/*! Acquires the semaphore, eg. decreases the internal counter if it is greater than 0. This is equivalent to
	  Dijkstras DOWN method
	  This is a blocking call, if the the count is already 0, it blocks until someone releases it
	  \return true if count could be decreased */
	bool Acquire();

	/*! tries to acquire the semaphore.
	  Does the same as the Acquire() function except blocking if the count could not be decreased
	  \return true if count could be decreased, false if we could not or an error occured */
	bool TryAcquire();

	/*! Releases the semaphore, eg. increases the internal count and signals a waiting thread. This is equivalent to
	  Dijkstras UP method */
	void Release();

	/*! Returns the actual value of the semaphore without altering its value. If the seamphore is locked, the return value
	  is either zero or negative. The absolute value of the methods return code indicates the threads waiting for the
	  semaphore. */
	int GetCount(int &svalue);

	//! dummy method to prevent optimizing compilers from optimizing away unused variables
	void Use() const {}

private:
	SEMA fSemaphore;

	// disabled standard functions (move to public section if implemented)
	Semaphore(const Semaphore &rSemaphore);		  // copy constructor
	void operator=(const Semaphore &rSemaphore);  // assignment
};

//! helper class to Acquire and Release Semaphores automatically in scope
class SemaphoreEntry {
public:
	/*! Acquires the semaphore
	  \param i_aSemaphore the semaphore object to acquire and release */
	SemaphoreEntry(Semaphore &i_aSemaphore);

	//! Releases the semaphore.
	~SemaphoreEntry();

private:
	//! A reference to the related semaphore.
	Semaphore &fSemaphore;
};

#endif /* SEMAPHORE_H_ */
