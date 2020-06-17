/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CheckBoxRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

CheckBoxRendererTest::CheckBoxRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) CheckBoxRenderer("CheckBoxRendererTest");
};

CheckBoxRendererTest::~CheckBoxRendererTest() {
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void CheckBoxRendererTest::setUp() {
	FieldRendererTest::setUp();
}

Test *CheckBoxRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCase0);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCase1);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCase2);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCase3);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, CheckBoxRendererTest, TestFaultOptionRendererOld);

	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void CheckBoxRendererTest::TestCase0() {
	fCurrentTestMethod = "CheckBox-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCase1() {
	fCurrentTestMethod = "CheckBox-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCase2() {
	fCurrentTestMethod = "CheckBox-TestCase2";
	this->TestField2();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCase3() {
	fCurrentTestMethod = "CheckBox-TestCase3";
	this->TestField3();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void CheckBoxRendererTest::TestCaseEmptyConf() {
	fCurrentTestMethod = "CheckBox-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual("", fReply.str());
}

void CheckBoxRendererTest::TestCaseWithoutName() {
	fCurrentTestMethod = "CheckBox-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

void CheckBoxRendererTest::TestCaseWithoutLabel() {
	fCurrentTestMethod = "CheckBox-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCaseWithoutValue() {
	fCurrentTestMethod = "CheckBox-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCaseWithoutSource() {
	fCurrentTestMethod = "CheckBox-TestFieldWithoutSource";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCaseWithoutMultiple() {
	fCurrentTestMethod = "CheckBox-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCaseWithoutChecked() {
	fCurrentTestMethod = "CheckBox-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestCaseWithoutOptions() {
	fCurrentTestMethod = "CheckBox-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void CheckBoxRendererTest::TestCaseWrong() {
	fCurrentTestMethod = "CheckBox-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void CheckBoxRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "CheckBoxRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestFaultOptionRendererOld() {
	fCurrentTestMethod = "CheckBoxRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "CheckBoxRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void CheckBoxRendererTest::TestFaultOptionRenderer() {
	fCurrentTestMethod = "CheckBoxRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}
