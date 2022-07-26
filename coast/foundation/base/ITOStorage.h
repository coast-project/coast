/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ITOStorage_H
#define _ITOStorage_H

#include "boost_or_std/memory.h"
#include "foundation.h"	 //!< for definition of own types

#include <cstddef>
#include <vector>

namespace itostorage {
	//! adaption of storage::Global / storage::Current for boost::poolXXX usage
	struct BoostPoolUserAllocatorGlobal {
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char *malloc(const size_type bytes);
		static void free(char *const block);
	};

	struct BoostPoolUserAllocatorCurrent {
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char *malloc(const size_type bytes);
		static void free(char *const block);
	};
}  // namespace itostorage

struct MemoryHeader;
//! Base class for memory allocation tracking
/*! helper class for debugging memory management problems */
class MemTracker {
	friend struct MemoryHeader;
	friend class MemTrackerTest;

public:
	typedef std::vector<MemoryHeader *> UsedListType;
	typedef UsedListType *UsedListTypePtr;

	explicit MemTracker(const char *name);
	virtual ~MemTracker();

	//! tracks allocation; chunk allocated has allocSz
	virtual void TrackAlloc(MemoryHeader *mh);

	//! tracks free; chunk freed has allocSz
	virtual void TrackFree(MemoryHeader *mh);

	//! print out still allocated memory blocks
	void DumpUsedBlocks();

	//! prints statistic of allocated and freed bytes
	virtual void PrintStatistic(long lLevel);

	//! returns currently allocated bytes
	ul_long CurrentlyAllocated() const { return fAllocated; }

	//! returns peak allocated bytes
	ul_long PeakAllocated() const { return fMaxAllocated; }

	//! sets the id of the allocator to be tracked
	void SetId(long);

	//! gets the Trackers/Allocators id
	long GetId() const { return fId; }

	//! returns the name of the Tracker
	const char *GetName() const { return fpName; }

protected:
	l_long fAllocated,		 //!< tracks the currently allocated size in bytes, might get negative in erroneous situations
		fMaxAllocated;		 //!< tracks the peek allocated size in bytes
	ul_long fSizeAllocated,	 //!< total bytes allocated
		fSizeFreed;			 //!< total bytes freed
	ul_long fNumAllocs,		 //!< tracks the number of allocations
		fNumFrees;			 //!< tracks the number of total frees

private:
	MemTracker();
	MemTracker(const MemTracker &);
	MemTracker &operator=(const MemTracker &);

	//! the id of the pool we track
	long fId;
	//! the name of the pool we track
	char *fpName;
	//! list to store used MemoryHeaders
	UsedListTypePtr fpUsedList;
};

class NullMemTracker : public MemTracker {
public:
	explicit NullMemTracker(const char *);
	~NullMemTracker();
	//! tracks allocation; chunk allocated has allocSz
	virtual void TrackAlloc(MemoryHeader *) {}

	//! tracks free; chunk freed has allocSz
	virtual void TrackFree(MemoryHeader *) {}

	//! prints statistic of allocated and freed bytes
	virtual void PrintStatistic(long) {}
};

//! helper class to check for memory leaks
class MemChecker {
	MemChecker(const MemChecker &);
	MemChecker &operator=(const MemChecker &);

public:
	//! check allocation in in a certain scope for one allocator
	//! we do not use String on purpose here
	MemChecker(const char *scope, class Allocator *a);
	~MemChecker();

	//! print delta bytes of allocation since the constructor was run on cerr
	void TraceDelta(const char *message);

	//! returns delta bytes of allocation since the constructor was run
	l_long CheckDelta();

protected:
	//! the allocator which is checked
	Allocator *fAllocator;

	//! the number of bytes allocated minus number of bytes freed
	l_long fSizeAllocated;

	//! message to identify the scope the check was executed
	const char *fScope;
};

#define StartTraceMem(scope) MemChecker rekcehc(_QUOTE_(scope), coast::storage::Current())
#define TraceMemDelta(message) rekcehc.TraceDelta(message)

#define StartTraceMem1(scope, allocator) MemChecker rekcehc1(_QUOTE_(scope), allocator)
#define TraceMemDelta1(message) rekcehc1.TraceDelta(message)

#define PoolTrackStatTriggered(trigger, pAlloc, level) \
	if (TriggerEnabled(trigger)) {                     \
		(pAlloc)->PrintStatistic(level);               \
	}

//! Base class for memory allocation policies
class Allocator {
public:
	typedef boost_or_std::shared_ptr<MemTracker> MemTrackerPtr;
	explicit Allocator(long allocatorid);
	virtual ~Allocator();

	//! analogous api to built in c function calloc
	void *Calloc(int n, size_t size);

	template <size_t SIZE>
	void *Malloc() {
		return Malloc(SIZE);
	}

	template <size_t SIZE>
	void Free(void *p) {
		Free(p, SIZE);
	}

	//! analogous api to built in c function malloc
	virtual void *Malloc(size_t size);

	//! analogous api to built in c function free
	virtual void Free(void *vp, size_t sz) = 0;

	//! analogous api to built in c function free
	virtual void Free(void *vp) = 0;

	//! refcounting support
	virtual void Ref() { ++fRefCnt; }
	//! refcounting support
	virtual void Unref() { --fRefCnt; }
	//! refcounting support
	virtual long RefCnt() const { return fRefCnt; }

	//! hook to allow allocators to optimize allocation of string buffers
	virtual size_t SizeHint(size_t size) { return size; }

	/*! set an identification for this pool
	 \param lId identification for pool
	 \return old identifier */
	virtual long SetId(long const lId) {
		long const lOldId = fAllocatorId;
		fAllocatorId = lId;
		return lOldId;
	}

	/*! get identifier of this pool
	 \return identifier of this pool */
	virtual long GetId() { return fAllocatorId; }

	//! Memory debugging and tracking support; implementer should report currently allocated bytes
	virtual ul_long CurrentlyAllocated() = 0;

	//! change of memtrackers to be e.g. MT-Safe
	virtual MemTrackerPtr ReplaceMemTracker(MemTrackerPtr t) {
		std::swap(fTracker, t);
		return t;
	}

	//! Retrieve current memory tracker in charge
	virtual MemTrackerPtr CurrentMemTracker() { return fTracker; }

	//! Memory debugging and tracking support; implementer should report all statistic on cerr
	virtual void PrintStatistic(long lLevel = -1) = 0;

	//! hook method to reorganize the managed memory
	virtual void Refresh();

protected:
	//! hook for allocation of memory
	virtual void *Alloc(size_t allocSize) = 0;

	// TODO The methods AllocSize, ExtMemStart and RealMemStart would be better moved to MemoryHeader

	//! calculates the memory needed
	//! \param n number of objects needed
	//! \param size size of the object
	virtual size_t AllocSize(size_t n, size_t size);

	//! calculates the offset into a buffer from the beginning of a managed piece of memory; clients should not care about the
	//! memory header before ExtMemStart
	virtual void *ExtMemStart(void *vp);

	//! calculates the real start of the memory managed from vp, assuming it contains a MemoryHeader
	virtual MemoryHeader *RealMemStart(void *vp);

	//! identification of the allocator
	long fAllocatorId;

	//! reference count
	long fRefCnt;

	//! tracks allocation and deallocation of memory
	MemTrackerPtr fTracker;
};

//! manages storage using the built-in C API and does some statistic
class GlobalAllocator : public Allocator {
	GlobalAllocator(const GlobalAllocator &);
	GlobalAllocator &operator=(const GlobalAllocator &);

public:
	GlobalAllocator();

	//! frees memory allocated by global allocator
	virtual void Free(void *vp);

	//! frees memory allocated by global allocator
	virtual void Free(void *vp, size_t sz);

	//! reference counting is disabled global allocator always exists (but only once)
	virtual long RefCnt() const { return 1; }

	//! print out the allocators statistics
	virtual void PrintStatistic(long lLevel = -1);

	//! returns the currently allocated bytes
	ul_long CurrentlyAllocated();

	//! replaces the memory tracker with sthg. different e.g. thread safe
	virtual MemTrackerPtr ReplaceMemTracker(MemTrackerPtr t);

protected:
	//! implements allocation bottleneck routine
	virtual void *Alloc(size_t allocSize);
	friend class MemChecker;
};

class StorageHooks;

namespace coast {
	namespace memory {
		template <typename T, typename U = long double>
		struct AlignedSize {
			struct X {
				T a;
				U b;
			};
			static size_t const alignment = (sizeof(T) < offsetof(X, b)) ? offsetof(X, b) : sizeof(T);
			static const size_t value = sizeof(T) + ((sizeof(T) % sizeof(U)) != 0U ? (sizeof(U) - sizeof(T) % sizeof(U)) : 0);
		};

		template <typename T>
		size_t calculateAllocationSize(size_t const sz) {
			return AlignedSize<T>::alignment + sz;
		}

		template <typename T>
		T &allocatorFor(void *ptr) COAST_NOEXCEPT_OR_NOTHROW {
			return *(reinterpret_cast<T *>(ptr));
		}

		template <typename T>
		void *payloadPtrFor(void *ptr) COAST_NOEXCEPT_OR_NOTHROW {
			return reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + AlignedSize<T>::alignment);
		}

		template <typename T>
		void *realPtrFor(void *ptr) COAST_NOEXCEPT_OR_NOTHROW {
			return reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) - AlignedSize<T>::alignment);
		}

		void safeFree(Allocator *a, void *ptr) COAST_NOEXCEPT_OR_NOTHROW;
	}  // namespace memory

	namespace storage {
		typedef boost_or_std::shared_ptr<StorageHooks> StorageHooksPtr;

		//! prints memory management statistics
		void PrintStatistic(long lLevel = -1);

		//! get the global allocator
		Allocator *Global();

		//! get currently used allocator (thread specific)
		Allocator *Current();

		/*! get statistic level which was initialized by getting value of COAST_TRACE_STORAGE environment variable
		 \return logging level, see description of fglStatisticLevel */
		long GetStatisticLevel();

		//! used by mt system to redefine the hooks for mt-local storage policy
		void registerHook(coast::storage::StorageHooksPtr h);

		//! used by mt system to redefine the hooks for mt-local storage policy
		coast::storage::StorageHooksPtr unregisterHook();

		//! temporarily disable thread local storage policies e.g. to reinitialize server
		struct ForceGlobalStorageEntry {
			ForceGlobalStorageEntry();
			~ForceGlobalStorageEntry();
		};

		//! access the global allocator
		Allocator *DoGlobal();

		/*! allocate a memory tracker object
		  \param name name of the tracker
		  \param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
		  \return pointer to a newly created MemTracker object */
		Allocator::MemTrackerPtr MakeMemTracker(const char *name, bool bThreadSafe);

		//! factory method to allocate MemTracker
		Allocator::MemTrackerPtr DoMakeMemTracker(const char *name);
	}  // namespace storage
}  // namespace coast

//! wrapper class to provide protocol for dispatching if non standard (GlobalAllocator) is used
class StorageHooks {
	StorageHooks(const StorageHooks &);
	StorageHooks &operator=(const StorageHooks &);
	coast::storage::StorageHooksPtr fParentHook;

public:
	StorageHooks() {}

	virtual ~StorageHooks() {}

	//! initialize storage subsystem
	void Initialize() { DoInitialize(); }
	//! finalize storage subsystem
	void Finalize() { DoFinalize(); }
	//! access global allocator
	Allocator *Global() { return DoGlobal(); }
	//! access allocator set for current context (e.g. thread)
	Allocator *Current() { return DoCurrent(); }

	/*! allocate a memory tracker object
	 \param name name of the tracker
	 \param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
	 \return pointer to a newly created MemTracker object */
	Allocator::MemTrackerPtr MakeMemTracker(const char *name, bool bThreadSafe) { return DoMakeMemTracker(name, bThreadSafe); }

	void SetOldHook(coast::storage::StorageHooksPtr pOld) { fParentHook = pOld; }

	coast::storage::StorageHooksPtr GetOldHook() { return fParentHook; }

protected:
	//! initialize storage subsystem
	virtual void DoInitialize() = 0;

	//! finalize storage subsystem
	virtual void DoFinalize() = 0;

	//! access global allocator
	virtual Allocator *DoGlobal() = 0;

	//! access allocator set for current context (e.g. thread)
	virtual Allocator *DoCurrent() = 0;

	/*! allocate a memory tracker object
	 \param name name of the tracker
	 \param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
	 \return pointer to a newly created MemTracker object */
	virtual Allocator::MemTrackerPtr DoMakeMemTracker(const char *name, bool bThreadSafe) = 0;
};

class TestStorageHooks {
	TestStorageHooks(const TestStorageHooks &);
	TestStorageHooks &operator=(const TestStorageHooks &);
	struct _Hooks : public StorageHooks {
		Allocator *fAllocator;
		explicit _Hooks(Allocator *wdallocator) : fAllocator(wdallocator) {}
		~_Hooks() { fAllocator = 0; }
		virtual void DoInitialize() {}
		virtual void DoFinalize() {}
		virtual Allocator *DoGlobal() { return coast::storage::DoGlobal(); }
		virtual Allocator *DoCurrent() {
			if (fAllocator != 0) {
				return fAllocator;
			}
			return coast::storage::DoGlobal();
		}
		virtual Allocator::MemTrackerPtr DoMakeMemTracker(const char *name, bool) {
			return Allocator::MemTrackerPtr(new MemTracker(name));
		}
	};
	coast::storage::StorageHooksPtr theHooks;

public:
	explicit TestStorageHooks(Allocator *wdallocator) : theHooks(new _Hooks(wdallocator)) {
		coast::storage::registerHook(theHooks);
	}
	virtual ~TestStorageHooks() {
		coast::storage::StorageHooksPtr pHook = coast::storage::unregisterHook();
		Assert(pHook != theHooks && "another coast::storage::SetHook() was called without restoring old Hook!");
	}
};

#endif
