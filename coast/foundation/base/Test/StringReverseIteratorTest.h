#ifndef StringReverseIteratorTest_H
#define StringReverseIteratorTest_H

#include "ITOString.h"
#include "TestCase.h"

class StringReverseIteratorTest : public testframework::TestCase {
protected:
	String fStr5;  // string with 5 elements set-up in setUp
	virtual void setUp();

public:
	StringReverseIteratorTest(TString tname) : TestCaseType(tname) {}
	static Test *suite();

	void testEmptyStringBegin();
	void testSimpleStringBegin();
	void testSimpleStringDeref();
	void testSimpleStringIncrement();
	void testSimpleStringDecrement();
	void testSimpleStringIncDec();
	void testSimpleStringAssignment();
	void testStringIteration();
	void testStringIndex();
	void testIteratorSubstract();
	void testIteratorIntAdd();
	void testIteratorIntSubstract();
	void testIteratorCompare();
};

#endif
