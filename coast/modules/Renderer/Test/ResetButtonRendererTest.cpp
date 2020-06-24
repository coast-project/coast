/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ResetButtonRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

ResetButtonRendererTest::ResetButtonRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) ResetButtonRenderer("ResetButtonRenderer");
};

ResetButtonRendererTest::~ResetButtonRendererTest() {
	delete fFieldRenderer;

	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ResetButtonRendererTest::setUp() {
	FieldRendererTest::setUp();
}

Test *ResetButtonRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCase0);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCase1);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCase2);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCase3);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, ResetButtonRendererTest, TestFaultOptionRendererOld);

	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void ResetButtonRendererTest::TestCase0() {
	fCurrentTestMethod = "ResetButton-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCase1() {
	fCurrentTestMethod = "ResetButton-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCase2() {
	fCurrentTestMethod = "ResetButton-TestCase2";
	this->TestField2();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCase3() {
	fCurrentTestMethod = "ResetButton-TestCase3";
	this->TestField3();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void ResetButtonRendererTest::TestCaseEmptyConf() {
	fCurrentTestMethod = "ResetButton-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual("", fReply.str());
}

void ResetButtonRendererTest::TestCaseWithoutName() {
	fCurrentTestMethod = "ResetButton-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

void ResetButtonRendererTest::TestCaseWithoutLabel() {
	fCurrentTestMethod = "ResetButton-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCaseWithoutValue() {
	fCurrentTestMethod = "ResetButton-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCaseWithoutSource() {
	fCurrentTestMethod = "ResetButton-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCaseWithoutMultiple() {
	fCurrentTestMethod = "ResetButton-TestCaseWithoutMultiple";
	this->TestFieldWithoutMultiple();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCaseWithoutChecked() {
	fCurrentTestMethod = "ResetButton-TestCaseWithoutChecked";
	this->TestFieldWithoutChecked();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestCaseWithoutOptions() {
	fCurrentTestMethod = "ResetButton-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void ResetButtonRendererTest::TestCaseWrong() {
	fCurrentTestMethod = "ResetButton-TestFieldWithoutOptions";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void ResetButtonRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "ResetButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestFaultOptionRendererOld() {
	fCurrentTestMethod = "ResetButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "ResetButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ResetButtonRendererTest::TestFaultOptionRenderer() {
	fCurrentTestMethod = "ResetButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}
