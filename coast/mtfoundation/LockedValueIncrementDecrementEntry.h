/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef LOCKEDVALUEINCREMENTDECREMENTENTRY_H_
#define LOCKEDVALUEINCREMENTDECREMENTENTRY_H_

#include "AllocatorNewDelete.h"
#include "ITOString.h"
#include "LockUnlockEntry.h"
#include "Tracer.h"

class LockedValueIncrementDecrementEntry {
	struct WrapperBase : public coast::AllocatorNewDelete {
		WrapperBase(Allocator *pAlloc) : fAllocator(pAlloc) {
			StatTrace(LockedValueIncrementDecrementEntry.WrapperBase, "", coast::storage::Current());
		}
		virtual ~WrapperBase() { StatTrace(LockedValueIncrementDecrementEntry.~WrapperBase, "", coast::storage::Current()); }

		Allocator *fAllocator;
	};

	template <typename MutexType>
	struct LockedValueIncrementDecrementEntryWrapper : public WrapperBase {
		typedef typename MutexType::ConditionType ConditionType;
		explicit LockedValueIncrementDecrementEntryWrapper(MutexType &aMutex, ConditionType &aCondition, long &lValue,
														   Allocator *pAlloc)
			: WrapperBase(pAlloc), fMutex(aMutex), fCondition(aCondition), fValue(lValue) {
			StartTrace(LockedValueIncrementDecrementEntry.LockedValueIncrementDecrementEntry);
			LockUnlockEntry sme(fMutex, fAllocator);
			++fValue;
			Trace("count:" << fValue);
		}
		~LockedValueIncrementDecrementEntryWrapper() {
			StartTrace(LockedValueIncrementDecrementEntry.~LockedValueIncrementDecrementEntry);
			LockUnlockEntry sme(fMutex, fAllocator);
			if (fValue > 0L) {
				--fValue;
			}
			Trace("count:" << fValue);
			Trace("signalling condition");
			fCondition.Signal();
		}
		MutexType &fMutex;
		ConditionType &fCondition;
		long &fValue;
	};

	boost_or_std::auto_ptr<WrapperBase> fWrapper;
	LockedValueIncrementDecrementEntry();
	LockedValueIncrementDecrementEntry(LockedValueIncrementDecrementEntry &);
	LockedValueIncrementDecrementEntry &operator=(const LockedValueIncrementDecrementEntry &);

public:
	template <typename MutexType>
	explicit LockedValueIncrementDecrementEntry(MutexType &aMutex, typename MutexType::ConditionType &aCondition, long &lValue,
												Allocator *pAlloc = coast::storage::Current())
		: fWrapper(new (pAlloc) LockedValueIncrementDecrementEntryWrapper<MutexType>(aMutex, aCondition, lValue, pAlloc)) {}
};

#endif /* LOCKEDVALUEINCREMENTDECREMENTENTRY_H_ */
