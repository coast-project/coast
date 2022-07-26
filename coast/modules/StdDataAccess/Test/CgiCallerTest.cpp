/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CgiCallerTest.h"

#include "CgiCaller.h"
#include "CgiParams.h"
#include "Context.h"
#include "HTTPConstants.h"
#include "TestSuite.h"

void CgiCallerTest::ExecOkTests() {
	StartTrace(CgiCallerTest.ExecOkTests);
	{
		Context ctx;
		Anything tmpStore(ctx.GetTmpStore());
		tmpStore["DocumentRoot"] = "config";
		tmpStore["REQUEST_URI"] = "/testcgi.sh";

		DoExecTest(ctx);
	}
	{
		Context ctx;
		Anything tmpStore(ctx.GetTmpStore());
		tmpStore["DocumentRoot"] = "";
		tmpStore["REQUEST_URI"] = "/config/testcgi.sh";

		DoExecTest(ctx);
	}
}
void CgiCallerTest::TestSplitPath() {
	StartTrace(CgiCallerTest.TestSplitPath);

	String path, file;
	CgiCaller::SplitPath("/a/b", path, file);
	assertEqual("/a/", path);
	assertEqual("b", file);

	CgiCaller::SplitPath("/ab", path, file);
	assertEqual("/", path);
	assertEqual("ab", file);
}

void CgiCallerTest::DoExecTest(Context &ctx) {
	StartTrace(CgiCallerTest.DoExecTest);

	Anything tmpStore(ctx.GetTmpStore());
	tmpStore["QUERY_STRING"] = "foo=bar";

	CgiCaller cgi("testcgi");
	CgiParams mapin("ExecTestIn");
	ResultMapper mout("ExecTestOut");
	cgi.Initialize("DataAccessImpl");
	mapin.Initialize("ParameterMapper");
	mout.Initialize("ResultMapper");

	t_assertm(cgi.Exec(ctx, &mapin, &mout), "expected success of cgi call");

	assertEqual(200L, ctx.Lookup(String("Mapper.").Append(coast::http::constants::protocolCodeSlotname), 400L));
	assertEqual("Ok", ctx.Lookup(String("Mapper.").Append(coast::http::constants::protocolMsgSlotname), "Not found"));
	ROAnything abody = ctx.Lookup("Mapper.HTTPBody");
	t_assertm(abody.GetType() == AnyCharPtrType, "body should be string");
	String body = abody.AsString("");
	Trace("body:\n" << body);
	String expected =
		"Content-type: text/plain\n\n"
		"hello_world\n"
		"Query: foo=bar\n"
		"generated by: testcgi.sh\n";

	assertEqualm(expected, body, "produced output");
}

// builds up a suite of testcases, add a line for each testmethod
Test *CgiCallerTest::suite() {
	StartTrace(CgiCallerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, CgiCallerTest, ExecOkTests);
	ADD_CASE(testSuite, CgiCallerTest, TestSplitPath);
	return testSuite;
}
