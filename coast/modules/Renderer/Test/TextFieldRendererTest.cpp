/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TextFieldRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

TextFieldRendererTest::TextFieldRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) TextFieldRenderer("TextFieldRenderer");
};

TextFieldRendererTest::~TextFieldRendererTest() {
		delete fFieldRenderer;

	fFieldRenderer = 0;
};

void TextFieldRendererTest::setUp() {}

void TextFieldRendererTest::TestCase0() {
	fCurrentTestMethod = "TextField-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" the options of field>), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestCase1() {
	fCurrentTestMethod = "TextField-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "TextField-OptionRendere";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "TextField-OptionRendere";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestValue() {
	fCurrentTestMethod = "TextField-TestCase1";
	this->TestFieldValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" VALUE="the value of field">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestUnreadable() {
	fCurrentTestMethod = "TextField-Unreadable";
	this->TestFieldUnreadable();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestSize() {
	fCurrentTestMethod = "TextField-Size";
	this->TestFieldSize();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" SIZE="50">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::TestMaxlength() {
	fCurrentTestMethod = "TextField-Maxlength";
	this->TestFieldMaxlength();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" MAXLENGTH="40">), fReply.str() );
	// clang-format on
}

void TextFieldRendererTest::Options4TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test4Opt";
	this->TestField4Opt();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field" 3="the option nr. 3 of field">), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::Options3TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test3Opt";
	this->TestField3Opt();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::Options2TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test2Opt";
	this->TestField2Opt();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field">), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::Option1TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test1Opt";
	this->TestField1Opt();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field">), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::OptionsNoSlotName2TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test2OptNoSlotName";
	this->TestField2OptNoSlotName();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" the option nr. 0 of field the option nr. 1 of field>), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::OptionNoSlotName1TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test1OptNoSlotName";
	this->TestField1OptNoSlotName();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" the option nr. 0 of field>), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::OptionsMixedTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test3OptMixed";
	this->TestField3OptMixed();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" the option nr. 0 of field 1="the option nr. 1 of field" the option nr. 2 of field>), fReply.str());
	// clang-format on
}

void TextFieldRendererTest::NoOptionsTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-TestOptEmpty";
	this->TestFieldOptEmpty();
	// clang-format off
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field">), fReply.str());
	// clang-format on
}

Test *TextFieldRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, TextFieldRendererTest, TestCase0);
	ADD_CASE(testSuite, TextFieldRendererTest, TestCase1);
	ADD_CASE(testSuite, TextFieldRendererTest, TestValue);
	ADD_CASE(testSuite, TextFieldRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, TextFieldRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, TextFieldRendererTest, TestUnreadable);
	ADD_CASE(testSuite, TextFieldRendererTest, TestSize);
	ADD_CASE(testSuite, TextFieldRendererTest, TestMaxlength);
	ADD_CASE(testSuite, TextFieldRendererTest, Options4TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, Options3TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, Options2TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, Option1TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, OptionsNoSlotName2TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, OptionNoSlotName1TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, OptionsMixedTextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, NoOptionsTextArea);

	return testSuite;
}
