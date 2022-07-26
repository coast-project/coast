/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SegStorAllocatorNewDelete_H
#define _SegStorAllocatorNewDelete_H

#include "ITOStorage.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace coast {

	//! Segregated Storage Allocator for operator new/delete
	/*!
	Caution: This class is only intended for AnyImpl's! Subclassing from one of the various AnyImpl
	classes can lead to space leaks because of a possible size differences.
	*/
	template <typename T>
	class SegStorAllocatorNewDelete {
	public:
		/*! operator used when placement new gets called
		 * @param sz size of memory block to allocate
		 * @param a Allocator used for allocating memory
		 */
		static void *operator new(std::size_t sz, Allocator *a) COAST_NOEXCEPT_OR_NOTHROW {
			assert(sz == sizeof(T));
			return a->Malloc<sizeof(T)>();
		}

		/*! operator used when placement new gets called without Allocator
		 * We fallback to using coast::storage::Global() to still fulfill the request
		 * @param sz size of memory block to allocate
		 */
		static void *operator new(std::size_t sz) COAST_NOEXCEPT_OR_NOTHROW { return operator new(sz, storage::Global()); }

		/*! operator used when placement new gets called for arrays
		 * @param sz size of memory block to allocate
		 * @param a Allocator used for allocating memory
		 */
		static void *operator new[](std::size_t sz, Allocator *a) COAST_NOEXCEPT_OR_NOTHROW {
			if (a != 0) {
				void *ptr = a->Malloc(memory::calculateAllocationSize<Allocator *>(sz));
				memory::allocatorFor<Allocator *>(ptr) = a;	 // remember address of responsible Allocator
				return memory::payloadPtrFor<Allocator *>(ptr);
			}
			return a;
		}

		/*! operator used when placement new gets called for arrays without Allocator
		 * We fallback to using coast::storage::Global() to still fulfill the request
		 * @param sz size of memory block to allocate
		 */
		static void *operator new[](std::size_t sz) COAST_NOEXCEPT_OR_NOTHROW { return operator new[](sz, storage::Global()); }

		/*! operator used when delete gets called
		 * @param ptr memory block to delete
		 */
		static void operator delete(void *ptr)COAST_NOEXCEPT_OR_NOTHROW {
			operator delete(ptr, static_cast<T *>(ptr)->MyAllocator());
		}

		/*! operator used when delete gets called within ctor of allocated class
		 * @param ptr memory block to delete
		 * @param a Allocator used for freeing memory
		 */
		static void operator delete(void *ptr, Allocator *a)COAST_NOEXCEPT_OR_NOTHROW { a->Free<sizeof(T)>(ptr); }

		/*! operator used when delete gets called for arrays
		 * @param ptr memory block to delete
		 */
		static void operator delete[](void *ptr) COAST_NOEXCEPT_OR_NOTHROW {
			void *realPtr = memory::realPtrFor<Allocator *>(ptr);
			Allocator *a = memory::allocatorFor<Allocator *>(realPtr);
			if (a != 0) {
				memory::safeFree(a, realPtr);
			} else {
				free(realPtr);
			}
		}

	private:
		//! disallow unintended creation of non-Allocator instances
		static void *operator new(std::size_t, const std::nothrow_t &) COAST_NOEXCEPT_OR_NOTHROW;
		static void *operator new[](std::size_t, const std::nothrow_t &) COAST_NOEXCEPT_OR_NOTHROW;
		static void operator delete(void *, const std::nothrow_t &)COAST_NOEXCEPT_OR_NOTHROW;
		static void operator delete[](void *, const std::nothrow_t &) COAST_NOEXCEPT_OR_NOTHROW;
		static void operator delete[](void *, void *) COAST_NOEXCEPT_OR_NOTHROW;
	};
}  // namespace coast
#endif /* _SegStorAllocatorNewDelete_H */
