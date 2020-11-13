/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnySorterTest.h"
#include "AnyUtilsTest.h"
#include "GenericXMLParserTest.h"
#include "TestRunner.h"

void setupRunner(TestRunner &runner) {
	ADD_SUITE(runner, AnySorterTest);
	ADD_SUITE(runner, AnyUtilsTest);
	ADD_SUITE(runner, GenericXMLParserTest);
}  // setupRunner
