/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AllocatorNewDelete.h"

namespace coast {
	void *AllocatorNewDelete::operator new(std::size_t sz, Allocator *a) throw () {
		if (a) {
			void *ptr = a->Calloc(1, memory::calculateAllocationSize<Allocator*>(sz));
			memory::allocatorFor<Allocator*>(ptr) = a; // remember address of responsible Allocator
			return memory::payloadPtrFor<Allocator*>(ptr);
		}
		return a;
	}

	void *AllocatorNewDelete::operator new[](std::size_t sz, Allocator *a) throw () {
		if (a) {
			return operator new(sz, a);
		} else {
			void *ptr = calloc(1, memory::calculateAllocationSize<Allocator*>(sz));
			return memory::payloadPtrFor<Allocator*>(ptr);
		}
	}

	void *AllocatorNewDelete::operator new[](std::size_t sz) throw()
	{
		return operator new[](sz, static_cast<Allocator*>(0));
	}

	void AllocatorNewDelete::operator delete(void *ptr) throw()
	{
		void *realPtr = memory::realPtrFor<Allocator*>(ptr);
		Allocator *a = memory::allocatorFor<Allocator*>(realPtr);
		if (a) {
			memory::safeFree(a, realPtr);
		} else {
			free(realPtr);
		}
	}

	void AllocatorNewDelete::operator delete(void *ptr, Allocator *a) throw()
	{
		memory::safeFree(a, ptr);
	}

	void AllocatorNewDelete::operator delete[](void *ptr) throw()
	{
		operator delete(ptr);//lint !e1023
	}

	AllocatorNewDelete::~AllocatorNewDelete() {}
}
