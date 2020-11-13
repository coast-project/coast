/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _THREADS_H
#define _THREADS_H

#include "AllocatorUnref.h"
#include "Anything.h"
#include "IFAObject.h"
#include "LockUnlockEntry.h"
#include "ObserverIf.h"
#include "SimpleMutex.h"
#include "Tracer.h"

class CleanupHandler;
//! thread abstraction implementing its own thread state model using EThreadState and the available native thread api
/*!
this class implements the thread abstraction ( its own thread of control ) using the system dependent thread api available.<br>
To ease its use we have defined a state machine which let clients query a thread object about the state.<br>
With this means it is possible to reliably control starting and stopping of a thread */
class Thread : public NamedObject, public Observable<Thread, ROAnything>, public coast::AllocatorNewDelete {
	typedef Observable<Thread, ROAnything> tObservableBase;

public:
	// clang-format off
	/*! possible thread states; there is an implicit ordering in these states<br> eThreadCreated < eStartRequested < eStarted < eRunning < eTerminationRequested < eTerminated. Only well defined transitions are possible
		These states exist:<br>
		<b>eCreated</b><br>thread exists; but Start has not yet been called<p>
		<b>eStartRequested</b><br>Thread::Start has been called, thread is being initialized for start<p>
		<b>eStarted</b><br>Thread::Start has been called, thread was successfully initialized; the new thread of control is forked<p>
		<b>eRunning</b><br>Thread::Run has been entered in its own thread of control<p>
		<b>eTerminationRequested</b><br>thread is still in Thread::Run but somebody has termination signaled; thread will eventually terminate its own thread of control<p>
		<b>eTerminated</b><br>Thread::Run has been left and its own thread of control is terminated. The object still exists and can be restarted now<p>
		The following <b>state transitions</b> from current to next state are possible:<br>
		<table border=1 align=center>
		<tr><th align=center>CurrentState/NextState</th><th>eCreated</th><th>eStartRequested</th><th>eStarted</th><th>eRunning</th><th>eTerminationRequested</th><th>eTerminated</th></tr>
		<tr><td>eCreated</td><td align=center>1</td><td align=center>1</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td></tr>
		<tr><td>eStartRequested</td><td align=center>0</td><td align=center>1</td><td align=center>1</td><td align=center>0</td><td align=center>0</td><td align=center>0</td></tr>
		<tr><td>eStarted</td><td align=center>0</td><td align=center>0</td><td align=center>1</td><td align=center>1</td><td align=center>0</td><td align=center>0</td></tr>
		<tr><td>eRunning</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td><td align=center>1</td><td align=center>0</td></tr>
		<tr><td>eTerminationRequested</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td><td align=center>1</td></tr>
		<tr><td>eTerminated</td><td align=center>0</td><td align=center>1</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td></tr>
		</table> */
	// clang-format on
	enum EThreadState {
		eCreated,
		eStartRequested,
		eStartInProgress,
		eStarted,
		eRunning,
		eTerminationRequested,
		eTerminatedRunMethod,
		eTerminated
	};

	//! predefined possible running states for specialized application
	enum ERunningState {
		//! Thread is running and ready to do sthg.
		eReady,
		//! Thread is doing sthg.
		eWorking
	};

	/*! Constructor geared towards solaris thread api
	  \param name identify the thread with a more or less unique name, used for debugging etc
	  \param daemon sets the daemon flag
	  \param detached if set to true the thread is not joinable by other threads
	  \param suspended creates this thread in suspended mode
	  \param bound bounds this thread to a lwp
	  \param a Allocator to use for member allocation */
	Thread(const char *name, bool daemon = false, bool detached = true, bool suspended = false, bool bound = false,
		   Allocator *a = 0);

	/*! since it is not possible to terminate a thread in the superclass destructor, because most of the object has gone by that
	  time, we provide some infrastructure through the CheckState API to check and wait for certain events to happen */
	virtual ~Thread();

	/*! naming support getting name
	  \param str string which gets back the threads name
	  \return true in case the name was set */
	virtual bool GetName(String &str) const;

	/*! naming support getting name */
	virtual const char *GetName() const { return fThreadName; }

	/*! naming support setting name
	  \param name name to set */
	virtual void SetName(const char *name);

	//! custom initialization for array allocated threads
	virtual int Init(ROAnything args);

	/*! starts a thread, when this method returns the thread might or might not already be running
	  \param wdallocator the storage allocator used in the thread to be started
	  \param args the arguments given to the DoStartRequestedHook
	  \return true if the thread is about to start and fState == eStarted; false if DoStartRequestedHook returns false and fState == eTerminated */
	bool Start(Allocator *wdallocator = coast::storage::Global(), ROAnything args = ROAnything());

	/*! blocks the caller until thread has reached the requested state
	  \param state EThreadState to check if the thread has reached it
	  \param timeout timeout for wait in seconds
	  \param nanotimeout timeout to wait in nanoseconds (10e-9)
	  \return true if state was reached otherwise false */
	bool CheckState(EThreadState state, long timeout = 0, long nanotimeout = 0);

	/*! blocks the caller until thread has reached the requested state
	  \param state ERunningState to check if the thread has reached it
	  \param timeout timeout for wait in seconds
	  \param nanotimeout timeout to wait in nanoseconds (10e-9)
	  \return true if state was reached otherwise false */
	bool CheckRunningState(ERunningState state, long timeout = 0, long nanotimeout = 0);

	/*! returns the threads current state; if using trylock and lock is already set returns dfltState
	  \param trylock set to true if you want to use TryLock instead of blocking on the state mutex
	  \param dfltState set value which should be returned in case the TryLock fails
	  \return current state of thread or dfltState when TryLock fails */
	EThreadState GetState(bool trylock = false, EThreadState dfltState = eRunning);

	//! tests if the thread object is still valid
	bool IsAlive();

	/*! Test if in we are in state eReady
	  \param bIsReady will get the correct value only when the methods return code is true!
	  \return true in case we could retrieve the value, false otherwise */
	bool IsReady(bool &bIsReady);

	/*! Test if in we are in state eWorking
	  \param bIsWorking will get the correct value only when the methods return code is true!
	  \return true in case we could retrieve the value, false otherwise */
	bool IsWorking(bool &bIsWorking);

	/*! Test if in we are in state eRunning
	  \param bIsRunning will get the correct value only when the methods return code is true!
	  \return true in case we could retrieve the value, false otherwise */
	bool IsRunning(bool &bIsRunning);

	//! Try to set Ready state
	bool SetReady(ROAnything args = ROAnything());

	//! Try to set Working state
	bool SetWorking(ROAnything args = ROAnything());

	//! Try to set Working state
	template <class WorkerParamType>
	bool SetWorking(WorkerParamType workerArgs);

	/*! terminates the thread of control; blocks its caller
	  \param timeout the caller is blocked until the state eTerminated is reached or the timeout period has elapsed. If timeout==0 the timeout period is ignored
	  \param args the arguments to the termination
	  \return true if eTerminated was reached; false if timeout period has elapsed without reaching the state */
	bool Terminate(long timeout = 0, ROAnything args = ROAnything());

	//! returns the id of this thread
	long GetId() { return static_cast<long>(fThreadId); }

	//! blocks the calling thread for delay secs and nanodelay nanosecs
	static void Wait(long delay, long nanodelay = 0);

	//! returns id of the calling thread
	static long MyId();

	//! returns the number of active thread objects - still in IntRun() method
	static long NumOfThreads();

	//! allows clients to register an additional cleanup handler, (which is put into a list)
	static bool RegisterCleaner(CleanupHandler *);

#if defined(WIN32)
	friend void ThreadWrapper(void *);
#else
	friend void *ThreadWrapper(void *);
#endif
	friend class ThreadInitializer;

protected:
	//! sets internal states and calls Run
	void IntRun();

	/*! blocks the caller until thread has reached the requested state
	  \param state EThreadState to check if the thread has reached it
	  \param timeout timeout for wait in seconds
	  \param nanotimeout timeout to wait in nanoseconds (10e-9)
	  \return true if state was reached otherwise false */
	bool IntCheckState(EThreadState state, long timeout = 0, long nanotimeout = 0);

	/*! blocks the caller until thread has reached the requested state
	  \param state ERunningState to check if the thread has reached it
	  \param timeout timeout for wait in seconds
	  \param nanotimeout timeout to wait in nanoseconds (10e-9)
	  \return true if state was reached otherwise false */
	bool IntCheckRunningState(ERunningState state, long timeout = 0, long nanotimeout = 0);

	//! this method gets called from the threads callback function
	virtual void Run() = 0;

	//! broadcast an state change to the Observers and broadcast the fStateCond for clients initiating the state change
	void BroadCastEvent(Anything evt);

	//! dispatch to subclass hooks for thread state changes
	bool CallStateHooks(EThreadState state, ROAnything args);

	/*! Use current list of cleanup handlers to cleanup the thread specific storage and then reset to an empty list. This method
	  is the last point where we can modify state values of the thread terminating. After TLS cleanup is done, the state is set
	  to eTerminated and fThreadId will be reset to 0.
	  \return true in case of success, false otherwise */
	bool CleanupThreadStorage();

	/*! This hook gets called before an operating system thread will be started. One can decide - returning true or false - if
	  an operating system thread should really be started or not. This method is useful when we want to initialize memory
	  dependant things because the fAllocator member is now initialized. The GetId() function will not return a valid id because
	  no real thread was started yet.
	  \param args arguments passed when calling the Start() method
	  \return true in case the Start() method should continue to attach us to a real operating system thread, false otherwises */
	virtual bool DoStartRequestedHook(ROAnything args);

	/*! This hook gets called when an operating system thread could be started. The IntRun() method and its internal call to
	  Run() will appear after we leave this function so it is guaranteed that we do not need to lock anything which could be
	  used already in the Run() method. This method is useful when we want to initialize memory dependant things because the
	  fAllocator member is now initialized. In addition - to DoStartRequestedHook - the GetId() function also returns a valid
	  threadid.
	  \param args arguments passed when calling the Start() method */
	virtual void DoStartedHook(ROAnything args);

	//! subclass hook
	virtual void DoRunningHook(ROAnything args);

	//! subclass hook
	virtual void DoTerminationRequestHook(ROAnything args);

	/*! subclass hook to catch event when thread specific (derived) Run() method returns */
	virtual void DoTerminatedRunMethodHook();

	/*! subclass hook to catch event when IntRun() method returns, eg. OS specific thread terminates
	  \note From now on, every Run() related code has finished executing */
	virtual void DoTerminatedHook();

	//! dispatch to subclass hooks for running state changes
	void CallRunningStateHooks(ERunningState state, ROAnything args);

	//! subclass hook
	virtual void DoReadyHook(ROAnything);

	//! subclass hook
	virtual void DoWorkingHook(ROAnything);

	//! allocator for thread local storage
	Allocator *fAllocator;

	//! 1st instance var (i.e. is destroyed last) used to unregister allocator *after* its no longer required
	AllocatorUnref fAllocCleaner;

	//! system dependent thread id
	THREAD fThreadId;
	//! thread id of parent which constructed us
	long fParentThreadId;

	//! solaris like thread flags; not very useful on all systems
	bool fDaemon, fDetached, fSuspended, fBound;

	// deletes running thread (but not the object)
	void Exit(int code);

	// some synchronization variables to make life easier for thread users guard for fState flag
	SimpleMutex fStateMutex;

	//! synchronization condition for checking thread states
	SimpleCondition fStateCond;

private:
	/*! internal method implementing the state transition matrix
	  \param state the next state wished
	  \param args the arguments for the transition
	  \return true if transition was possible; else false
	  \note this internal method is not to be touched, it implements the state transition matrix and calls IntSetState if ok */
	bool SetState(EThreadState state, ROAnything args = ROAnything());

	/*! internal method setting the state and calling the hook methods
	  \param state the next state accepted
	  \param args the arguments for the transition forwarded to the hook methods
	  \return returns always true apart from the DoStartRequestHook that can abort the transition
	  \note internal method; it calls the state hook with the new state and the args<br>if successful it changes the state to the new state and Broadcasts the state change to observers and waiters of the state condition */
	bool IntSetState(EThreadState state, ROAnything args = ROAnything());

	/*! internal method setting the running state
	  \param state the running state wished
	  \param args the arguments to the state change
	  \return returns true if the state change was possible; returns false if not in EThreadState::eRunning or the running state is already the state wished
	  \note internal method setting the running state; if successful it broadcasts the state change to observers and waiters of the state condition */
	bool SetRunningState(ERunningState state, ROAnything args);

	/*! internal method setting the running state
	  \param state the running state wished
	  \param args the arguments to the state change
	  \return returns true if the state change was possible; returns false if not in EThreadState::eRunning or the running state is already the state wished
	  \note internal method setting the running state; if successful it broadcasts the state change to observers and waiters of the state condition */
	template <class WorkerParamType>
	bool SetRunningState(ERunningState state, WorkerParamType args);

	//! disallow clones for now
	/*! @copydoc IFAObject::Clone(Allocator *) const */
	IFAObject *Clone(Allocator *a) const { return (IFAObject *)0; }

	//! state variable defining the threads current running state
	ERunningState fRunningState;
	//! state variable defining the threads state
	EThreadState fState;

	//! signature to check alive state of thread even the object itself got destructed
	long fSignature;

	//! the name of the thread
	String fThreadName;

	// a temporary placeholder for passing arguments to DoStartedHook
	Anything fanyArgTmp;

	friend class AllocatorUnref;
#if defined(WIN32)
	friend BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID);
	friend void TerminateKilledThreads();
#endif
};

template <class WorkerParamType>
bool Thread::SetWorking(WorkerParamType workerArgs) {
	StatTrace(Thread.SetWorking, "ThrdId: " << GetId() << " CallId: " << MyId(), coast::storage::Current());
	return SetRunningState(eWorking, workerArgs);
}

template <class WorkerParamType>
bool Thread::SetRunningState(ERunningState state, WorkerParamType args) {
	LockUnlockEntry me(fStateMutex);

	// allocate things used before and after call to CallRunningStateHooks() on coast::storage::Global() because Allocator could
	// be refreshed during call

	StatTrace(Thread.SetRunningState, "-- entering -- CallId: " << MyId(), coast::storage::Current());

	if (fState != eRunning || fRunningState == state) {
		return false;
	}

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
		BroadCastEvent(anyEvt);
	}
	StatTrace(Thread.SetRunningState, "-- leaving --", coast::storage::Current());
	return true;
}

#endif
