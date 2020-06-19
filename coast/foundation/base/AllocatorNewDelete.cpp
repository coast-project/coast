/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AllocatorNewDelete.h"

#include <cstddef>
#include <cstdlib>

namespace coast {
	void *AllocatorNewDelete::operator new(std::size_t sz, Allocator *a) COAST_NOEXCEPT_OR_NOTHROW {
		if (a) {
			void *ptr = a->Calloc(1, memory::calculateAllocationSize<Allocator *>(sz));
			memory::allocatorFor<Allocator *>(ptr) = a;	 // remember address of responsible Allocator
			return memory::payloadPtrFor<Allocator *>(ptr);
		}
		return a;
	}

	void *AllocatorNewDelete::operator new[](std::size_t sz, Allocator *a) COAST_NOEXCEPT_OR_NOTHROW {
		if (a) {
			return operator new(sz, a);
		}
		void *ptr = calloc(1, memory::calculateAllocationSize<Allocator *>(sz));
		return memory::payloadPtrFor<Allocator *>(ptr);
	}

	void *AllocatorNewDelete::operator new[](std::size_t sz) COAST_NOEXCEPT_OR_NOTHROW {
		return operator new[](sz, static_cast<Allocator *>(0));
	}

	void AllocatorNewDelete::operator delete(void *ptr)COAST_NOEXCEPT_OR_NOTHROW {
		void *realPtr = memory::realPtrFor<Allocator *>(ptr);
		Allocator *a = memory::allocatorFor<Allocator *>(realPtr);
		if (a) {
			memory::safeFree(a, realPtr);
		} else {
			free(realPtr);
		}
	}

	void AllocatorNewDelete::operator delete(void *ptr, Allocator *a)COAST_NOEXCEPT_OR_NOTHROW { memory::safeFree(a, ptr); }

	void AllocatorNewDelete::operator delete[](void *ptr) COAST_NOEXCEPT_OR_NOTHROW { operator delete(ptr); }

	AllocatorNewDelete::~AllocatorNewDelete() {}
}  // namespace coast
