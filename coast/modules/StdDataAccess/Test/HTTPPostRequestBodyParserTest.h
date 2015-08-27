/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestBodyParserTest_H
#define _RequestBodyParserTest_H

#include "FoundationTestTypes.h"

class HTTPPostRequestBodyParserTest : public testframework::TestCaseWithConfig {
public:
	//! \param name name of the test
	HTTPPostRequestBodyParserTest(TString tstrName);

	//!builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

	void ReadToBoundaryTest();
	void ParseMultiPartTest();
	void ReadMultiPartPost();
	void ReadToBoundaryTestWithStreamFailure();
	void ReadMultiPartPostBody();
};
#endif
