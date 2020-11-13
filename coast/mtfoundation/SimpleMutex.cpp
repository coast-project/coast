/*
 * Copyright (c) 2020, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SimpleMutex.h"

#include "SystemLog.h"
#include "Threads.h"
#include "Tracer.h"

SimpleMutex::SimpleMutex(const char *name, Allocator *a) : fName(name, -1, a) {
	int iRet = 0;
	bool bRet(false);
	if (!(bRet = CREATEMUTEX(fMutex, iRet))) {
		SystemLog::Error(String("SimpleMutex<", coast::storage::Global())
							 .Append(fName)
							 .Append(">: CREATEMUTEX failed: ")
							 .Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.SimpleMutex,
			  "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " [" << fName << "] code:" << iRet
					<< (bRet ? " succeeded" : " failed"),
			  coast::storage::Current());
}

SimpleMutex::~SimpleMutex() {
	int iRet = 0;
	bool bRet(false);
	if (!(bRet = DELETEMUTEX(fMutex, iRet))) {
		SystemLog::Error(String("SimpleMutex<", coast::storage::Global())
							 .Append(fName)
							 .Append(">: DELETEMUTEX failed: ")
							 .Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.~SimpleMutex,
			  "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " [" << fName << "] code:" << iRet
					<< (bRet ? " succeeded" : " failed"),
			  coast::storage::Current());
}

void SimpleMutex::Lock() {
	int iRet = 0;
	bool bRet(false);
	if (!(bRet = LOCKMUTEX(fMutex, iRet))) {
		SystemLog::Error(String("SimpleMutex<", coast::storage::Global())
							 .Append(fName)
							 .Append(">: LOCKMUTEX failed: ")
							 .Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.Lock,
			  "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " code:" << iRet << (bRet ? " succeeded" : " failed"),
			  coast::storage::Current());
}

void SimpleMutex::Unlock() {
	int iRet = 0;
	bool bRet(false);
	if (!(bRet = UNLOCKMUTEX(fMutex, iRet))) {
		SystemLog::Error(String("SimpleMutex<", coast::storage::Global())
							 .Append(fName)
							 .Append(">: UNLOCKMUTEX failed: ")
							 .Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.Unlock, "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " code:" << iRet,
			  coast::storage::Current());
}

bool SimpleMutex::TryLock() {
	int iRet = 0;
	bool bRet = TRYLOCK(fMutex, iRet);
	StatTrace(SimpleMutex.TryLock,
			  "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " code:" << iRet << (bRet ? " succeeded" : " failed"),
			  coast::storage::Current());
	if (!bRet && (iRet != EBUSY)) {
		SystemLog::Error(String("SimpleMutex<", coast::storage::Global())
							 .Append(fName)
							 .Append(">: TRYLOCKMUTEX failed: ")
							 .Append(SystemLog::SysErrorMsg(iRet)));
	}
	return bRet;
}
