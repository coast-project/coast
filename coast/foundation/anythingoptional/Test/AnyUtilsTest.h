/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyUtilsTest_h_
#define _AnyUtilsTest_h_

#include "Anything.h"
#include "TestCase.h"
class AnyUtilsTest : public testframework::TestCase {
public:
	AnyUtilsTest(TString tstrName) : TestCaseType(tstrName) {}
	static Test *suite();
	void CompareTest();
	void printEmptyXmlTest();
	void printSimpleXmlTest();
	void printSequenceXmlTest();
	void printHashXmlTest();
	void printMixedXmlTest();
	void MergeTest();

protected:
	bool DoXMLTest(const char *expect, ROAnything foroutput);
	// utility
	void DoCheck(Anything testCases, bool expectedResult, String description);
};

#endif
