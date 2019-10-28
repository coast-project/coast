/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StrSpecialTest_h_
#define _StrSpecialTest_h_

#include "TestCase.h"
class StrSpecialTest: public testframework::TestCase {
public:
	StrSpecialTest(TString tname) :
		TestCaseType(tname) {
	}
	static Test *suite();
	void simpleAppendTest();
	void umlauteTest();
};

#endif
