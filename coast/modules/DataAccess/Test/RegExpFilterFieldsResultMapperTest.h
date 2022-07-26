/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegExpFilterFieldsResultMapperTest_H
#define _RegExpFilterFieldsResultMapperTest_H

#include "FoundationTestTypes.h"

//---- RegExpFilterFieldsResultMapperTest ----------------------------------------------------------
class RegExpFilterFieldsResultMapperTest : public testframework::TestCaseWithConfig {
public:
	//! ConfiguredTestCase constructor
	//! \param name name of the test
	RegExpFilterFieldsResultMapperTest(TString tstrName) : TestCaseType(tstrName) {}

	//! builds up a suite of tests
	static Test *suite();

	//! take a simple http response and parse it
	void ConfiguredTests();
};

#endif
