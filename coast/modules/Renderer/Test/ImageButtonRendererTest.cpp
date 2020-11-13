/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ImageButtonRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

ImageButtonRendererTest::ImageButtonRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) ImageButtonRenderer("ImageButtonRenderer");
};

ImageButtonRendererTest::~ImageButtonRendererTest() {
	delete fFieldRenderer;

	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ImageButtonRendererTest::setUp() {
	FieldRendererTest::setUp();
}

Test *ImageButtonRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCase0);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCase1);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCase2);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCase3);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, ImageButtonRendererTest, ConfigurationWithRenderer);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, ImageButtonRendererTest, TestFaultOptionRendererOld);

	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void ImageButtonRendererTest::TestCase0() {
	fCurrentTestMethod = "ImageButton-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCase1() {
	fCurrentTestMethod = "ImageButton-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCase2() {
	fCurrentTestMethod = "ImageButton-TestCase2";
	this->TestField2();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCase3() {
	fCurrentTestMethod = "ImageButton-TestCase3";
	this->TestField3();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void ImageButtonRendererTest::TestCaseEmptyConf() {
	fCurrentTestMethod = "ImageButton-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual("", fReply.str());
}

void ImageButtonRendererTest::TestCaseWithoutName() {
	fCurrentTestMethod = "ImageButton-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

void ImageButtonRendererTest::TestCaseWithoutLabel() {
	fCurrentTestMethod = "ImageButton-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCaseWithoutValue() {
	fCurrentTestMethod = "ImageButton-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCaseWithoutSource() {
	fCurrentTestMethod = "ImageButton-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCaseWithoutMultiple() {
	fCurrentTestMethod = "ImageButton-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCaseWithoutChecked() {
	fCurrentTestMethod = "ImageButton-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestCaseWithoutOptions() {
	fCurrentTestMethod = "ImageButton-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void ImageButtonRendererTest::TestCaseWrong() {
	fCurrentTestMethod = "ImageButton-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void ImageButtonRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "ImageButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestFaultOptionRendererOld() {
	fCurrentTestMethod = "ImageButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "ImageButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void ImageButtonRendererTest::TestFaultOptionRenderer() {
	fCurrentTestMethod = "ImageButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fConfig with Renderers             */
/*===============================================================*/
void ImageButtonRendererTest::ConfigurationWithRenderer() {
	fCurrentTestMethod = "ImageButton-ConfigurationWithRenderer";
	this->TestConfigureFieldWithRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_String rendered Name" SRC="String rendered Source" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="String rendered Option">), fReply.str() );
	// clang-format on
}
