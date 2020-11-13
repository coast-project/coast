/*
 * Copyright (c) 2020, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Semaphore.h"

#include "SystemLog.h"
#include "Threads.h"
#include "Tracer.h"

Semaphore::Semaphore(unsigned i_nCount) {
	StartTrace(Semaphore.Semaphore);
	if (!CREATESEMA(fSemaphore, i_nCount)) {
		SystemLog::Error("Sema create failed");
	}
}

Semaphore::~Semaphore() {
	StartTrace(Semaphore.~Semaphore);
	if (!DELETESEMA(fSemaphore)) {
		Trace("failed to close semaphore");
	}
}

bool Semaphore::Acquire() {
	StatTrace(Semaphore.Acquire, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId(), coast::storage::Current());
	return LOCKSEMA(fSemaphore);
}

bool Semaphore::TryAcquire() {
	StatTrace(Semaphore.TryAcquire, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId(), coast::storage::Current());
	return TRYSEMALOCK(fSemaphore);
}

int Semaphore::GetCount(int &count) {
	int iRet(GETSEMACOUNT(fSemaphore, count));
	StatTrace(Semaphore.GetCount,
			  "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId() << " count: " << count << " iRet: " << iRet,
			  coast::storage::Current());
	return iRet;
}

void Semaphore::Release() {
	StatTrace(Semaphore.Release, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId(), coast::storage::Current());
	if (!UNLOCKSEMA(fSemaphore)) {
		SystemLog::Error("UNLOCKSEMA failed");
	}
}

SemaphoreEntry::SemaphoreEntry(Semaphore &i_aSemaphore) : fSemaphore(i_aSemaphore) {
	fSemaphore.Acquire();
}

SemaphoreEntry::~SemaphoreEntry() {
	fSemaphore.Release();
}
