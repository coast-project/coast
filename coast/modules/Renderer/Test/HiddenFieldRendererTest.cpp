/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HiddenFieldRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

HiddenFieldRendererTest::HiddenFieldRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) HiddenFieldRenderer("HiddenFieldRendererTest");
};

HiddenFieldRendererTest::~HiddenFieldRendererTest() {
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void HiddenFieldRendererTest::setUp() {
	FieldRendererTest::setUp();
}

Test *HiddenFieldRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCase0);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCase1);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCase2);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCase3);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, HiddenFieldRendererTest, TestFaultOptionRendererOld);

	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void HiddenFieldRendererTest::TestCase0() {
	fCurrentTestMethod = "HiddenField-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCase1() {
	fCurrentTestMethod = "HiddenField-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCase2() {
	fCurrentTestMethod = "HiddenField-TestCase2";
	this->TestField2();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCase3() {
	fCurrentTestMethod = "HiddenField-TestCase3";
	this->TestField3();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void HiddenFieldRendererTest::TestCaseEmptyConf() {
	fCurrentTestMethod = "HiddenField-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual("", fReply.str());
}

void HiddenFieldRendererTest::TestCaseWithoutName() {
	fCurrentTestMethod = "HiddenField-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

void HiddenFieldRendererTest::TestCaseWithoutLabel() {
	fCurrentTestMethod = "HiddenField-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCaseWithoutValue() {
	fCurrentTestMethod = "HiddenField-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCaseWithoutSource() {
	fCurrentTestMethod = "HiddenField-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" the options of field>), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCaseWithoutMultiple() {
	fCurrentTestMethod = "HiddenField-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCaseWithoutChecked() {
	fCurrentTestMethod = "HiddenField-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestCaseWithoutOptions() {
	fCurrentTestMethod = "HiddenField-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void HiddenFieldRendererTest::TestCaseWrong() {
	fCurrentTestMethod = "HiddenField-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void HiddenFieldRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "HiddenFieldRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestFaultOptionRendererOld() {
	fCurrentTestMethod = "HiddenFieldRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "HiddenFieldRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void HiddenFieldRendererTest::TestFaultOptionRenderer() {
	fCurrentTestMethod = "HiddenFieldRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}
