/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MTStorageTest2.h"

#include "MT_Storage.h"
#include "SystemBase.h"
#include "TestSuite.h"
#include "Threads.h"
#if defined(WIN32)
#include <io.h>
#endif

class TestWorkerThread : public Thread {
protected:
	TestWorkerThread(Allocator *a) : Thread("TestWorkerThread", false, true, false, false, a) {}
};

class DataProviderThread : public TestWorkerThread {
	Allocator *fDataAllocator;
	Anything fData;

public:
	DataProviderThread(Allocator *a, Allocator *d) : TestWorkerThread(a), fDataAllocator(d), fData(d) {}

	Anything &GetData() { return fData; }

protected:
	virtual void Run() {
		long allocatedOnThreadAllocOnEnterRun = fAllocator->CurrentlyAllocated();
		long allocatedOnDataAllocOnEnterRun = fDataAllocator->CurrentlyAllocated();
		fData.Append("sfdsfdsfd");
		fData.Append(1432);
		fData.Append(2323.4343);
		long allocatedOnThreadAllocAfterDataWrite = fAllocator->CurrentlyAllocated();
		long allocatedOnDataAllocAfterDataWrite = fDataAllocator->CurrentlyAllocated();
		long allocatedOnThreadAllocAfterLocalWrite = 0;
		long allocatedOnDataAllocAfterLocalWrite = 0;
		long allocatedOnThreadAllocAfterLocalToDataAssign = 0;
		long allocatedOnDataAllocAfterLocalToDataAssign = 0;
		{
			Anything b;
			b["1"] = 1;
			b["2"] = "ok";
			b["3"] = "hhgfjhgfjhgf";
			b["4"] = 0.4334;
			allocatedOnThreadAllocAfterLocalWrite = fAllocator->CurrentlyAllocated();
			allocatedOnDataAllocAfterLocalWrite = fDataAllocator->CurrentlyAllocated();
			fData["Sub"] = b;
			allocatedOnThreadAllocAfterLocalToDataAssign = fAllocator->CurrentlyAllocated();
			allocatedOnDataAllocAfterLocalToDataAssign = fDataAllocator->CurrentlyAllocated();
		}
		long allocatedOnThreadAllocAfterLocalDestroy = fAllocator->CurrentlyAllocated();
		long allocatedOnDataAllocAfterLocalDestroy = fDataAllocator->CurrentlyAllocated();
		fData["Memstats"]["allocatedOnThreadAllocOnEnterRun"] = allocatedOnThreadAllocOnEnterRun;
		fData["Memstats"]["allocatedOnDataAllocOnEnterRun"] = allocatedOnDataAllocOnEnterRun;
		fData["Memstats"]["allocatedOnThreadAllocAfterDataWrite"] = allocatedOnThreadAllocAfterDataWrite;
		fData["Memstats"]["allocatedOnDataAllocAfterDataWrite"] = allocatedOnDataAllocAfterDataWrite;
		fData["Memstats"]["allocatedOnThreadAllocAfterLocalWrite"] = allocatedOnThreadAllocAfterLocalWrite;
		fData["Memstats"]["allocatedOnDataAllocAfterLocalWrite"] = allocatedOnDataAllocAfterLocalWrite;
		fData["Memstats"]["allocatedOnThreadAllocAfterLocalToDataAssign"] = allocatedOnThreadAllocAfterLocalToDataAssign;
		fData["Memstats"]["allocatedOnDataAllocAfterLocalToDataAssign"] = allocatedOnDataAllocAfterLocalToDataAssign;
		fData["Memstats"]["allocatedOnThreadAllocAfterLocalDestroy"] = allocatedOnThreadAllocAfterLocalDestroy;
		fData["Memstats"]["allocatedOnDataAllocAfterLocalDestroy"] = allocatedOnDataAllocAfterLocalDestroy;
	}
};

MTStorageTest2::MTStorageTest2(TString tname) : TestCaseType(tname), fGlobal(0), fPool(0) {
	StartTrace1(MTStorageTest2.Ctor, "ThrdId: " << Thread::MyId());
}

MTStorageTest2::~MTStorageTest2() {
	StartTrace1(MTStorageTest2.Dtor, "ThrdId: " << Thread::MyId());
}

void MTStorageTest2::setUp() {
	StartTrace1(MTStorageTest2.setUp, "ThrdId: " << Thread::MyId());
	// setting up a hardcoded debug context
	if (fGlobal == 0) {
		fGlobal = coast::storage::Global();
	}
	if (fPool == 0) {
		fPool = MT_Storage::MakePoolAllocator(2000, 25, 4711);
		MT_Storage::RefAllocator(fPool);
	}
}

void MTStorageTest2::tearDown() {
	StartTrace(MTStorageTest2.tearDown);
	MT_Storage::UnrefAllocator(fPool);
	fPool = 0;
}

void MTStorageTest2::trivialTest() {
	StartTrace(MTStorageTest2.trivialTest);
	t_assert(fGlobal != fPool);	 // allocators should be different
}

void MTStorageTest2::twoThreadTest() {
	StartTrace1(MTStorageTest2.twoThreadTest, "ThrdId: " << Thread::MyId());
	if (coast::storage::GetStatisticLevel() >= 1) {
		assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	}

	DataProviderThread *t1 = new (coast::storage::Global()) DataProviderThread(0, coast::storage::Global());
	t1->Start(fPool);
	// wait for thread to finish
	t1->CheckState(Thread::eTerminated);
	// copy memory stats
	Anything memstats = t1->GetData()["Memstats"];
	delete t1;

	if (coast::storage::GetStatisticLevel() >= 1) {
		TraceAny(memstats, "memory usage");
#if defined(POOL_STARTEDHOOK)
		assertComparem(0L, equal_to, memstats["allocatedOnThreadAllocOnEnterRun"].AsLong(),
					   "Pool Memory should have been left untouched so far");
#else
		assertComparem(
			0L, equal_to, memstats["allocatedOnThreadAllocOnEnterRun"].AsLong(),
			"everything should have been allocated within pool XXX: changes when semantic of Thread::eStarted changes!");
#endif
		assertComparem(static_cast<ul_long>(0), equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	}
}

void MTStorageTest2::twoThreadAssignmentTest() {
	StartTrace1(MTStorageTest2.twoThreadAssignmentTest, "ThrdId: " << Thread::MyId());
	ul_long l = fGlobal->CurrentlyAllocated();
	DataProviderThread *t1 = new (coast::storage::Global()) DataProviderThread(fPool, coast::storage::Global());
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	Anything y = t1->GetData();
	delete t1;

	// data should have been copied to global store now
	assertComparem(static_cast<ul_long>(0), equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in
	// coast::storage::GetStatisticLevel() >= 1
	if (coast::storage::GetStatisticLevel() >= 1) {
		assertCompare(fGlobal->CurrentlyAllocated(), greater, l);
	}
}

void MTStorageTest2::twoThreadCopyConstructorTest() {
	StartTrace1(MTStorageTest2.twoThreadCopyConstructorTest, "ThrdId: " << Thread::MyId());
	ul_long l = fGlobal->CurrentlyAllocated();
	DataProviderThread *t1 = new (coast::storage::Global()) DataProviderThread(fPool, coast::storage::Global());
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	Anything y(t1->GetData());	// should be equivalent to the use of operator=
	delete t1;

	// data should have been copied to global store now
	assertComparem(static_cast<ul_long>(0), equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in
	// coast::storage::GetStatisticLevel() >= 1
	if (coast::storage::GetStatisticLevel() >= 1) {
		assertCompare(fGlobal->CurrentlyAllocated(), greater, l);
	}
}

void MTStorageTest2::twoThreadArrayAccessTest() {
	StartTrace1(MTStorageTest2.twoThreadArrayAccessTest, "ThrdId: " << Thread::MyId());
	DataProviderThread *t1 = new (coast::storage::Global()) DataProviderThread(fPool, fPool);
	ul_long l = fGlobal->CurrentlyAllocated();
	{
		t1->Start(fPool);
		t1->CheckState(Thread::eRunning);
		assertCompare(l, equal_to, fGlobal->CurrentlyAllocated());

		// wait for other thread to finish
		t1->CheckState(Thread::eTerminated);
		assertCompare(l, equal_to, fGlobal->CurrentlyAllocated());
		// CAUTION: always pass a reference to long lived Anything to ROAnything! methods
		//          often return a temporary Anything which is *NOT* suitable to initialize
		//          a ROAnything...
		ROAnything sub(t1->GetData()["Sub"]["2"]);	// no copy should be necessary for access
		assertEqual("ok", sub.AsCharPtr(""));

		Anything copy(t1->GetData()["Sub"]["2"]);  // must be copied since allocators dont match
		// the current implementation allows size testing, eg. tracking of allocated and freed memory only in
		// coast::storage::GetStatisticLevel() >= 1
		if (coast::storage::GetStatisticLevel() >= 1) {
			assertCompare(fGlobal->CurrentlyAllocated(), greater, l);
		}
		delete t1;
	}
	// new we should be back where we started
	assertComparem(static_cast<ul_long>(0), equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
}

void MTStorageTest2::reusePoolTest() {
	StartTrace1(MTStorageTest2.reusePoolTest, "ThrdId: " << Thread::MyId());

	Allocator *pa = MT_Storage::MakePoolAllocator(3);
	t_assertm(pa != 0, "expected allocation of PoolAllocator to succeed");
	if (pa == 0) {
		return;
	}
	MT_Storage::RefAllocator(pa);  // need to refcount poolstorage
	assertEqualm(1L, pa->RefCnt(), "expected refcnt to be 1");
	DataProviderThread *t1 = new (coast::storage::Global()) DataProviderThread(pa, coast::storage::Global());
	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");

	// do some work
	t1->Start(pa);

	// wait for thread to finish
	t1->CheckState(Thread::eTerminated);
	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");
	delete t1;
	assertEqualm(1L, pa->RefCnt(), "expected refcnt to be 1");

	// do it again
	t1 = new (coast::storage::Global()) DataProviderThread(pa, coast::storage::Global());
	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");

	// do some work
	t1->Start(pa);
	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");
	delete t1;
	assertEqualm(1L, pa->RefCnt(), "expected refcnt to be 1");

	MT_Storage::UnrefAllocator(pa);	 // need to refcount poolstorage
}

Test *MTStorageTest2::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MTStorageTest2, trivialTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadAssignmentTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadCopyConstructorTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadArrayAccessTest);
	ADD_CASE(testSuite, MTStorageTest2, reusePoolTest);
	return testSuite;
}
