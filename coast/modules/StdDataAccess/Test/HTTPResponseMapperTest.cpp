/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPResponseMapperTest.h"
#include "TestSuite.h"
#include "HTTPResponseMapper.h"
#include "Dbg.h"
#include "StringStream.h"
#include "Context.h"

//---- HTTPResponseMapperTest ----------------------------------------------------------------
HTTPResponseMapperTest::HTTPResponseMapperTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(HTTPResponseMapperTest.Ctor);
}

HTTPResponseMapperTest::~HTTPResponseMapperTest()
{
	StartTrace(HTTPResponseMapperTest.Dtor);
}

void HTTPResponseMapperTest::TestParsedResponse()
{
	StartTrace(HTTPResponseMapperTest.TestParsedResponse);
	String strIn("HTTP/1.1 200 Ok\r\n");
	IStringStream is(strIn);
	HTTPResponseMapper m("HTTPResponseMapper");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(m.Put("", is, ctx));
	Anything result(ctx.GetTmpStore()["Mapper"]["HTTPResponse"]);
	assertEqual(200L, result["Code"].AsLong(0));
	assertEqual("Ok", result["Msg"].AsCharPtr());
	assertEqual("HTTP/1.1", result["HTTP-Version"].AsCharPtr());
}
void HTTPResponseMapperTest::TestBadResponseLine()
{
	StartTrace(HTTPResponseMapperTest.TestBadResponseLine);
	String strIn("HTTP/1.1 ");
	IStringStream is(strIn);
	HTTPResponseMapper m("HTTPResponseMapper");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(!m.Put("", is, ctx));
	Anything result(ctx.GetTmpStore()["Mapper"]["HTTPResponse"]);
	assertEqual(-1L, result["Code"].AsLong(-1));
	assertEqual("undefined", result["Msg"].AsCharPtr("undefined"));
	assertEqual("HTTP/1.1", result["HTTP-Version"].AsCharPtr());
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPResponseMapperTest::suite ()
{
	StartTrace(HTTPResponseMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPResponseMapperTest, TestParsedResponse);
	ADD_CASE(testSuite, HTTPResponseMapperTest, TestBadResponseLine);

	return testSuite;
}
