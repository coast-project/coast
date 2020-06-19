/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef CLEANUPHANDLER_H_
#define CLEANUPHANDLER_H_

#include "IFAObject.h"

class CleanupHandler : public IFAObject {
public:
	bool Cleanup() { return DoCleanup(); }

protected:
	//! subclasses implement cleanup of thread specific storage
	virtual bool DoCleanup() = 0;
	virtual IFAObject *Clone(Allocator *) const { return 0; }
};

#endif /* CLEANUPHANDLER_H_ */
