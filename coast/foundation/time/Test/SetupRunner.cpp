/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DiffTimerTest.h"
#include "TestRunner.h"
#include "TimeStampTest.h"

void setupRunner(TestRunner &runner) {
	ADD_SUITE(runner, DiffTimerTest);
	ADD_SUITE(runner, TimeStampTest);
}  // setupRunner
