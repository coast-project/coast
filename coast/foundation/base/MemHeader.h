/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MemHeader_H
#define _MemHeader_H

#include <sys/types.h>
#if defined(WIN32)
#include <windows.h>
#endif
#include <iosfwd>

//! handling headers for memory management
//! supports custom memory management by easing handling of header information
struct MemoryHeader {
	//! the states a chunk of memory can be in
#if __cplusplus >= 201103L
	enum EMemState : unsigned char {
#else
	enum EMemState {
#endif
		eFree = 1L,
		eUsed = 2L,
		eNotPooled = 4L,
		eFreeNotPooled = (eFree | eNotPooled),
		eUsedNotPooled = (eUsed | eNotPooled),
	};

	//! magic cookie to determine memory boundaries
	static unsigned char const gcMagic;

	//! constructor simplifying settings of values
	MemoryHeader(u_long size, EMemState ems) : fMagic(MemoryHeader::gcMagic), fState(ems), fUsableSize(size), fNextFree(0) {}
	//! the cookie defining the boundary of the memory
	unsigned char fMagic;
	//! state a chunk of memory is in
	EMemState fState;
	//! size of the memory chunk
	size_t fUsableSize;
	//! link to next free chunk in pool
	MemoryHeader *fNextFree;
	std::ostream &DumpAsHex(std::ostream &stream) const;
};

#endif
