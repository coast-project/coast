/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SegStoreAllocatorTest.h"

#include "Anything.h"
#include "FoundationTestTypes.h"
#include "SegStoreAllocator.h"
#include "TestSuite.h"
#include "Tracer.h"

void SegStoreAllocatorTest::CreateSimpleAllocator() {
	StartTrace(SegStoreAllocatorTest.CreateSimpleAllocator);
	SegStoreAllocator p(3210L);
	Anything a(&p);
	a.Append("Test");
	t_assertm(true, "dummy assertion to generate summary output");
}

void SegStoreAllocatorTest::RealAnythingTest() {
	StartTrace(SegStoreAllocatorTest.RealAnythingTest);
	SegStoreAllocator p(987654L);
	{
		// Index at the end
		Anything test(&p);
		Anything result(&p);
		test[0].Append("a");
		test[0].Append("b");
		test["100"] = test[0];
		test[0].Append("c");
		t_assert(test.LookupPath(result, "100:2"));
		assertAnyEqual(Anything("c"), result);
	}
}

Test *SegStoreAllocatorTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SegStoreAllocatorTest, CreateSimpleAllocator);
	ADD_CASE(testSuite, SegStoreAllocatorTest, RealAnythingTest);
	return testSuite;
}
