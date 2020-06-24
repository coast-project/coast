/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Threads.h"

#include "CleanupHandler.h"
#include "DiffTimer.h"
#include "InitFinisManager.h"
#include "MT_Storage.h"
#include "SystemLog.h"
#include "singleton.hpp"
#if !defined(WIN32)
#include <errno.h>
#endif
#include <cstring>

//#define TRACE_LOCKS_IMPL 1
//#define POOL_STARTEDHOOK 1

namespace {
	long fgNumOfThreads = 0;
	long const deadSignature = 0xaaddeeff;
	long const aliveSignature = 0x0f0055AA;

	class ThreadInitializer {
		typedef boost_or_std::shared_ptr<SimpleMutex> SimpleMutexPtr;
		THREADKEY fCleanerKey;
		SimpleMutexPtr fNumOfThreadsMutex;

	public:
		ThreadInitializer() : fCleanerKey(0), fNumOfThreadsMutex(new SimpleMutex("NumOfThreads", coast::storage::Global())) {
			if (THRKEYCREATE(fCleanerKey, 0)) {
				SystemLog::Error("TlsAlloc of fCleanerKey failed");
			}
			InitFinisManager::IFMTrace("ThreadCleaner::Initialized\n");
		}
		~ThreadInitializer() {
			if (THRKEYDELETE(fCleanerKey) != 0) {
				SystemLog::Error("TlsFree of Thread::fCleanerKey failed");
			}
			InitFinisManager::IFMTrace("ThreadCleaner::Finalized\n");
		}
		void incrementNumOfThreads() {
			LockUnlockEntry me(*fNumOfThreadsMutex);
			++fgNumOfThreads;
		}
		void decrementNumOfThreads() {
			LockUnlockEntry me(*fNumOfThreadsMutex);
			--fgNumOfThreads;
		}
		long getNumberOfThreads() const {
			LockUnlockEntry me(*fNumOfThreadsMutex);
			return fgNumOfThreads;
		}
		THREADKEY getCleanerKey() const { return fCleanerKey; }
	};
	typedef coast::utility::singleton_default<ThreadInitializer> ThreadInitializerSingleton;
}  // namespace

#if defined(__APPLE__)	//!@FIXME check if this is still needed with most current version of framework
// notice: never delete this class or its internal structure!
// check with the base class FinalCleaner for understanding the sequence of deletion
// the CleanupInitializer instance will be deleted from within FinalCleaner::~FinalCleaner
class CleanupAllocator {
private:
	static ThreadInitializer *globalCleanupInitializer;

public:
	CleanupAllocator() { SystemLog::Info("----CREATED CleanupAllocator----"); }

	~CleanupAllocator() { SystemLog::Info("----DESTROYED CleanupAllocator----"); }

	static void initializeCleanupAllocator() {
		if (!globalCleanupInitializer) {
			globalCleanupInitializer = new ThreadInitializer();
		}
	}
};

CleanupInitializer *CleanupAllocator::globalCleanupInitializer = 0;
static CleanupAllocator fgInitKey;
#endif

Thread::Thread(const char *name, bool daemon, bool detached, bool suspended, bool bound, Allocator *a)
	: tObservableBase(name),
	  fAllocator(a),
	  fAllocCleaner(this),
	  fThreadId(0),
	  fParentThreadId(0),
	  fDaemon(daemon),
	  fDetached(detached),
	  fSuspended(suspended),
	  fBound(bound),
	  fStateMutex("ThreadStateMutex", coast::storage::Global()),

	  fRunningState(eReady),
	  fState(eCreated),
	  fSignature(deadSignature),
	  fThreadName(name, strlen(name), coast::storage::Global()),
	  fanyArgTmp(coast::storage::Global()) {
	StartTrace1(Thread.Constructor, "IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " Name ["
											  << GetName() << "]");
	if (fAllocator != 0) {
		fAllocator->Ref();
	}
	fParentThreadId = MyId();
}

Thread::~Thread() {
	StartTrace1(Thread.Destructor, "IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " Name ["
											 << GetName() << "]");

	// doesn't really solve the problem if a subclass would do sthg in the hook method
	// but at least it will go to the state eTerminated
	if ((fState < eTerminated) && !Terminate()) {
		SYSERROR("Terminate() failed");
	}

	// check if and who is still waiting on the state mutex if possible
	bool bDidLock(fStateMutex.TryLock());
	long lMaxCount = 10L;
	while ((!bDidLock || IsAlive()) && (--lMaxCount >= 0L)) {
		// aha, someone is still locking it!
		// try to give some time and then terminate finally even the mutex was not locked
		// give some 20ms slices to finish everything
		if (bDidLock) {
			fStateCond.TimedWait(fStateMutex, 0, 10000000);
			fStateMutex.Unlock();
			SystemLog::WriteToStderr("*", 1);
		} else {
			Thread::Wait(0L, 20000000);
			SystemLog::WriteToStderr("~", 1);
		}
		bDidLock = fStateMutex.TryLock();
	}
	if (bDidLock) {
		fStateMutex.Unlock();
	} else {
		SystemLog::Error(String("Thread::~Thread<").Append(GetName()).Append(">: fStateMutex was still in use!"));
	}
	if (IsAlive()) {
		SystemLog::Error(String("Thread::~Thread<")
							 .Append(GetName())
							 .Append(">: ThreadId: ")
							 .Append(GetId())
							 .Append(" did not terminate properly (state: ")
							 .Append((long)fState)
							 .Append("), still destructing it, you should check this!"));
	}
}

void Thread::SetName(const char *name) {
	fThreadName = name;
}

bool Thread::GetName(String &str) const {
	str = fThreadName;
	return (str.Length() > 0);
}

int Thread::Init(ROAnything args) {
	StartTrace(Thread.Init);
	return 0;
}

bool Thread::Start(Allocator *pAllocator, ROAnything args) {
	StartTrace1(Thread.Start, "IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	// prec: Thread *MUST NOT* already run!
	EThreadState aState(GetState(false, eStarted));
	if ((aState > eCreated) && (aState < eTerminated)) {
		SYSERROR("ThreadId: " << GetId() << " is still alive, eg. did not detach from system thread yet, exiting!");
	} else {
		// reset alive signature to allow re-use of object
		fSignature = aliveSignature;
		if (GetState() == eTerminated) {
			SetState(eCreated);
		}
		if (GetState() < eStartRequested) {
			Trace("pAllocator: " << (long)pAllocator << " fAllocator: " << (long)fAllocator);
			if ((pAllocator != 0) || (fAllocator != 0)) {
				if ((pAllocator != 0) && fAllocator != pAllocator) {
					Allocator *oldAlloc = fAllocator;
					fAllocator = pAllocator;
					// CAUTION: Thread *MUST NOT* have any instance variables
					// that still use the previous allocator!
					MT_Storage::UnrefAllocator(oldAlloc);
					MT_Storage::RefAllocator(fAllocator);
				}

				Assert(fAllocator);
				if (SetState(eStartRequested, args)) {
					bool *b = new bool[4];
					b[0] = fBound;
					b[1] = fDetached;
					b[2] = fDaemon;
					b[3] = fSuspended;
					bool ret = STARTTHREAD(this, b, &fThreadId, ThreadWrapper);
					if (ret) {
						if (fAllocator->GetId() == 0) {
							fAllocator->SetId((long)fThreadId);
						}
						SetState(eStartInProgress, args);
#if !defined(POOL_STARTEDHOOK)
						SetState(eStarted, args);  // XXX moved to IntRun
#endif
					} else {
						SYSERROR("could not start and attach system thread");
						fThreadId = 0;
						fSignature = deadSignature;
					}
					delete[] b;
					Trace("Start MyId(" << MyId() << ") started GetId( " << (long)fThreadId << ")");
					return ret;
				}
			} else {
				SystemLog::Error("No valid allocator supplied; no thread started");
			}
		} else {
			SystemLog::Error("Thread::Start has already been called");
		}
		SetState(eTerminationRequested);
	}
	return false;
}

void Thread::Exit(int) {
	// now we should be able to safely reset the fThreadId, needed only in case the Thread gets reused
	// not to trace a fThreadId which just finished to live
	// IMPORTANT: these values must be set BEFORE deleting and detaching the system thread
	// -> if not, the values will not get updated in 'this' object!

	// schedule cleanup of thread resources
	int iDetachCode(0);
	// even though pthread_detach will not work on non-joined threads (EINVAL), we will call the method
	// to cleanup resources on solaris/linux as it seems to do what expected
	if (((iDetachCode = THRDETACH(fThreadId)) != 0) && (iDetachCode != EINVAL)) {
		SYSERROR("pthread_detach failed: " << SystemLog::SysErrorMsg(iDetachCode));
	}
	StatTrace(Thread.Exit, "destroying system thread object id: " << GetId(), coast::storage::Global());
	// reset alive flag, must be set in Start() again!
	fSignature = deadSignature;
	fThreadId = 0;
	SetState(Thread::eTerminated);
}

void Thread::BroadCastEvent(Anything evt) {
	StatTrace(Thread.BroadCastEvent, "notifying observers", coast::storage::Current());
	NotifyAll(evt);
	fStateCond.BroadCast();
	StatTrace(Thread.BroadCastEvent, "state broadcasted", coast::storage::Current());
}

bool Thread::CheckState(EThreadState state, long timeout, long nanotimeout) {
	LockUnlockEntry me(fStateMutex);
	return IntCheckState(state, timeout, nanotimeout);
}

bool Thread::IntCheckState(EThreadState state, long timeout, long nanotimeout) {
	if (fState == state) {
		StatTrace(Thread.IntCheckState,
				  "State already reached! fState(" << (long)fState << ")==state(" << (long)state << ") IntId: " << GetId()
												   << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
		return true;
	}
	if (fState > state) {
		StatTrace(Thread.IntCheckState,
				  "State passed! fState(" << (long)fState << ")>state(" << (long)state << ") IntId: " << GetId()
										  << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
		return false;
	}
	if ((timeout != 0) || (nanotimeout != 0)) {
		DiffTimer dt;
		long militimeout = timeout * 1000L + nanotimeout / 1000000;
		while ((fState < state) && (dt.Diff() < (militimeout))) {
			StatTrace(Thread.IntCheckState,
					  "still waiting on fState(" << (long)fState << ")==state(" << (long)state << ") IntId: " << GetId()
												 << " CallId: " << MyId(),
					  coast::storage::Current());
			fStateCond.TimedWait(fStateMutex, timeout, nanotimeout);
		}
	} else {
		while (fState < state) {
			StatTrace(Thread.IntCheckState,
					  "still waiting on fState(" << (long)fState << ")==state(" << (long)state << ") IntId: " << GetId()
												 << " CallId: " << MyId(),
					  coast::storage::Current());
			fStateCond.Wait(fStateMutex);
		}
	}
	StatTrace(Thread.IntCheckState,
			  "fState(" << (long)fState << ")==state(" << (long)state << ") is " << ((fState == state) ? "true" : "false")
						<< " IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
			  coast::storage::Current());
	return (fState == state);
}

// a helper macro for debugging...
#define CHECKRUNNING_STATE_DBG(message)                                                                                      \
	StatTrace(Thread.CheckRunningState,                                                                                      \
			  (message) << " fRunningState: " << (long)fRunningState << " state: " << (long)state << " fState: "             \
						<< (long)fState << " realDiff: " << realDiff << " compDiff: " << compDiff << " Seconds: " << seconds \
						<< " NanoSeconds: " << nanoSeconds << " Timeout given: " << (timeout * 1000L),                       \
			  coast::storage::Current());

bool Thread::CheckRunningState(ERunningState state, long timeout, long nanotimeout) {
	LockUnlockEntry me(fStateMutex);
	return IntCheckRunningState(state, timeout, nanotimeout);
}
bool Thread::IntCheckRunningState(ERunningState state, long timeout, long nanotimeout) {
	StatTrace(Thread.IntCheckRunningState,
			  "waiting on fRunningState(" << (long)fRunningState << ")>=state(" << (long)state << ") IntId: " << GetId()
										  << " ParId: " << fParentThreadId << " CallId: " << MyId(),
			  coast::storage::Current());
	if (fState < eRunning) {
		StatTrace(Thread.CheckRunningState,
				  "not running yet, waiting until it is running. IntId: " << GetId() << " ParId: " << fParentThreadId
																		  << " CallId: " << MyId(),
				  coast::storage::Current());
		bool ret = IntCheckState(eRunning, timeout, nanotimeout);
		if (!ret) {
			StatTrace(Thread.CheckRunningState,
					  "timeout while waiting. IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
					  coast::storage::Current());
			return ret;
		}
	}

	if (fState > eRunning) {
		StatTrace(Thread.CheckRunningState,
				  "not running anymore. IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
		return false;
	}
	Assert(fState == eRunning);

	if (fRunningState == state) {
		StatTrace(Thread.CheckRunningState,
				  "State already reached! IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
		return true;
	}

	if ((timeout != 0) || (nanotimeout != 0)) {
		// In below code we have to consider the fact that TimedWait(t) may return
		// earlier then the specified t value. This may be caused by signalling the
		// condition (and then we still want to wait our timeout amount of time)
		// presumed fRunningState != state
		// More likely: Even if the condition is not signalled there are situations where
		// the underlying system calls used in DiffTimer and SimpleCondition differ
		// slightly in their time measurement. We have to consider that and recalculate
		// the wait.

		DiffTimer dt;
		long realDiff = timeout * 1000L + nanotimeout / 1000000;  // the time we like to wait, passed as argument to us
		long compDiff = 0;										  // the adjusted time we really have to wait
		while ((fRunningState != state) && (fState == eRunning)) {
			compDiff = dt.Diff();			 // get time passed since DiffTimer was instantiated
			compDiff = realDiff - compDiff;	 // new value to wait
			long nanoSeconds = 0;
			long int seconds = 0;
			if (compDiff <= 0) {
				CHECKRUNNING_STATE_DBG("Break.");
				break;
			}

			nanoSeconds = ((compDiff % 1000) * 1000000);
			seconds = (compDiff / 1000);
			CHECKRUNNING_STATE_DBG("Before CondTimedWait.");
			fStateCond.TimedWait(fStateMutex, seconds, nanoSeconds);
		}
	} else {
		while ((fRunningState != state) && (fState == eRunning)) {
			fStateCond.Wait(fStateMutex);
		}
	}
	return (fRunningState == state && fState == eRunning);
}

Thread::EThreadState Thread::GetState(bool trylock, EThreadState dfltState) {
	if (!trylock) {
		LockUnlockEntry me(fStateMutex);
		StatTrace(Thread.GetState,
				  "Locked access, fState:" << (long)fState << " IntId: " << GetId() << " ParId: " << fParentThreadId
										   << " CallId: " << MyId(),
				  coast::storage::Current());
		return fState;
	}
	if (fStateMutex.TryLock()) {
		StatTrace(Thread.GetState,
				  "Locked access, fState:" << (long)fState << " IntId: " << GetId() << " ParId: " << fParentThreadId
										   << " CallId: " << MyId(),
				  coast::storage::Current());
		dfltState = fState;
		fStateMutex.Unlock();
	} else {
		StatTrace(Thread.GetState,
				  "Lock failed, returning default:" << (long)dfltState << " IntId: " << GetId() << " ParId: " << fParentThreadId
													<< " CallId: " << MyId(),
				  coast::storage::Current());
	}
	return dfltState;
}

bool Thread::SetState(EThreadState state, ROAnything args) {
	bool bRetCode(false);
	LockUnlockEntry me(fStateMutex);
	if (state == fState + 1) {
		StatTrace(Thread.SetState,
				  "state(" << (long)state << ")==fState(" << (long)fState << ")+1 " << (long)state << " IntId: " << GetId()
						   << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ((state == eTerminationRequested) && (fState < eTerminationRequested)) {
		StatTrace(Thread.SetState,
				  "state(eTerminationRequested) && fState(" << (long)fState << ")<eTerminationRequested " << (long)state
															<< " IntId: " << GetId() << " ParId: " << fParentThreadId
															<< " CallId: " << MyId(),
				  coast::storage::Current());
		bRetCode = IntSetState(((fState == eCreated) || (fState == eStartRequested)) ? eTerminated : state, args);
	} else if ((fState == eTerminated) && (state == eCreated)) {
		StatTrace(Thread.SetState,
				  "fState(eTerminated) && state(eCreated) => re-use Thread "
					  << " IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ((state == eTerminatedRunMethod) && (MyId() == GetId())) {
		StatTrace(Thread.SetState,
				  "state==eTerminatedRunMethod && MyId==GetId IntId: " << GetId() << " ParId: " << fParentThreadId
																	   << " CallId: " << MyId(),
				  coast::storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ((state == eTerminated) && (MyId() == GetId())) {
		StatTrace(
			Thread.SetState,
			"state==eTerminated && MyId==GetId IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
			coast::storage::Current());
		bRetCode = IntSetState(state, args);
	} else if (state == eTerminated) {
		StatTrace(Thread.SetState,
				  "state==eTerminated => no-op! IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(),
				  coast::storage::Current());
	}
	return bRetCode;
}

bool Thread::IntSetState(EThreadState state, ROAnything args) {
	bool bRet = false;
	if (CallStateHooks(state, args)) {
		Anything anyEvt;
		anyEvt["this"] = static_cast<IFAObject *>(this);
		anyEvt["ParentThreadId"] = (long)fParentThreadId;
		anyEvt["ThreadId"] = GetId();
		anyEvt["ThreadState"]["Old"] = (long)fState;
		anyEvt["ThreadState"]["New"] = (long)state;
		if (!args.IsNull()) {
			anyEvt["Args"] = args.DeepClone();
		}
		fState = state;
		bRet = true;
		BroadCastEvent(anyEvt);
	}
	return bRet;
}

bool Thread::CallStateHooks(EThreadState state, ROAnything args) {
	switch (state) {
		case eStartRequested:
			StatTrace(Thread.CallStateHooks, "eStartRequested", coast::storage::Current());
			return DoStartRequestedHook(args);
		case eStartInProgress:
			StatTrace(Thread.CallStateHooks, "eStartInProgress", coast::storage::Current());
			fanyArgTmp = args.DeepClone(coast::storage::Global());
			break;
		case eStarted:
			StatTrace(Thread.CallStateHooks, "eStarted", coast::storage::Current());
			DoStartedHook(args);
			break;
		case eRunning:
			StatTrace(Thread.CallStateHooks, "eRunning", coast::storage::Current());
			DoRunningHook(args);
			break;
		case eTerminationRequested:
			StatTrace(Thread.CallStateHooks, "eTerminationRequested", coast::storage::Current());
			DoTerminationRequestHook(args);
			break;
		case eTerminatedRunMethod:
			StatTrace(Thread.CallStateHooks, "eTerminatedRunMethod", coast::storage::Current());
			DoTerminatedRunMethodHook();
			break;
		case eTerminated:
			StatTrace(Thread.CallStateHooks, "eTerminated", coast::storage::Global());
			DoTerminatedHook();
			break;
		default:;
	}
	return true;
}

bool Thread::DoStartRequestedHook(ROAnything) {
	return true;
};
void Thread::DoStartedHook(ROAnything){};
void Thread::DoRunningHook(ROAnything){};
void Thread::DoTerminationRequestHook(ROAnything){};
void Thread::DoTerminatedRunMethodHook(){};
void Thread::DoTerminatedHook(){};

bool Thread::SetRunningState(ERunningState state, ROAnything args) {
	bool bRetCode(false);
	LockUnlockEntry me(fStateMutex);
	// allocate things used before and after call to CallRunningStateHooks() on coast::storage::Global() because Allocator could
	// be refreshed during call
	StatTrace(Thread.SetRunningState, "-- entering -- CallId: " << MyId(), coast::storage::Current());

	if ((fState != eRunning) || (fRunningState == state)) {
		bRetCode = false;
	} else {
		ERunningState oldState = fRunningState;
		fRunningState = state;
		// after this call we potentially have refreshed Allocator
		CallRunningStateHooks(state, args);
		{
			// scoping to force compiler not to move around automatic variables which could make strange things happen
			Anything anyEvt;
			anyEvt["ThreadId"] = GetId();
			anyEvt["RunningState"]["Old"] = (long)oldState;
			anyEvt["RunningState"]["New"] = (long)state;
			if (!args.IsNull()) {
				anyEvt["Args"] = args.DeepClone();
			}
			bRetCode = true;
			BroadCastEvent(anyEvt);
		}
	}
	StatTrace(Thread.SetRunningState, "-- leaving --", coast::storage::Current());
	return bRetCode;
}

void Thread::CallRunningStateHooks(ERunningState state, ROAnything args) {
	switch (state) {
		case eReady:
			StatTrace(Thread.CallRunningStateHooks, "eReady", coast::storage::Current());
			DoReadyHook(args);
			break;
		case eWorking:
			StatTrace(Thread.CallRunningStateHooks, "eWorking", coast::storage::Current());
			DoWorkingHook(args);
		default:;
	}
}

void Thread::DoReadyHook(ROAnything){};
void Thread::DoWorkingHook(ROAnything){};

bool Thread::IsAlive() {
	return (fSignature == aliveSignature);
}

bool Thread::IsReady(bool &bIsReady) {
	bool bCouldLock(false);
	if (IsAlive()) {
		if (fStateMutex.TryLock()) {
			// now we have locked the mutex
			bIsReady = ((fState == eRunning) && (fRunningState == eReady));
			fStateMutex.Unlock();
			bCouldLock = true;
			StatTrace(Thread.IsReady, "ThrdId: " << GetId() << " CallId: " << MyId() << (bIsReady ? " true" : " false"),
					  coast::storage::Current());
		} else {
			StatTrace(Thread.IsReady,
					  "ThrdId: " << GetId() << " CallId: " << MyId() << (bIsReady ? " true" : " false")
								 << " (StateMutex was not lockable)!",
					  coast::storage::Current());
		}
	} else {
		bIsReady = false;
	}
	return bCouldLock;
}

bool Thread::IsWorking(bool &bIsWorking) {
	bool bCouldLock(false);
	if (IsAlive()) {
		if (fStateMutex.TryLock()) {
			// now we have locked the mutex
			bIsWorking = ((fState == eRunning) && (fRunningState == eWorking));
			fStateMutex.Unlock();
			bCouldLock = true;
			StatTrace(Thread.IsWorking, "ThrdId: " << GetId() << " CallId: " << MyId() << (bIsWorking ? " true" : " false"),
					  coast::storage::Current());
		} else {
			StatTrace(Thread.IsWorking,
					  "ThrdId: " << GetId() << " CallId: " << MyId() << (bIsWorking ? " true" : " false")
								 << " (StateMutex was not lockable)!",
					  coast::storage::Current());
		}
	} else {
		bIsWorking = false;
	}
	return bCouldLock;
}

bool Thread::IsRunning(bool &bIsRunning) {
	// assume that if we can't lock the mutex this Thread is not running
	bool bCouldLock(false);
	if (IsAlive()) {
		if (fStateMutex.TryLock()) {
			// now we have locked the mutex
			bIsRunning = (fState == eRunning);
			fStateMutex.Unlock();
			bCouldLock = true;
			StatTrace(Thread.IsRunning,
					  "fState:" << (long)fState << " ThrdId: " << GetId() << " CallId: " << MyId()
								<< (bIsRunning ? " true" : " false"),
					  coast::storage::Current());
		} else {
			StatTrace(Thread.IsRunning,
					  "fState:" << (long)fState << " ThrdId: " << GetId() << " CallId: " << MyId()
								<< (bIsRunning ? " true" : " false") << " (StateMutex was not lockable)!",
					  coast::storage::Current());
		}
	} else {
		bIsRunning = false;
	}
	return bCouldLock;
}

//:Try to set Ready state
bool Thread::SetReady(ROAnything args) {
	StatTrace(Thread.SetReady, "ThrdId: " << GetId() << " CallId: " << MyId(), coast::storage::Current());
	return SetRunningState(eReady, args);
}

//:Try to set Working state
bool Thread::SetWorking(ROAnything args) {
	StatTrace(Thread.SetWorking, "ThrdId: " << GetId() << " CallId: " << MyId(), coast::storage::Current());
	return SetRunningState(eWorking, args);
}

bool Thread::Terminate(long timeout, ROAnything args) {
	StartTrace1(Thread.Terminate, "Timeout: " << timeout << " ThrdId: " << GetId() << " CallId: " << MyId());
	bool bRet = true;
	SetState(eTerminationRequested, args);
	bRet = CheckState(eTerminated, timeout);
	return bRet;
}

void Thread::IntRun() {
	// IntId might be wrong since thread starts in parallel with the forking thread
	StatTrace(Thread.IntRun, "IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " --- entering",
			  coast::storage::Global());
#if defined(POOL_STARTEDHOOK)
	if (!CheckState(eStartInProgress))
#else
	if (!CheckState(eStarted))	// XXX remove
#endif
	{
		if (!SetState(eTerminatedRunMethod)) {
			StatTrace(Thread.IntRun, "SetState(eTerminatedRunMethod) failed MyId(" << MyId() << ") GetId( " << GetId() << ")",
					  coast::storage::Global());
		}
		return;
	}
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("Thr[") << fName << "]<" << Thread::MyId() << ">"
											<< "\n");
#endif
	StatTrace(Thread.IntRun, "Registering thread(fAllocator) MyId(" << MyId() << ") GetId( " << GetId() << ")",
			  coast::storage::Global());
	// adds allocator reference to thread local store
	MT_Storage::RegisterThread(fAllocator);

	// now call DoStartedHook which has access to threads allocator using coast::storage::Current()
#if defined(POOL_STARTEDHOOK)
	if (SetState(eStarted, fanyArgTmp) && CheckState(eStarted))
#endif
	{
		if (SetState(eRunning)) {
			ThreadInitializerSingleton::instance().incrementNumOfThreads();
			{
				String strWho(GetName(), coast::storage::Global());
				strWho.Append("::Run [").Append(MyId()).Append(']');
				MemChecker rekcehc(strWho, ((coast::storage::Current() != coast::storage::Global()) ? coast::storage::Current()
																									: (Allocator *)0));
				Run();
			}
			ThreadInitializerSingleton::instance().decrementNumOfThreads();
		} else {
			StatTrace(Thread.IntRun, "SetState(eRunning) failed MyId(" << MyId() << ") GetId( " << GetId() << ")",
					  coast::storage::Global());
		}
	}
	if (!SetState(eTerminatedRunMethod)) {
		StatTrace(Thread.IntRun, "SetState(eTerminatedRunMethod) failed MyId(" << MyId() << ") GetId( " << GetId() << ")",
				  coast::storage::Global());
	}
	StatTrace(Thread.IntRun, "IntId: " << GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " --- leaving",
			  coast::storage::Global());
}

void Thread::Wait(long secs, long nanodelay) {
	StatTrace(Thread.Wait, "secs:" << secs << " nano:" << nanodelay, coast::storage::Current());

	SimpleMutex m("ThreadWaitMutex", coast::storage::Global());
	SimpleMutex::ConditionType c;
	LockUnlockEntry me(m);
	int ret = 0;
	do {
		ret = c.TimedWait(m, secs, nanodelay);
	} while (ret != TIMEOUTCODE);
}

long Thread::MyId() {
	return THRID();
}

long Thread::NumOfThreads() {
	StartTrace(Thread.NumOfThreads);
	return ThreadInitializerSingleton::instance().getNumberOfThreads();
}

bool Thread::RegisterCleaner(CleanupHandler *handler) {
#if defined(__APPLE__)
	CleanupAllocator::initializeCleanupAllocator();
#endif
	StartTrace1(Thread.RegisterCleaner, "CallId: " << MyId());
	Anything *handlerList = 0;
	if (!GETTLSDATA(ThreadInitializerSingleton::instance().getCleanerKey(), handlerList, Anything)) {
		handlerList = new Anything(Anything::ArrayMarker(), coast::storage::Global());
		if (!SETTLSDATA(ThreadInitializerSingleton::instance().getCleanerKey(), handlerList)) {
			// failed: immediately destroy Anything to avoid leak..
			delete handlerList;
			SystemLog::Error("Thread::RegisterCleaner(CleanupHandler *) could not register cleanup handler");
			return false;  // giving up..
		}
	}
	(*handlerList).Append(Anything(handler, coast::storage::Global()));
	return true;
}

bool Thread::CleanupThreadStorage() {
	StatTrace(Thread.CleanupThreadStorage, "CallId: " << MyId() << " entering", coast::storage::Global());
	bool bRet = true;
	Anything *handlerList = 0;
	if (GETTLSDATA(ThreadInitializerSingleton::instance().getCleanerKey(), handlerList, Anything) && (handlerList != 0)) {
		for (long i = (handlerList->GetSize() - 1); i >= 0; --i) {
			CleanupHandler *handler = dynamic_cast<CleanupHandler *>((*handlerList)[i].AsIFAObject(0));
			if (handler != 0) {
				handler->Cleanup();
			}
		}
		delete handlerList;
		if (!SETTLSDATA(ThreadInitializerSingleton::instance().getCleanerKey(), 0)) {
			SystemLog::Error("Thread::CleanupThreadStorage(CleanupHandler *) could not cleanup handler");
			bRet = false;
		}
	}
	// unregister PoolAllocator of thread -> influences coast::storage::Current() retrieval
	MT_Storage::UnregisterThread();
	return bRet;
}
