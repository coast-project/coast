/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef LOCKUNLOCKENTRY_H_
#define LOCKUNLOCKENTRY_H_

#include "AllocatorNewDelete.h"
#include "RWLock.h"
#include "Tracer.h"

class LockUnlockEntry {
	struct WrapperBase : public coast::AllocatorNewDelete {
		WrapperBase(Allocator *pAlloc) : fAllocator(pAlloc) {
			StatTrace(LockUnlockEntry.WrapperBase, "", coast::storage::Current());
		}
		virtual ~WrapperBase() { StatTrace(LockUnlockEntry.~WrapperBase, "", coast::storage::Current()); }
		Allocator *fAllocator;
	};

	template <typename TMutex, typename dummy>
	struct LockUnlockEntryWrapper : public WrapperBase {
		typedef TMutex LockType;
		explicit LockUnlockEntryWrapper(LockType &m, Allocator *pAlloc) : WrapperBase(pAlloc), fLock(m) {
			StatTrace(LockUnlockEntry.LockUnlockEntryWrapper, "Mutex", coast::storage::Current());
			this->fLock.Lock();
		}
		~LockUnlockEntryWrapper() {
			StatTrace(LockUnlockEntry.~LockUnlockEntryWrapper, "Mutex", coast::storage::Current());
			this->fLock.Unlock();
		}
		LockType &fLock;
	};

	template <typename dummy>
	struct LockUnlockEntryWrapper<RWLock, dummy> : public WrapperBase {
		typedef RWLock LockType;
		explicit LockUnlockEntryWrapper(LockType &m, LockType::eLockMode mode, Allocator *pAlloc)
			: WrapperBase(pAlloc), fLock(m), fMode(mode) {
			StatTrace(LockUnlockEntry.LockUnlockEntryWrapper, "RWLock", coast::storage::Current());
			this->fLock.Lock(fMode);
		}
		~LockUnlockEntryWrapper() {
			StatTrace(LockUnlockEntry.~LockUnlockEntryWrapper, "RWLock", coast::storage::Current());
			this->fLock.Unlock(fMode);
		}
		LockType &fLock;
		LockType::eLockMode fMode;
	};

	boost_or_std::auto_ptr<WrapperBase> fWrapper;
	LockUnlockEntry();
	LockUnlockEntry(LockUnlockEntry &);
	LockUnlockEntry &operator=(const LockUnlockEntry &);

public:
	//! acquires the mutex in the constructor
	template <typename TMutex>
	explicit LockUnlockEntry(TMutex &m, Allocator *pAlloc = coast::storage::Current())
		: fWrapper(new (pAlloc) LockUnlockEntryWrapper<TMutex, bool>(m, pAlloc)) {}

	template <typename LockType>
	explicit LockUnlockEntry(LockType &m, typename LockType::eLockMode mode, Allocator *pAlloc = coast::storage::Current())
		: fWrapper(new (pAlloc) LockUnlockEntryWrapper<LockType, bool>(m, mode, pAlloc)) {}
};

#endif /* LOCKUNLOCKENTRY_H_ */
