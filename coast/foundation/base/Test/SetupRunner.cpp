/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyBuiltInSortTest.h"
#include "AnyImplsTest.h"
#include "AnythingConstructorsTest.h"
#include "AnythingDeepCloneTest.h"
#include "AnythingImportExportTest.h"
#include "AnythingIteratorTest.h"
#include "AnythingKeyIndexTest.h"
#include "AnythingLookupTest.h"
#include "AnythingParserSemanticTest.h"
#include "AnythingParserTest.h"
#include "AnythingSTLTest.h"
#include "AnythingTest.h"
#include "ROSimpleAnythingTest.h"
#include "StrSpecialTest.h"
#include "StringTest.h"
#include "StringTestExtreme.h"
#include "StringTokenizer2Test.h"
#include "StringTokenizerTest.h"
#include "SysLogTest.h"
#include "SystemBaseTest.h"
#include "SystemFileTest.h"
#include "TestRunner.h"
#include "TracerTest.h"
#include "TypeTraitsTest.h"
#if !defined(WIN32)
#include "MmapTest.h"
#endif
#include "StringIteratorTest.h"
#include "StringReverseIteratorTest.h"
#include "StringSTLTest.h"
#include "StringStreamTest.h"

void setupRunner(TestRunner &runner) {
	ADD_SUITE(runner, StringTokenizerTest);
	ADD_SUITE(runner, StringTokenizer2Test);
	ADD_SUITE(runner, StringTest);
	ADD_SUITE(runner, AnyImplsTest);
	ADD_SUITE(runner, AnythingConstructorsTest);
	ADD_SUITE(runner, AnythingKeyIndexTest);
	ADD_SUITE(runner, AnythingTest);
	ADD_SUITE(runner, AnythingDeepCloneTest);
	ADD_SUITE(runner, AnythingImportExportTest);
	ADD_SUITE(runner, AnythingLookupTest);
	ADD_SUITE(runner, AnythingParserSemanticTest);
	ADD_SUITE(runner, AnythingParserTest);
	ADD_SUITE(runner, StrSpecialTest);
#if !defined(WIN32)
	ADD_SUITE(runner, MmapTest);
#endif
	ADD_SUITE(runner, StringStreamTest);
	ADD_SUITE(runner, TracerTest);
	ADD_SUITE(runner, ROSimpleAnythingTest);
	ADD_SUITE(runner, SysLogTest);
	ADD_SUITE(runner, AnyBuiltInSortTest);
	ADD_SUITE(runner, TypeTraitsTest);
	ADD_SUITE(runner, AnythingIteratorTest);
	ADD_SUITE(runner, AnythingSTLTest);

	// put last since cirtical system paths are manipulated that may
	// affect proper operation of other tests
	ADD_SUITE(runner, SystemBaseTest);
	ADD_SUITE(runner, SystemFileTest);
	ADD_SUITE(runner, StringTestExtreme);
	ADD_SUITE(runner, StringIteratorTest);
	ADD_SUITE(runner, StringReverseIteratorTest);
	ADD_SUITE(runner, StringSTLTest);
}  // setupRunner
