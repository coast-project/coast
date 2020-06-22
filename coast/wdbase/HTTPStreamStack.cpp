/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPStreamStack.h"

#include "HTTPChunkedOStream.h"
#include "ZipStream.h"

HTTPStreamStack::HTTPStreamStack(std::ostream &output, bool chunked, bool zipEnc)
	: fOutput(output), fTopOfStack(&output), fChunker(0), fZipper(0) {
	if (chunked) {
		fOutput << "Transfer-Encoding: chunked" << ENDL;

		fChunker = new HTTPChunkedOStream(*fTopOfStack, 0x1000);
		fTopOfStack = fChunker;
	}

	if (zipEnc) {
		fOutput << "Content-Encoding: gzip" << ENDL;

		fZipper = new ZipOStream(*fTopOfStack);
		fTopOfStack = fZipper;
	}

	// Header separator
	fOutput << ENDL;
};

std::ostream &HTTPStreamStack::GetBodyStream() {
	return *fTopOfStack;
}

HTTPStreamStack::~HTTPStreamStack() {
		delete fZipper;

		delete fChunker;

	fOutput.flush();
}
