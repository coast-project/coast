/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "AnyToXMLTest.h"
#include "BasicTableRendererTest.h"
#include "ButtonRendererTest.h"
#include "CallRendererTest.h"
#include "CheckBoxRendererTest.h"
#include "ConditionalRendererTest.h"
#include "DateRendererTest.h"
#include "FileBrowseRendererTest.h"
#include "FirstNonEmptyRendererTest.h"
#include "FormRendererTest.h"
#include "GetEnvRendererTest.h"
#include "HTMLCacheLoaderTest.h"
#include "HiddenFieldRendererTest.h"
#include "ImageButtonRendererTest.h"
#include "NewRendererTest.h"
#include "PageRelatedRendererTest.h"
#include "PulldownMenuRendererTest.h"
#include "RadioButtonRendererTest.h"
#include "ResetButtonRendererTest.h"
#include "SelectBoxRendererTest.h"
#include "StringRendererTest.h"
#include "SwitchRendererTest.h"
#include "TemplateParserTest.h"
#include "TextAreaRendererTest.h"
#include "URLRendererTest.h"
#include "UTF8RendererTest.h"

void setupRunner(TestRunner &runner) {
	// add a whole suite with the ADD_SUITE(runner,"Suites's Classname") macro
	ADD_SUITE(runner, NewRendererTest);
	ADD_SUITE(runner, PageRelatedRendererTest);
	ADD_SUITE(runner, ConditionalRendererTest);
	ADD_SUITE(runner, DateRendererTest);
	ADD_SUITE(runner, FormRendererTest);
	ADD_SUITE(runner, StringRendererTest);
	ADD_SUITE(runner, SwitchRendererTest);
	ADD_SUITE(runner, URLRendererTest);
	ADD_SUITE(runner, ResetButtonRendererTest);
	ADD_SUITE(runner, ButtonRendererTest);
	ADD_SUITE(runner, CheckBoxRendererTest);
	ADD_SUITE(runner, HiddenFieldRendererTest);
	ADD_SUITE(runner, ImageButtonRendererTest);
	ADD_SUITE(runner, RadioButtonRendererTest);
	ADD_SUITE(runner, PulldownMenuRendererTest);
	ADD_SUITE(runner, SelectBoxRendererTest);
	ADD_SUITE(runner, TextAreaRendererTest);
	ADD_SUITE(runner, SelectBoxRendererTest);
	ADD_SUITE(runner, PulldownMenuRendererTest);
	ADD_SUITE(runner, TextFieldRendererTest);
	ADD_SUITE(runner, BasicTableRendererTest);
	ADD_SUITE(runner, FileBrowseRendererTest);
	ADD_SUITE(runner, AnyToXMLTest);
	ADD_SUITE(runner, HTMLCacheLoaderTest);
	ADD_SUITE(runner, CallRendererTest);
	ADD_SUITE(runner, FirstNonEmptyRendererTest);
	ADD_SUITE(runner, TemplateParserTest);
	ADD_SUITE(runner, GetEnvRendererTest);
	ADD_SUITE(runner, UTF8RendererTest);
}  // setupRunner
