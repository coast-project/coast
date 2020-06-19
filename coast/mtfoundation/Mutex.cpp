/*
 * Copyright (c) 2020, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Mutex.h"

#include "Anything.h"
#include "CleanupHandler.h"
#include "InitFinisManager.h"
#include "SimpleMutex.h"
#include "StringStream.h"
#include "SystemLog.h"
#include "Threads.h"
#include "Tracer.h"
#include "boost_or_std/memory.h"
#include "singleton.hpp"

namespace {
	long fgMutexId = 0;

	class MutexInitializer {
		typedef boost_or_std::shared_ptr<SimpleMutex> SimpleMutexPtr;
		SimpleMutexPtr fMutexIdMutex;
		THREADKEY fCountTableKey;

	public:
		MutexInitializer() : fMutexIdMutex(new SimpleMutex("MutexIdMutex", coast::storage::Global())), fCountTableKey(0) {
			if (THRKEYCREATE(fCountTableKey, 0)) {
				SystemLog::Error("TlsAlloc of fCountTableKey failed");
			}
			InitFinisManager::IFMTrace("MutexCleaner::Initialized\n");
		}
		~MutexInitializer() {
			if (THRKEYDELETE(fCountTableKey) != 0) {
				SystemLog::Error("TlsFree of fCountTableKey failed");
			}
			InitFinisManager::IFMTrace("MutexCleaner::Finalized\n");
		}
		long incrementMutexId() {
			LockUnlockEntry aMtx(*fMutexIdMutex);
			return ++fgMutexId;
		}
		long decrementMutexId() {
			LockUnlockEntry aMtx(*fMutexIdMutex);
			return --fgMutexId;
		}
		THREADKEY getCountTableKey() const { return fCountTableKey; }
	};
	typedef coast::utility::singleton_default<MutexInitializer> MutexInitializerSingleton;
	/*! subclasses may be defined to perform cleanup in thread specific storage
		while thread is still alive. CleanupHandlers are supposed to be singletons.. */
	class MutexCountTableCleaner : public CleanupHandler {
	protected:
		//! method used to cleanup specific settings within thread specific storage
		virtual bool DoCleanup() {
			StatTrace(MutexCountTableCleaner.DoCleanup, "ThrdId: " << Thread::MyId(), coast::storage::Global());
			Anything *countarray = 0;
			if (GETTLSDATA(MutexInitializerSingleton::instance().getCountTableKey(), countarray, Anything)) {
				// as the countarray behavior changed, mutex entries which were used by the thread
				//  are still listed but should all have values of 0
				long lSize((*countarray).GetSize());
				bool bHadLockEntries = false;
				while (--lSize >= 0L) {
					if ((*countarray)[lSize].AsLong(-1L) > 0L) {
						bHadLockEntries = true;
					}
				}
				// trace errors only
				if (bHadLockEntries) {
					String strbuf(coast::storage::Global());
					OStringStream stream(strbuf);
					stream << "MutexCountTableCleaner::DoCleanup: ThrdId: " << Thread::MyId()
						   << "\n  countarray still contained Mutex locking information!\n";
					(*countarray).Export(stream, 2);
					stream.flush();
					SystemLog::WriteToStderr(strbuf);
				}
				delete countarray;
				countarray = 0;
				if (SETTLSDATA(MutexInitializerSingleton::instance().getCountTableKey(), countarray)) {
					return true;
				}
			}
			return false;
		}
	};
	MutexCountTableCleaner fgCleaner;
}  // namespace

Mutex::Mutex(const char *name, Allocator *a) : fMutexId(a), fLocker(-1), fName(name, -1, a) {
	fMutexId.Append(MutexInitializerSingleton::instance().incrementMutexId());
	int iRet = 0;
	if (!CREATEMUTEX(fMutex, iRet)) {
		SystemLog::Error(String("Mutex<").Append(fName).Append(">: CREATEMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> A"
										 << "\n");
#endif
}

Mutex::~Mutex() {
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> D"
										 << "\n");
#endif
	// cleanup countarray
	int iRet = 0;
	if (!DELETEMUTEX(fMutex, iRet)) {
		SystemLog::Error(String("Mutex<").Append(fName).Append(">: DELETEMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
}

long Mutex::GetCount() {
	Anything *countarray = 0;
	long lCount = 0L;
	GETTLSDATA(MutexInitializerSingleton::instance().getCountTableKey(), countarray, Anything);
	if (countarray) {
		StatTraceAny(Mutex.GetCount, (*countarray), "countarray", coast::storage::Current());
		lCount = ((ROAnything)(*countarray))[fMutexId].AsLong(0L);
	}
	StatTrace(Mutex.GetCount, "Count:" << lCount << " CallId: " << Thread::MyId(), coast::storage::Current());
	return lCount;
}

bool Mutex::SetCount(long newCount) {
	StatTrace(Mutex.SetCount, "CallId: " << Thread::MyId() << " newCount: " << newCount, coast::storage::Current());
	Anything *countarray = 0;
	GETTLSDATA(MutexInitializerSingleton::instance().getCountTableKey(), countarray, Anything);

	if (!countarray && newCount > 0) {
		countarray = new Anything(Anything::ArrayMarker(), coast::storage::Global());
		Thread::RegisterCleaner(&fgCleaner);
		if (!SETTLSDATA(MutexInitializerSingleton::instance().getCountTableKey(), countarray)) {
			SystemLog::Error("Mutex::SetCount: could not store recursive locking structure in TLS!");
			return false;
		}
	}
	if (countarray) {
		StatTraceAny(Mutex.SetCount, (*countarray), "countarray", coast::storage::Current());
		if (newCount <= 0) {
			(*countarray).Remove(fMutexId);
		} else {
			(*countarray)[fMutexId] = newCount;
			StatTraceAny(Mutex.SetCount, (*countarray), "Mutex::SetCount: Id[" << fMutexId << "] count: " << newCount,
						 coast::storage::Current());
		}
	}
	return true;
}

const String &Mutex::GetId() {
#if defined(WIN32) && defined(TRACE_LOCK_UNLOCK)
	fMutexIdTL = (long)fMutex;
	return fMutexIdTL;
#else
	return fMutexId;
#endif
}

void Mutex::Lock() {
	if (!TryLock()) {
		StatTrace(Mutex.Lock,
				  "First try to lock failed, eg. not recursive lock -> 'hard'-locking now. Id: " << GetId() << " CallId: "
																								 << Thread::MyId(),
				  coast::storage::Current());
		int iRet = 0;
		if (!LOCKMUTEX(fMutex, iRet)) {
			SystemLog::Error(
				String("Mutex<").Append(fName).Append(">: LOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
		}
#ifdef TRACE_LOCKS_IMPL
		if (fLocker != -1) {
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker
												 << "> overriding current locker!"
												 << "\n");
		}
#endif
		fLocker = Thread::MyId();
		SetCount(1);
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> L"
											 << "\n");
#endif
	} else {
		StatTrace(Mutex.Lock, "TryLock success, Id: " << GetId() << " CallId: " << Thread::MyId(), coast::storage::Current());
	}
}

void Mutex::Unlock() {
	if (fLocker == Thread::MyId()) {
		long actCount = GetCount() - 1;
		SetCount(actCount);
		StatTrace(Mutex.Unlock, "new lockcount:" << actCount << " Id: " << GetId() << " CallId: " << Thread::MyId(),
				  coast::storage::Current());
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TR"
											 << "\n");
#endif
		if (actCount <= 0) {
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> R"
												 << "\n");
#endif
			fLocker = -1;
			SetCount(0);
			int iRet = 0;
			if (!UNLOCKMUTEX(fMutex, iRet)) {
				SystemLog::Error(
					String("Mutex<").Append(fName).Append(">: UNLOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
			}
		}
	} else {
		String logMsg("[");
		logMsg << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << ">";
		logMsg << " Locking error: calling unlock while not holding the lock";
		SystemLog::Error(logMsg);
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(logMsg << "\n");
#endif
	}
}

bool Mutex::TryLock() {
	int iRet = 0;
	bool hasLocked = TRYLOCK(fMutex, iRet);
	StatTrace(Mutex.TryLock,
			  "Mutex " << (hasLocked ? "" : "not") << " locked Id: " << GetId() << " CallId: " << Thread::MyId()
					   << " Locker: " << fLocker << " Count: " << GetCount(),
			  coast::storage::Current());
	long lCount = 1;
#if defined(WIN32)
	if (hasLocked) {
		// WIN32 allows same thread to acquire the mutex more than once without deadlocking
		lCount = GetCount();
		if (lCount == 0) {
			// mutex acquired for the first time
			fLocker = Thread::MyId();
			++lCount;
			SetCount(lCount);
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL"
												 << "\n");
#endif
		} else {
			// consecutive acquire
			++lCount;
			SetCount(lCount);
			UNLOCKMUTEX(fMutex, iRet);
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[")
									 << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA"
									 << "\n");
#endif
		}
	} else if (fLocker == Thread::MyId()) {
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA"
											 << "\n");
#endif
		lCount += GetCount();
		SetCount(lCount);
		hasLocked = true;
	}
#else
	if (hasLocked) {
		fLocker = Thread::MyId();
		SetCount(lCount);
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL"
											 << "\n");
#endif
		StatTrace(Mutex.TryLock, "first lock, count:" << lCount << " Id: " << GetId() << " CallId: " << Thread::MyId(),
				  coast::storage::Current());
	} else {
		if (fLocker == Thread::MyId()) {
			lCount += GetCount();
			SetCount(lCount);
			hasLocked = true;
			StatTrace(Mutex.TryLock, "recursive lock, count:" << lCount << " Id: " << GetId() << " CallId: " << Thread::MyId(),
					  coast::storage::Current());
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[")
									 << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA"
									 << "\n");
#endif
		} else {
			if (iRet != EBUSY) {
				SystemLog::Error(
					String("Mutex<").Append(fName).Append(">: TRYLOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
			}
		}
	}
#endif
	return hasLocked;
}

void Mutex::ReleaseForWait() {
	StartTrace1(Mutex.ReleaseForWait, "Id: " << GetId() << " CallId: " << Thread::MyId());
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> RFW"
										 << "\n");
#endif
	fLocker = -1;
}

void Mutex::AcquireAfterWait() {
	StartTrace1(Mutex.AcquireAfterWait, "Id: " << GetId() << " CallId: " << Thread::MyId() << " fLocker: " << fLocker);
	fLocker = Thread::MyId();
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> AAW"
										 << "\n");
#endif
}
