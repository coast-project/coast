/*
 * Copyright (c) 2020, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RWLock.h"

RWLock::RWLock() {
	CREATERWLOCK(fLock);
}

RWLock::RWLock(const char *name, Allocator *a) : fName(name, -1, a) {
	CREATERWLOCK(fLock);
}

RWLock::RWLock(const RWLock &) {
	// private no op -> don't use this;
}

void RWLock::operator=(const RWLock &) {
	// private no op -> don't use this;
}

RWLock::~RWLock() {
	DELETERWLOCK(fLock);
}

void RWLock::Lock(eLockMode mode) const {
	LOCKRWLOCK(fLock, (mode == eReading));
}

void RWLock::Unlock(eLockMode mode) const {
	UNLOCKRWLOCK(fLock, (mode == eReading));
}

bool RWLock::TryLock(eLockMode mode) const {
	return TRYRWLOCK(fLock, (mode == eReading));
}
