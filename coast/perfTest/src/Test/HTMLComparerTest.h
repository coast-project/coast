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

class HTMLComparerTest : public testframework::TestCaseWithConfig {
public:
	HTMLComparerTest(TString tstrName) : TestCaseType(tstrName) {}
	static Test *suite();
	TString getConfigFileName();
	void NoDifferenceComparison();
	void DifferenceComparison();
};

#endif
