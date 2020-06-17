/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConversionUtilsTest.h"
#include "ObjectListTest.h"
#include "SegStoreAllocatorTest.h"
#include "TestRunner.h"

void setupRunner(TestRunner &runner) {
	ADD_SUITE(runner, ObjectListTest);
	ADD_SUITE(runner, ConversionUtilsTest);
	ADD_SUITE(runner, SegStoreAllocatorTest);
}  // setupRunner
