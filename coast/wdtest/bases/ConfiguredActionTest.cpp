/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConfiguredActionTest.h"
#include "TestSuite.h"
#include "Page.h"
#include "Server.h"
#include "Role.h"
#include "Action.h"
#include "AnyUtils.h"
#include "Timers.h"
#include <iostream>

//---- ConfiguredActionTest ----------------------------------------------------------------
ConfiguredActionTest::ConfiguredActionTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(ConfiguredActionTest.ConfiguredActionTest);
}

TString ConfiguredActionTest::getConfigFileName()
{
	return "ConfiguredActionTestConfig";
}

ConfiguredActionTest::~ConfiguredActionTest()
{
	StartTrace(ConfiguredActionTest.Dtor);
}

void ConfiguredActionTest::setUp()
{
	StartTrace(ConfiguredActionTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
}

void ConfiguredActionTest::TestCases()
{
	StartTrace(ConfiguredActionTest.TestCases);

	Anything testCases;
	long runOnlySz = GetConfig()["RunOnly"].GetSize();
	if (runOnlySz > 0) {
		String warning;
		warning << "ConfiguredActionTest not complete : Running only " << runOnlySz << " Testcases";
		t_assertm(false, (const char *)warning);
		for (long i = 0; i < runOnlySz; ++i) {
			String testCaseName = GetConfig()["RunOnly"][i].AsString("Unknown");
			testCases[testCaseName] = GetTestCaseConfig()[testCaseName].DeepClone();
		}
		TraceAny(testCases, "TestCases");
	} else {
		testCases = GetTestCaseConfig().DeepClone();
	}

	long sz = testCases.GetSize();
	for (long i = 0; i < sz; ++i) {
		if ( i > 0 ) {
			std::cerr << ".";
		}
		String testCaseName = testCases.SlotName(i);
		DoTest(PrepareConfig(testCases[i]), testCaseName);
	}
}

Anything ConfiguredActionTest::PrepareConfig(Anything originalConfig)
{
	StartTrace(ConfiguredActionTest.PrepareConfig);

	if (!originalConfig.IsDefined("UseConfig")) {
		return originalConfig;
	}

	String useConfigName = originalConfig["UseConfig"].AsString();
	Anything result = PrepareConfig(GetTestCaseConfig()[useConfigName].DeepClone());

	Anything replaceList = originalConfig["Replace"];

	long sz = replaceList.GetSize();
	for (long i = 0; i < sz; ++i) {
		SlotPutter::Operate(replaceList[i], result, String(replaceList.SlotName(i)));
	}

	TraceAny(result, "Patched Config");
	return result;
}

void ConfiguredActionTest::DoTest(Anything testCase, const char *testCaseName)
{
	StartTrace1(ConfiguredActionTest.DoTest, "<" << NotNull(testCaseName) << ">");
	Context ctx;
	DoTest(testCase, testCaseName, ctx);
	RequestTimeLogger(ctx);
}

void ConfiguredActionTest::DoTest(Anything testCase, const char *testCaseName, Context &ctx)
{
	StartTrace1(ConfiguredActionTest.DoTest, "<" << NotNull(testCaseName) << ">");

	DoTestWithContext(testCase, testCaseName, ctx);
	// do existence tests
	CheckStores(testCase["Result"], ctx, testCaseName, exists);
	// non-existence tests
	CheckStores(testCase["NotResult"], ctx, testCaseName, notExists);
}

void ConfiguredActionTest::CheckStoreContents(ROAnything anyInput, ROAnything anyMaster, const char *storeName, const char *testCaseName, char delimSlot, char delimIdx, eResultCheckType rct)
{
	StartTrace(ConfiguredActionTest.CheckStoreContents);
	if (rct == exists) {
		String strTestPath = storeName;
		strTestPath << "." << testCaseName;
		assertAnyCompareEqual(anyMaster, anyInput, strTestPath, delimSlot, delimIdx);
	} else if (rct == notExists) {
		// anyInput == ctxToBeChecked
		// anyMaster == notExpected
		// ------------------------------------------------------------
		// With this version, it is only possible to check for absence
		// of named slots (cannot have unnamed slots so far).
		// ------------------------------------------------------------

		// generate list of paths and check for existence
		Anything pathList;
		GeneratePathList(pathList, anyMaster, "", delimSlot);

		TraceAny(anyInput, "Store to be checked (" << storeName << "):");
		TraceAny(pathList, "List of paths to check for non-existence:");

		ROAnything luResult;
		if ( !pathList.IsNull() ) {
			for (long i = 0, sz = pathList.GetSize(); i < sz; ++i) {
				if ( anyInput.LookupPath(luResult, pathList[i].AsCharPtr()) ) {
					// error, if lookup succeeds
					String strfail(testCaseName);
					strfail << "\n\t" << "Slot [" << pathList[i].AsCharPtr() << "] should not exist in " << storeName << "!" ;
					t_assertm(false, (const char *)strfail);
				}
			}
		}
	}
}

void ConfiguredActionTest::GeneratePathList(Anything &pathList, ROAnything &notExpected, String const & pathSoFar, char delimSlot)
{
	StartTrace(ConfiguredActionTest.GeneratePathList);

	for (long i = 0, sz = notExpected.GetSize(); i < sz; ++i) {
		String path(pathSoFar);
		path << notExpected.SlotName(i);
		if (notExpected[i].GetType() == AnyArrayType ) {
			// continue recursively
			ROAnything next(notExpected[i]);
			path << delimSlot;
			GeneratePathList(pathList, next, path, delimSlot);
		} else {
			// leaf reached, add path to list
			pathList[pathList.GetSize()] = path;
		}
	}
}

void ConfiguredActionTest::CheckStores(ROAnything expected, Context &ctxToCheck, const char *testCaseName, eResultCheckType rct)
{
	StartTrace(ConfiguredActionTest.CheckStores);
	char delimSlot, delimIdx;
	delimSlot = expected["Delim"].AsCharPtr(".")[0L];
	delimIdx = expected["IndexDelim"].AsCharPtr(":")[0L];
	if (expected.IsDefined("SessionStore") ) {
		CheckStoreContents(ctxToCheck.GetSessionStore(), expected["SessionStore"], "SessionStore", testCaseName, delimSlot, delimIdx, rct);
	}
	if (expected.IsDefined("RoleStore") ) {
		CheckStoreContents(ctxToCheck.GetRoleStoreGlobal(), expected["RoleStore"], "RoleStore", testCaseName, delimSlot, delimIdx, rct);
	}
	if (expected.IsDefined("Query") ) {
		CheckStoreContents(ctxToCheck.GetQuery(), expected["Query"], "Query", testCaseName, delimSlot, delimIdx, rct);
	}
	if (expected.IsDefined("TmpStore") ) {
		CheckStoreContents(ctxToCheck.GetTmpStore(), expected["TmpStore"], "TmpStore", testCaseName, delimSlot, delimIdx, rct);
	}
}

void ConfiguredActionTest::DoTestWithContext(ROAnything testCase, const String &testCaseName, Context &ctx)
{
	StartTrace(ConfiguredActionTest.DoTestWithContext);
	DoTestWithContext(testCase.DeepClone(), testCaseName, ctx);
}

void ConfiguredActionTest::DoTestWithContext(Anything testCase, const String &testCaseName, Context &ctx)
{
	StartTrace(ConfiguredActionTest.DoTestWithContext);
	TraceAny(testCase, "Config of " << testCaseName);

	AlterTestStoreHook(testCase);
	PutInStore(testCase["SessionStore"], ctx.GetSessionStore());
	PutInStore(testCase["RoleStore"], ctx.GetRoleStoreGlobal());
	// Can not use real Session Store because Lookup does not find it ! - fix me
	TraceAny(ctx.GetRoleStoreGlobal(), "SessionStore");
	PutInStore(testCase["TmpStore"], ctx.GetTmpStore());
	PutInStore(testCase["Query"], ctx.GetQuery());
	PutInStore(testCase["Env"], ctx.GetEnvStore());

	if (!testCase.IsDefined("Server") && GetConfig().IsDefined("Server")) {
		testCase["Server"] = GetConfig()["Server"].DeepClone();
	}
	if (testCase.IsDefined("Server")) {
		Server *s = Server::FindServer(testCase["Server"].AsCharPtr("Server"));
		ctx.SetServer(s);
	}
	if (testCase.IsDefined("Page")) {
		ctx.Push(testCase["Page"].AsCharPtr("Page"), Page::FindPage(testCase["Page"].AsCharPtr("Page")));
	}
	if (testCase.IsDefined("Role")) {
		ctx.Push(testCase["Role"].AsCharPtr("Role"), Role::FindRole(testCase["Role"].AsCharPtr("Role")));
	}
	TraceAny(testCase["TmpStore"], "Language");
	if (testCase["TmpStore"].IsDefined("Language")) {
		ctx.SetLanguage(testCase["TmpStore"]["Language"].AsCharPtr("D"));
	}

	bool expected = testCase["ExpectedResult"].AsBool(true);

	String token = testCase["StartToken"].AsString("TheAction");
	t_assertm(expected == Action::ExecAction(token, ctx, testCase["TheAction"]), (const char *)testCaseName);
	TraceAny(ctx.GetTmpStore(), "tmp store after action");
	String expectedToken = testCase["ExpectedToken"].AsString("TheAction");
	assertEqualm(expectedToken, token, (const char *)testCaseName);
}

void ConfiguredActionTest::AlterTestStoreHook(Anything &testCase)
{
	StartTrace(ConfiguredActionTest.AlterTestStoreHook);
}

// builds up a suite of testcases, add a line for each testmethod
Test *ConfiguredActionTest::suite ()
{
	StartTrace(ConfiguredActionTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConfiguredActionTest, TestCases);
	return testSuite;
}
