/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ALLOCATORUNREF_H_
#define ALLOCATORUNREF_H_

class Thread;
class AllocatorUnref {
public:
	//! stores away the thread the object is working for
	explicit AllocatorUnref(Thread *t);
	//! destroys the thread local store of the fThread
	~AllocatorUnref();

protected:
	//! the thread which is deleted
	Thread *fThread;
};

#endif /* ALLOCATORUNREF_H_ */
