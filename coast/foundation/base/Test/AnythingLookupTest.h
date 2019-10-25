/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingLookupTest_H
#define _AnythingLookupTest_H

#include "TestCase.h"
#include "Anything.h"
class AnythingLookupTest: public testframework::TestCase {
public:
	AnythingLookupTest(TString tname) :
		TestCaseType(tname) {
	}
	static Test *suite();
	void LookUp0Test();
	void LookUp1Test();
	void LookupPathByIndex();
	void EmptyLookup();
	void invPathLookup();
	void LookUpWithSpecialCharsTest();
	void LookupCaseSensitiveTest();
protected:
	Anything init5DimArray(long);
	void intLookupPathCheck(Anything &test, const char *path);
};

#endif		//ifndef _AnythingLookupTest_H
