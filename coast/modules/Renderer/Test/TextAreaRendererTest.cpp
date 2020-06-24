/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TextAreaRendererTest.h"

#include "Anything.h"
#include "Context.h"
#include "FormRenderer.h"
#include "Renderer.h"
#include "StringStream.h"
#include "TestSuite.h"

TextAreaRendererTest::TextAreaRendererTest(TString tname) : FieldRendererTest(tname) {
	fFieldRenderer = new (coast::storage::Global()) TextAreaRenderer("TextAreaRenderer");
}
TextAreaRendererTest::~TextAreaRendererTest() {
	delete fFieldRenderer;

	fFieldRenderer = 0;
}

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void TextAreaRendererTest::setUp() {
	FieldRendererTest::setUp();
}

Test *TextAreaRendererTest::suite() {
	TestSuite *testSuite = new TestSuite;

	// Generic test done for each type of FieldRenderer
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase0);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase1);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase2);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase3);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, TextAreaRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, TextAreaRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, TextAreaRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, TextAreaRendererTest, TestFaultOptionRendererOld);

	// Specific test ONLY for TextAreaRenderer
	ADD_CASE(testSuite, TextAreaRendererTest, AllAttributesTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, WidthTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, HeightTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, WrapTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Options4TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Options3TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Options2TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Option1TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, OptionsNoSlotName2TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, OptionNoSlotName1TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, OptionsMixedTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, NoOptionsTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, TextTextArea);

	return testSuite;
}

/*=================================================================================*/
/* Generic tests done for each type of FieldRendererTest                           */
/*=================================================================================*/
/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void TextAreaRendererTest::TestCase0() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCase0";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCase1() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCase1";
	this->TestField1();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCase2() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCase2";
	this->TestField2();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCase3() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCase3";
	this->TestField3();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void TextAreaRendererTest::TestCaseEmptyConf() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual("", fReply.str());
}

void TextAreaRendererTest::TestCaseWithoutName() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

void TextAreaRendererTest::TestCaseWithoutLabel() {
	fCurrentTestMethod = "TextAreaRendererTest-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCaseWithoutValue() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field></TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCaseWithoutSource() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field></TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCaseWithoutMultiple() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCaseWithoutChecked() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestCaseWithoutOptions() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10>the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void TextAreaRendererTest::TestCaseWrong() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutName();
	assertCharPtrEqual("", fReply.str());
}

/*=================================================================================*/
/* Special testcases ONLY for TextAreaRenderer                                     */
/*=================================================================================*/
void TextAreaRendererTest::AllAttributesTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-AllAttributesTextArea";
	this->TestField0();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::WidthTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWidth";
	this->TestFieldWidth();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30></TEXTAREA>), fReply.str());
	// clang-format on
}
void TextAreaRendererTest::HeightTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldHeight";
	this->TestFieldHeight();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" ROWS=20></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::WrapTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWrap";
	this->TestFieldWrap();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" WRAP=10></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::Options4TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test4Opt";
	this->TestField4Opt();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field" 3="the option nr. 3 of field"></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::Options3TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test3Opt";
	this->TestField3Opt();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field"></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::Options2TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test2Opt";
	this->TestField2Opt();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field"></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::Option1TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test1Opt";
	this->TestField1Opt();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field"></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::OptionsNoSlotName2TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test2OptNoSlotName";
	this->TestField2OptNoSlotName();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" the option nr. 0 of field the option nr. 1 of field></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::OptionNoSlotName1TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test1OptNoSlotName";
	this->TestField1OptNoSlotName();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" the option nr. 0 of field></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::OptionsMixedTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Test3OptMixed";
	this->TestField3OptMixed();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" the option nr. 0 of field 1="the option nr. 1 of field" the option nr. 2 of field></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::NoOptionsTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-TestOptEmpty";
	this->TestFieldOptEmpty();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field"></TEXTAREA>), fReply.str());
	// clang-format on
}

void TextAreaRendererTest::TextTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextAreaRendererTest-Text";
	this->TestFieldValue();
	// clang-format off
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field">the value of field</TEXTAREA>), fReply.str());
	// clang-format on
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void TextAreaRendererTest::TestOptionRendererOld() {
	fCurrentTestMethod = "TextAreaRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestFaultOptionRendererOld() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestOptionRenderer() {
	fCurrentTestMethod = "TextAreaRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}

void TextAreaRendererTest::TestFaultOptionRenderer() {
	fCurrentTestMethod = "TextAreaRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	// clang-format off
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
	// clang-format on
}
