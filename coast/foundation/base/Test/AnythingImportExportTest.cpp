/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingImportExportTest.h"

#include "FoundationTestTypes.h"
#include "TestSuite.h"

using namespace coast;

Test *AnythingImportExportTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingImportExportTest, ImportTest);
	ADD_CASE(testSuite, AnythingImportExportTest, ReadFailsTest);
	ADD_CASE(testSuite, AnythingImportExportTest, WriteRead0Test);
	ADD_CASE(testSuite, AnythingImportExportTest, WriteRead1Test);
	ADD_CASE(testSuite, AnythingImportExportTest, WriteRead5Test);
	ADD_CASE(testSuite, AnythingImportExportTest, WriteRead7Test);
	ADD_CASE(testSuite, AnythingImportExportTest, WriteRead8Test);
	ADD_CASE(testSuite, AnythingImportExportTest, RefSlotTest);
	ADD_CASE(testSuite, AnythingImportExportTest, AnyIncludeTest);
	ADD_CASE(testSuite, AnythingImportExportTest, RefBug227Test);
	ADD_CASE(testSuite, AnythingImportExportTest, RefBug231Test);
	ADD_CASE(testSuite, AnythingImportExportTest, RefBug220Test);
	return testSuite;
}

Anything AnythingImportExportTest::init5DimArray(long anzElt) {
	long i0 = 0, i1 = 0;
	char idx0[3] = {0}, idx1[3] = {0};
	Anything anyInit;

	for (i0 = '0'; i0 < anzElt + '0'; i0++) {
		long l0 = (long)(i0 - '0');
		idx0[0L] = (char)i0;

		for (i1 = '0'; i1 < anzElt + '0'; i1++) {
			long l1 = (long)(i1 - '0');
			idx1[0L] = (char)i1;
			anyInit[idx0][idx1] = (l0) + (l1);
		}
	}
	return (anyInit);
}

bool AnythingImportExportTest::check5DimArray(Anything &any0, Anything &any1, long anzElt) {
	long i0 = 0, i1 = 0;
	char idx0[3] = {0}, idx1[3] = {0};
	bool retVal = true;
	Anything any;

	for (i0 = '0'; i0 < anzElt + '0'; i0++) {
		idx0[0L] = (char)i0;
		if (any0[idx0].At("0").At("0").At("0").At("0") != any1[idx0].At("0").At("0").At("0").At("0")) {
			retVal = false;
		}
		for (i1 = '0'; i1 < anzElt + '0'; i1++) {
			idx1[0L] = (char)i1;
			if (any0[idx0][idx1].At("0").At("0").At("0") != any1[idx0][idx1].At("0").At("0").At("0")) {
				retVal = false;
			}
		}
	}
	return (retVal);
}

void AnythingImportExportTest::ImportTest() {
	std::istream *ifp = system::OpenStream("tmp/ImportTest", "any");
	if (t_assertm(ifp != 0, "expected ImportTest file to be there")) {
		Anything config;
		config.Import(*ifp);
		assertEqual(2, config.GetSize());
		Anything expectedValue;
		expectedValue["DLL"][0L] = "CoastSecurity";
		expectedValue.Append("grmbl");
		assertAnyEqual(expectedValue, config);
		delete ifp;
	}
}

void AnythingImportExportTest::WriteRead0Test() {
	Anything any0 = AnythingImportExportTest::init5DimArray(5);
	String buffer;
	{
		OStringStream os(&buffer);
		os << any0;
	}
	IStringStream is(buffer);
	Anything any1;
	is >> any1;

	t_assert(AnythingImportExportTest::check5DimArray(any0, any1, 5) == true);
}

void AnythingImportExportTest::WriteRead1Test() {
	Anything any0 = AnythingImportExportTest::init5DimArray(5);

	std::ostream *os = system::OpenOStream("tmp/anything0", "tst", std::ios::out);
	if (os != 0) {
		any0.PrintOn(*os, true);
		delete os;
	} else {
		assertEqual("'write tmp/anything0.tst'", "'could not write tmp/anything0.tst'");
	}

	Anything any1;
	std::istream *is = system::OpenIStream("tmp/anything0", "tst");
	if (is != 0) {
		any1.Import(*is);
		delete is;
	} else {
		assertEqual("'read tmp/anything0.tst'", "'could not read tmp/anything0.tst'");
	}

	t_assert(AnythingImportExportTest::check5DimArray(any0, any1, 5) == true);
}

void AnythingImportExportTest::WriteRead5Test() {
	Anything any0("Anything: test");

	std::ostream *os = system::OpenOStream("tmp/anything6", "tst", std::ios::out);
	if (os != 0) {
		int i = 0;
		for (i = 0; i < 5; i++) {
			*os << any0;
		}
		delete os;
	} else {
		assertEqual("'write tmp/anything6.tst'", "'could not write tmp/anything6.tst'");
	}

	std::istream *is = system::OpenIStream("tmp/anything6", "tst");
	if (is != 0) {
		Anything any1;
		*is >> any1;
		delete is;
		t_assert(any0 == any1);	 // PATRU ????
		t_assert(any0.AsCharPtr() == any0.AsCharPtr());
		t_assert(any0.AsString() == any0.AsString());
		t_assert(any0.GetSize() == any1.GetSize());
		t_assert(any0.GetType() == any1.GetType());
		t_assert(any0.IsEqual(any1));
	} else {
		assertEqual("'read tmp/anything6.tst'", "'could not read tmp/anything6.tst'");
	}
}

void AnythingImportExportTest::WriteRead7Test() {
	Anything any0("Anything: test");

	String buf;
	{
		OStringStream os(&buf);
		any0.PrintOn(os, false);
	}

	IStringStream is(buf);
	Anything any1;

	any1.Import(is);

	t_assert(any0 == any1);	 // equality works for simple StringImpls!
	assertEqual(any0.AsCharPtr(), any1.AsCharPtr());
	t_assert(any0.AsString() == any1[0L].AsString());
	assertEqual(any0.GetSize(), any1.GetSize());
	t_assert(any0.GetType() == any1.GetType());
	t_assert(any0.IsEqual(any1));  // equality works for simple Strings
}

void AnythingImportExportTest::WriteRead8Test() {
	Anything any0, any1;

	any0 = AnythingImportExportTest::init5DimArray(5);

	std::ostream *os0 = system::OpenOStream("tmp/anything9", "tst", std::ios::out);
	if (os0 != 0) {
		any0.Export(*os0, 0);
		delete os0;
	} else {
		assertEqual("'write tmp/anything9.tst'", "'could not write tmp/anything9.tst'");
	}

	std::istream *is0 = system::OpenIStream("tmp/anything9", "tst");
	if (is0 != 0) {
		any1.Import(*is0);
		delete is0;
	} else {
		assertEqual("'read tmp/anything9.tst'", "'could not read tmp/anything9.tst'");
	}

	t_assert(AnythingImportExportTest::check5DimArray(any0, any1, 5) == true);
}

void AnythingImportExportTest::RefSlotTest() {
	{
		// Check Ref Export and Import of primitive types
		Anything a;

		Anything b;
		b[0L] = "hallo";
		b[1L] = "Peter";

		a["slot1"] = b;
		a["slot2"] = b;

		//		t_assert(((long)(a["slot1"].GetImpl()) != 0));
		//		assertEqual((long)(a["slot1"].GetImpl()), (long)(a["slot2"].GetImpl()));
		Anything c = a.DeepClone();
		//		assertEqual((long)(c["slot1"].GetImpl()), (long)(c["slot2"].GetImpl()));

		String tempString;

		OStringStream os(&tempString);

		a.Export(os);

		IStringStream is(tempString);

		c.Import(is);

		c["slot1"][0L] = "mod";
		assertEqual("mod", c["slot2"][0L].AsCharPtr("no"));
	}
	{
		// test a reference to a named anything
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga}/100 %200}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		IStringStream is(str);
		anyResult.Import(is);
		// test if the link was made
		anyExpected["100"] = anyExpected["200"];
		assertAnyEqual(anyExpected, anyResult);
		// try to modify the 'original' and compare
		anyResult["200"]["c"] = "fooBar";
		anyExpected["200"]["c"] = "fooBar";
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test the inverse order of the reference definition
		// clang-format off
		String str(_QUOTE_( {/1 %200 /200 {/a gugus /b gaga}}));
		// clang-format on
		IStringStream is(str);
		Anything anyResult, anyExpected;
		anyExpected["1"]["a"] = "gugus";
		anyExpected["1"]["b"] = "gaga";
		anyExpected["200"] = anyExpected["1"];
		anyResult.Import(is);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a reference to a named slot
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga}/100 %200.a}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["100"] = anyExpected["200"]["a"];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test unnamed references
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga /c {%200.a}}/100 {c %200.b}%200.a}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected.Append(anyExpected["200"]["a"]);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(anyExpected["200"]["a"]);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a double linked reference
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga /c {%200.a}}/100 {c %200.b}%200.c}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(anyExpected["200"]["a"]);
		anyExpected.Append(anyExpected["200"]["c"]);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a reference to a unnamed slot
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga /c {34}}/100 {c %200.b /e %200.c:0}%200.a}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected.Append(anyExpected["200"]["a"]);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(34L);
		anyExpected["100"]["e"] = anyExpected["200"]["c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed reference to a unnamed slot
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga /c {34}}/100 {c %200.b /e %200.c:0}%200.c:0}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(34L);
		anyExpected.Append(anyExpected["200"]["c"][0L]);
		anyExpected["100"]["e"] = anyExpected["200"]["c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed reference to a unnamed slot with a reference
		// clang-format off
		String str(_QUOTE_( {/200 {/a gugus /b gaga /c {%200.a}}/100 {c %200.b /e %200.c:0}%200.a}));
		// clang-format on
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected.Append(anyExpected["200"]["a"]);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(anyExpected["200"]["a"]);
		anyExpected["100"]["e"] = anyExpected["200"]["c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed reference to a unnamed slot with a reference
		// clang-format off
		String str(_QUOTE_( {/100 {"a" {/200 "b" /300 %"100:1.200"}}}));
		// clang-format on
		Anything anyResult, anyExpected, temp1, temp2;

		temp1["200"] = "b";
		temp1["300"] = temp1["200"];
		temp2.Append("a");
		temp2.Append(temp1);
		anyExpected["100"] = temp2;

		IStringStream is(str);
		anyResult.Import(is);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed and named reference where the last part of the key contains 'special characters'
		// like indexDelimiter or pathDelimiter!!
		/* here is the complete Anything, listed for better understanding
		 especially have a look at the slotnames "23:21:36" and "23.22.28"
		 until now this path had been resolved too and lead to unfindable slots... which is in fact a bug!
		 {
		 /P {
		 /T {
		 /T {
		 { a }
		 { b }
		 { c }
		 }
		 }
		 }
		 /T {
		 /"2002" {
		 /"03" {
		 /"20" {
		 %"P.T.T:0"
		 /"23:21:36" %"P.T.T:1"
		 /"23.22.28" %"P.T.T:2"
		 }
		 }
		 }
		 }
		 }
		 */
		// clang-format off
		String str(_QUOTE_(
			{ /P {/T {/T { {	a} {b} {c}}}}/T {/"2002" {/"03" {/"20" {%"P.T.T:0" /"23:21:36" %"P.T.T:1" /"23.22.28" %"P.T.T:2"}}}}}));
		// clang-format on
		Anything anyResult, anyExpected, temp1, temp2;

		anyExpected["P"]["T"]["T"][0L][0L] = "a";
		anyExpected["P"]["T"]["T"][1L][0L] = "b";
		anyExpected["P"]["T"]["T"][2L][0L] = "c";
		anyExpected["T"]["2002"]["03"]["20"][0L] = anyExpected["P"]["T"]["T"][0L];
		anyExpected["T"]["2002"]["03"]["20"]["23:21:36"] = anyExpected["P"]["T"]["T"][1L];
		anyExpected["T"]["2002"]["03"]["20"]["23.22.28"] = anyExpected["P"]["T"]["T"][2L];

		IStringStream is(&str);
		anyResult.Import(is);
		assertAnyEqual(anyExpected, anyResult);
	}
}

void AnythingImportExportTest::AnyIncludeTest() {
	{
		// Test an include in a unnamed slot with a relativ uri without localhost
		Anything anyMain, anyIncl, anyRef;
		// clang-format off
		String strMain(_QUOTE_( {/200 {/a gugus /b gaga}!"file:///include.any"}));
		String strIncl(_QUOTE_( {/100 {/d foo /e frim}}));
		String strRef(_QUOTE_( {/200 {/a gugus /b gaga} {/100 {/d foo /e frim}}}));
		// clang-format on
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			std::iostream *pStream = system::OpenOStream("include", "any");
			if (pStream != 0) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
	{
		// Test an include in a unnamed slot with a absolut uri without localhost
		Anything anyMain, anyIncl, anyRef;

		String strMain;
		strMain << "{ /200 { /a gugus /b gaga } !\"file:///" << system::GetFilePath("include", "any") << "\"}";
		// clang-format off
		String strIncl(_QUOTE_( {/100 {/d foo /e frim}}));
		String strRef(_QUOTE_( {/200 {/a gugus /b gaga} {/100 {/d foo /e frim}}}));
		// clang-format on
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			std::iostream *pStream = system::OpenOStream("include", "any");
			if (pStream != 0) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
	//!@FIXME needs further analysis what to do when specifying a machine name
	// it seems that when a machine name other than localhost is used wh should use FTP to get the file
	//	{
	//		// Test an include in a named slot with a relativ uri with localhost
	//		Anything anyMain, anyIncl, anyRef;
	//		// clang-format off
	//		String strMain(_QUOTE_({ /200 { /a gugus /b gaga } /300 !"file://localhost/include.any" }));
	//		String strIncl(_QUOTE_({ /100 { /d foo /e frim } }));
	//		String strRef (_QUOTE_({ /200 { /a gugus /b gaga } /300 { /100 { /d foo /e frim } } }));
	//		// clang-format on
	//		{
	//			IStringStream is(strIncl);
	//			anyIncl.Import(is);
	//			std::iostream *pStream = system::OpenOStream("include", "any");
	//			if (pStream)
	//			{
	//				anyIncl.Export(*pStream);
	//				delete pStream;
	//			}
	//		}
	//		{
	//			IStringStream is(strMain);
	//			anyMain.Import(is);
	//		}
	//		{
	//			IStringStream is(strRef);
	//			anyRef.Import(is);
	//		}
	//		assertAnyEqual(anyRef, anyMain);
	//	}
	//	{
	//		// Test an include with missing of the file
	//		Anything anyMain, anyIncl, anyRef;
	//		// clang-format off
	//		String strMain(_QUOTE_({ /200 { /a gugus /b gaga } /300 !"file://localhost/gugus.any" }));
	//		String strIncl(_QUOTE_({ /100 { /d foo /e frim } }));
	//		String strRef (_QUOTE_({ /200 { /a gugus /b gaga } /300  * }));
	//		// clang-format on
	//		{
	//			IStringStream is(strIncl);
	//			anyIncl.Import(is);
	//			std::iostream *pStream = system::OpenOStream("include", "any");
	//			if (pStream)
	//			{
	//				anyIncl.Export(*pStream);
	//				delete pStream;
	//			}
	//		}
	//		{
	//			IStringStream is(strMain);
	//			anyMain.Import(is);
	//		}
	//		{
	//			IStringStream is(strRef);
	//			anyRef.Import(is);
	//		}
	//		assertAnyEqual(anyRef, anyMain);
	//	}
	{
		// Test an include with a query for a subtree in the include file
		Anything anyMain, anyIncl, anyRef;
		// clang-format off
		String strMain(_QUOTE_( {/200 {/a gugus /b gaga}!"file:///include.any?100.d:0"}));
		String strIncl(_QUOTE_( {/100 {/d {foo}/e frim}}));
		String strRef(_QUOTE_( {/200 {/a gugus /b gaga}"foo"}));
		// clang-format on
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			std::iostream *pStream = system::OpenOStream("include", "any");
			if (pStream != 0) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
	{
		// Test an include with a query for a subtree with escaped path and index delims in the include file
		Anything anyMain, anyIncl, anyRef;
		// clang-format off
		String strMain(_QUOTE_( {/200 {/a gugus /b gaga}!"file:///include.any?a\.b\.c.d:0.\:bla"}));
		String strIncl(_QUOTE_( {/"a.b.c" {/d { {/":bla" foo}}/e frim}}));
		String strRef(_QUOTE_( {/200 {/a gugus /b gaga}"foo"}));
		// clang-format on
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			std::iostream *pStream = system::OpenOStream("include", "any");
			if (pStream != 0) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
	{
		// Test an include with a query for a subtree in the include file that does not exist
		Anything anyMain, anyIncl, anyRef;
		// clang-format off
		String strMain(_QUOTE_( {/200 {/a gugus /b gaga}!"file:///include.any?d:0"}));
		String strIncl(_QUOTE_( {/100 {/d {foo}/e frim}}));
		String strRef(_QUOTE_( {/200 {/a gugus /b gaga}*}));
		// clang-format on
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			std::iostream *pStream = system::OpenOStream("include", "any");
			if (pStream != 0) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
}

void AnythingImportExportTest::ReadFailsTest() {
	String incompleteAny("{ /Slot { /No \"Ending curly bracket\"");
	IStringStream is(incompleteAny);
	Anything any;
	t_assert(!any.Import(is));
	assertEqual("Ending curly bracket", any["Slot"]["No"].AsString("x"));
}

void AnythingImportExportTest::RefBug227Test() {
	{
		// test an empty Anything at dotted slotname
		// clang-format off
		String str(_QUOTE_( {/200 {/"a.b.c" *}/name blub}));
		// clang-format on
		Anything anyResult, anyExpected;
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["200"]["a.b.c"] = Anything();
		anyExpected["name"] = "blub";
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test an unnamed reference
		// clang-format off
		String str(_QUOTE_( {/200 {/"a.b.c" *}/name blub /100 {%name /e 123}/300 {%100}}));
		// clang-format on
		Anything anyResult, anyExpected;
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["200"]["a.b.c"] = Anything();
		anyExpected["name"] = "blub";
		anyExpected["100"][0L] = anyExpected["name"];
		anyExpected["100"]["e"] = 123;
		anyExpected["300"][0L] = anyExpected["100"];
		assertAnyEqual(anyExpected, anyResult);
	}
}

void AnythingImportExportTest::RefBug231Test() {
	{
		// test an unnamed reference
		// clang-format off
		String str(_QUOTE_(
			{ /BackendName blabla /Params {/Name "avt" /Server {/RendererMapper {%BackendName}}/Port 443 /Timeout 10 /UseSSL 1}/a.b.c {3 /NewRef %BackendName}}));
		// clang-format on
		Anything anyResult, anyExpected;
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["BackendName"] = "blabla";
		anyExpected["Params"]["Name"] = "avt";
		anyExpected["Params"]["Server"]["RendererMapper"][0L] = anyExpected["BackendName"];
		anyExpected["Params"]["Port"] = 443L;
		anyExpected["Params"]["Timeout"] = 10L;
		anyExpected["Params"]["UseSSL"] = 1L;
		anyExpected["a.b.c"][0L] = 3L;
		anyExpected["a.b.c"]["NewRef"] = anyExpected["BackendName"];
		assertAnyEqual(anyExpected, anyResult);
	}
}

void AnythingImportExportTest::RefBug220Test() {
	{
		// test escaped reference
		// clang-format off
		String str(_QUOTE_( {/"a.b.c" {33}/name blub /300 {%"a\.b\.c:0"}}));
		// clang-format on
		Anything anyResult, anyExpected;
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["a.b.c"][0L] = 33;
		anyExpected["name"] = "blub";
		anyExpected["300"][0L] = anyExpected["a.b.c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test escaped reference
		// clang-format off
		String str(_QUOTE_( {/"a.b.c" {/level1 {/level2 33}}/name blub /300 {%"a\.b\.c.level1.level2"}}));
		// clang-format on
		Anything anyResult, anyExpected;
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["a.b.c"]["level1"]["level2"] = 33;
		anyExpected["name"] = "blub";
		anyExpected["300"][0L] = anyExpected["a.b.c"]["level1"]["level2"];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test escaped reference
		// clang-format off
		String str(_QUOTE_( {/"a.b.c" {/":3" {/level2 33}}/name blub /300 {%"a\.b\.c.\:3.level2"}}));
		// clang-format on
		Anything anyResult, anyExpected;
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["a.b.c"][":3"]["level2"] = 33;
		anyExpected["name"] = "blub";
		anyExpected["300"][0L] = anyExpected["a.b.c"][":3"]["level2"];
		assertAnyEqual(anyExpected, anyResult);
	}
}
