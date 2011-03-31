/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _InterruptHandlerTest_H
#define _InterruptHandlerTest_H

#include "WDBaseTestPolicies.h"

//---- InterruptHandlerTest ----------------------------------------------------------
//!TestCases description
class InterruptHandlerTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	InterruptHandlerTest(TString tstrName);

	//!destroys the test case
	~InterruptHandlerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!describe this testcase
	void PidFileHandlingTest();
};


#endif
