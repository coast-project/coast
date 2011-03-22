/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SplitCookieResultMapperTest.h"
#include "SplitCookieResultMapper.h"
#include "TestSuite.h"
#include "CacheHandler.h"
#include "HierarchyInstallerWithConfig.h"
#include "AnyUtils.h"

namespace {
	bool setupMappers(ROAnything roaMapperConfigs) {
		StartTrace(SplitCookieResultMapperTest.setupMappers);
		Anything mappersToInitialize;
		ROAnything mapperConfig;
		AnyExtensions::Iterator<ROAnything> aMapperConfigIterator(roaMapperConfigs);
		String mapperName;
		while (aMapperConfigIterator.Next(mapperConfig)) {
			if ( aMapperConfigIterator.SlotName(mapperName) ) {
				mappersToInitialize[mapperName] = Anything();
			}
		}
		HierarchyInstallerWithConfig ip(ResultMapper::gpcCategory, roaMapperConfigs);
		return RegisterableObject::Install(mappersToInitialize, ResultMapper::gpcCategory, &ip);
	}
}
void SplitCookieResultMapperTest::ConfiguredTests()
{
	StartTrace(SplitCookieResultMapperTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if ( !aEntryIterator.SlotName(caseName) ) {
			t_assertm(false, "can not execute with unnamed case name, only named anything slots allowed");
			continue;
		}
		Anything value = caseConfig["Value"].DeepClone();
		String putKeyName = caseConfig["Putkey"].AsString("HTTPHeader");
		if ( t_assertm(setupMappers(caseConfig["MapperConfig"]), "ResultMapper setup must succeed to execute tests") ) {
			String mapperName = caseConfig["MapperConfig"].SlotName(0L);
			ResultMapper *rm = ResultMapper::FindResultMapper(mapperName);
			if ( t_assertm(rm != 0, TString("could not find mapper [") << mapperName.cstr() << "]") ) {
				Context ctx;
				t_assertm(rm->Put(putKeyName, value, ctx), caseName);
				String outputLocation = rm->GetDestinationSlot(ctx);
				outputLocation.Append(rm->getDelim()).Append(putKeyName);
				assertAnyCompareEqual(caseConfig["Expected"], ctx.GetTmpStore(), caseName, rm->getDelim(), rm->getIndexDelim());
				rm->Finalize();
			}
		}
		CacheHandler::Finis();
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *SplitCookieResultMapperTest::suite ()
{
	StartTrace(SplitCookieResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SplitCookieResultMapperTest, ConfiguredTests);
	return testSuite;
}
