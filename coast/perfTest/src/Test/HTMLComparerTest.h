/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLComparerTest_H
#define _HTMLComparerTest_H

#include "FoundationTestTypes.h"

//---- HTMLComparerTest ----------------------------------------------------------
//!TestCases description
class HTMLComparerTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	HTMLComparerTest(TString tstrName);

	//!destroys the test case
	~HTMLComparerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

	//! compares two HTML Any tree that are considererd equal
	void NoDifferenceComparison();

	//! compares two HTML Any tree that are not considererd equal
	void DifferenceComparison();
};

#endif
