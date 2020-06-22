/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ButtonRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

ButtonRendererTest::ButtonRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) ButtonRenderer("ButtonRenderer");
};

ButtonRendererTest::~ButtonRendererTest() {
	if (fFieldRenderer != 0) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ButtonRendererTest::setUp() {
	FieldRendererTest::setUp();
}

Test *ButtonRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ButtonRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, ButtonRendererTest, TestCase0);
	ADD_CASE(testSuite, ButtonRendererTest, TestCase1);
	ADD_CASE(testSuite, ButtonRendererTest, TestCase2);
	ADD_CASE(testSuite, ButtonRendererTest, TestCase3);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, ButtonRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, ButtonRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, ButtonRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, ButtonRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, ButtonRendererTest, TestFaultOptionRendererOld);

	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void ButtonRendererTest::TestCase0() {
	fCurrentTestMethod = "ButtonRendererTest-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCase1() {
	fCurrentTestMethod = "ButtonRendererTest-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCase2() {
	fCurrentTestMethod = "ButtonRendererTest-TestCase2";
	this->TestField2();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCase3() {
	fCurrentTestMethod = "ButtonRendererTest-TestCase3";
	this->TestField3();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void ButtonRendererTest::TestCaseEmptyConf() {
	fCurrentTestMethod = "ButtonRendererTest-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual("", fReply.str());
}

void ButtonRendererTest::TestCaseWithoutName() {
	fCurrentTestMethod = "ButtonRendererTest-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

void ButtonRendererTest::TestCaseWithoutLabel() {
	fCurrentTestMethod = "ButtonRendererTest-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCaseWithoutValue() {
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCaseWithoutSource() {
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCaseWithoutMultiple() {
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCaseWithoutChecked() {
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestCaseWithoutOptions() {
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void ButtonRendererTest::TestCaseWrong() {
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void ButtonRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "ButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestFaultOptionRendererOld() {
	fCurrentTestMethod = "ButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "ButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ButtonRendererTest::TestFaultOptionRenderer() {
	fCurrentTestMethod = "ButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}
