/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "FileBrowseRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "StringStream.h"
#include "TestSuite.h"

FileBrowseRendererTest::FileBrowseRendererTest(TString tname) : TextFieldRendererTest(tname) {
	delete fFieldRenderer;	// base class already initialized this !

	fFieldRenderer = new (coast::storage::Global()) FileBrowseRenderer("FileBrowseRenderer");
}

FileBrowseRendererTest::~FileBrowseRendererTest() {
	delete fFieldRenderer;

	fFieldRenderer = 0;
};

Test *FileBrowseRendererTest::suite()
/* what: return the whole suite of tests for FileBrowseRendererTest, add all top level
	 test functions here.
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, FileBrowseRendererTest, TestCase0);

	return testSuite;
}

void FileBrowseRendererTest::TestCase0() {
	fCurrentTestMethod = "TextField-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="FILE" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" the options of field>), fReply.str() );
	// clang-format on
}
