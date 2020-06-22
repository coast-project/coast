/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AllocatorUnref.h"

#include "MT_Storage.h"
#include "SystemLog.h"
#include "Threads.h"

AllocatorUnref::AllocatorUnref(Thread *t) : fThread(t) {}

AllocatorUnref::~AllocatorUnref() {
	StatTrace(AllocatorUnref.~AllocatorUnref, "ThreadId: " << (fThread ? fThread->GetId() : 0) << " CallId: " << Thread::MyId(),
			  coast::storage::Global());
	if (fThread != 0) {
		Allocator *a = fThread->fAllocator;
		StatTrace(AllocatorUnref.~AllocatorUnref, "Allocator: " << (long)a, coast::storage::Global());
		if (a != 0) {
			if (a != coast::storage::Global() && (a->CurrentlyAllocated() > 0L)) {
				StatTrace(AllocatorUnref.~AllocatorUnref,
						  "currently allocated on " << (long)a << " " << (long)a->CurrentlyAllocated(),
						  coast::storage::Global());
				long lMaxCount = 3L;
				while ((a->CurrentlyAllocated() > 0L) && (--lMaxCount >= 0L)) {
					// give some 20ms slices to finish everything
					// it is in almost every case Trace-logging when enabled
					// normally only used in opt-wdbg or dbg mode
					SystemLog::WriteToStderr("#", 1);
					Thread::Wait(0L, 20000000);
				}
			}
			MT_Storage::UnrefAllocator(a);
			fThread->fAllocator = 0;
		}
	}
}
