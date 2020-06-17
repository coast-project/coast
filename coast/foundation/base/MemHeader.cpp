/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MemHeader.h"

#include <cstddef>
#include <iomanip>
#include <ostream>

unsigned char const MemoryHeader::gcMagic = 0xC0;

std::ostream &MemoryHeader::DumpAsHex(std::ostream &stream) const {
	stream << std::setw(sizeof(fMagic) * 2) << std::setfill('0') << std::hex << static_cast<int>(fMagic) << ' ';
	stream << std::setw(sizeof(char) * 2) << std::setfill('0') << std::hex << static_cast<int>(fState) << ' ';
	stream << std::setw(0) << std::dec << fUsableSize << ' ';
	stream << std::setw(sizeof(reinterpret_cast<ptrdiff_t>(fNextFree)) * 2) << std::setfill('0') << std::hex << fNextFree
		   << std::setw(0);
	return stream;
}
