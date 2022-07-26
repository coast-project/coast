/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemFileTest.h"

#include "SystemLog.h"
#include "TestSuite.h"
#include "boost/bind.hpp"

using namespace coast;

void SystemFileTest::initPathTest() {
	String pathList(system::GetPathList());
	String rootDir(system::GetRootDir());

	t_assert(pathList.Length() > 0);
	t_assert(rootDir.Length() > 0);

	const char tmpRoot[] = "/usr/local";
	const char tmpPath[] = "bin";
	system::InitPath(tmpRoot, tmpPath);

	assertEqual("/usr/local", system::GetRootDir());
	assertEqual("bin", system::GetPathList());

#if defined(__linux)
	unsetenv("COAST_ROOT");
	unsetenv("COAST_PATH");
#else
	putenv("COAST_ROOT=");
	putenv("COAST_PATH=");
#endif
	// should have no effect now, path and root should remain
	system::InitPath();

	assertEqual("/usr/local", system::GetRootDir());
	assertEqual("bin", system::GetPathList());

	// test InitPath with root argument, path will be restored to default ".:config:src:"
	system::InitPath("/usr/local/bin");
	assertEqual("/usr/local/bin", system::GetRootDir());
	// path will be restored to ".:config:src:"
	assertEqual(".:config:src:", system::GetPathList());

	// test InitPath with path argument, root will be restored to default "."
	system::InitPath(0, "config");

	assertEqual(".", system::GetRootDir());
	assertEqual("config", system::GetPathList());

#if defined(WIN32)
	// test InitPath with NT specific arguments
	system::InitPath("d:\\gugus/blabla", "bin");

	assertEqual("d:/gugus/blabla", system::GetRootDir());
	assertEqual("bin", system::GetPathList());
#endif

	// restore old settings
	system::SetPathList(pathList, false);
	system::SetRootDir(rootDir, false);
	assertEqual(rootDir, system::GetRootDir());
	assertEqual(pathList, system::GetPathList());
}

void SystemFileTest::statTests() {
	t_assertm(system::IsDirectory("."), "expected '.' to be a directory");
	t_assertm(system::IsDirectory(".."), "expected '.' to be a directory");
	t_assertm(!system::IsDirectory("config/Test.any"), "expected 'Test.any' to be a file");
	t_assertm(!system::IsDirectory("config/Tracer.any"), "expected 'Tracer.any' to be a file");
	t_assertm(!system::IsRegularFile("."), "expected '.' to be a directory");
	t_assertm(system::IsRegularFile("config/SystemFileTest.any"), "expected 'SystemTest.any' to be a file");
	t_assertm(!system::IsRegularFile(".."), "expected '.' to be a directory");
	t_assertm(system::IsRegularFile("config/Test.any"), "expected 'Test.any' to be a file");
	t_assertm(system::IsRegularFile("config/Tracer.any"), "expected 'Tracer.any' to be a file");
	String strLinkToPrjRunTest("aLinkToTestAny");
#if !defined(WIN32)
	if (assertComparem(system::eSuccess, equal_to, system::CreateSymbolicLink("config/Test.any", strLinkToPrjRunTest),
					   "expected creation of symbolic link to file to succeed")) {
		t_assertm(system::IsSymbolicLink(strLinkToPrjRunTest), "expected link to be valid");
		t_assertm(!system::IsDirectory(strLinkToPrjRunTest), "expected link not to be a directory");
		t_assertm(system::IsRegularFile(strLinkToPrjRunTest), "expected link to point to a regular file");
		assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strLinkToPrjRunTest),
					   "expected removal of symbolic link to succeed");
	}
	String strLinkToDirectory("aLinkToDirectory");
	if (assertComparem(system::eSuccess, equal_to, system::CreateSymbolicLink("config", strLinkToDirectory),
					   "expected creation of symbolic link to file to succeed")) {
		t_assertm(system::IsSymbolicLink(strLinkToDirectory), "expected link to be valid");
		t_assertm(system::IsDirectory(strLinkToDirectory), "expected link to point to a directory");
		t_assertm(!system::IsRegularFile(strLinkToDirectory), "expected link not to be a regular file");
		assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strLinkToDirectory),
					   "expected removal of symbolic link to succeed");
	}
#endif
}

void SystemFileTest::pathListTest() {
	String pathList(system::GetPathList());

	t_assert(pathList.Length() > 0);
	String newPath("/:/usr:/usr/local");

	system::SetPathList(newPath, false);

	assertEqual(newPath, system::GetPathList());

	// restore old pathList
	system::SetPathList(pathList, false);
}

void SystemFileTest::rooDirTest() {
	String rootDir(system::GetRootDir());

	t_assert(rootDir.Length() > 0);

	String newRoot("/");

	system::SetRootDir(newRoot, false);

	assertEqual(newRoot, system::GetRootDir());

	// restore old root dir
	system::SetRootDir(rootDir, false);
}

void SystemFileTest::CWDTests() {
	String wd;
	system::GetCWD(wd);
	t_assertm(system::IsAbsolutePath(wd), "working dir should be absolute");
	t_assertm(wd.Length() > 0, "working dir should not be empty");
	t_assertm(wd != ".", "working dir should not be default .");

	String upper = wd.SubString(0, wd.StrRChr(system::Sep()));
	t_assertm(system::ChangeDir(".."), "shouldn't fail, .. dir always accessible");
	t_assertm(upper.Length() > 0, "should have upper directory");
	assertEqual(0L, wd.Contains(upper));
	t_assertm(upper.Length() < wd.Length(), "should really be upper if .. isn't mount point");

	String u;
	system::GetCWD(u);
	assertEqual(upper, u);
	t_assertm(u.Length() > 0, "huh, directory empty");
	t_assertm(u != ".", "working dir should not be default .");

	String strCwd;
	system::GetCWD(strCwd);
	String root;
	root << system::Sep();
	t_assertm(system::ChangeDir(root), "shouldn't fail, root dir always accessible");
	String r1;
	system::GetCWD(r1);
#if defined(WIN32)
	// on WIN32, if we change to 'root' it is actually the root Dir of the current drive
	// in the form <drive>:<system::SEP>
	char driveLetter;
	t_assertm(system::GetDriveLetter(strCwd, driveLetter), TString("couldn't get drive letter from [") << strCwd << "]");
	root = "";
	root << driveLetter << ":" << system::Sep();
	assertEqual(root, r1);
#else
	assertEqual(root, r1);
#endif
	t_assertm(r1.Length() > 0, "huh, directory empty");
	t_assertm(r1 != ".", "working dir should not be default .");

	String invaliddir;
	invaliddir << system::Sep() << "wuggi" << system::Sep() << "waggi" << system::Sep() << "gugus" << system::Sep() << "this"
			   << system::Sep() << "directory" << system::Sep() << "doesnt" << system::Sep() << "exist";
	t_assertm(!system::ChangeDir(invaliddir), "should fail, dir never exists");
	String r2;
	system::GetCWD(r2);
	assertEqual(root, r2);
	t_assertm(r2.Length() > 0, "huh, directory empty");
	t_assertm(r2 != ".", "working dir should not be default .");

	//-- clean up
	t_assertm(system::ChangeDir(wd), "shouldn't fail, .. original dir always accessible");
	String w1;
	system::GetCWD(w1);
	assertEqual(wd, w1);
	t_assertm(w1.Length() > 0, "huh, directory empty");
	t_assertm(w1 != ".", "working dir should not be default .");
}

void SystemFileTest::IsAbsolutePathTest() {
	t_assertm(system::IsAbsolutePath("/"), "should be absolute");
	t_assertm(system::IsAbsolutePath("//"), "should be absolute");
	t_assertm(system::IsAbsolutePath("/blabla/"), "should be absolute");
	t_assertm(!system::IsAbsolutePath("./"), "should not be absolute");
	t_assertm(!system::IsAbsolutePath("../"), "should not be absolute");
	t_assertm(!system::IsAbsolutePath("config"), "should not be absolute");
#if defined(WIN32)
	t_assertm(system::IsAbsolutePath("\\"), "should be absolute");
	t_assertm(system::IsAbsolutePath("\\gaga"), "should be absolute");
	// 'normal' WIN32 drive notation, is NOT absolute, uses current dir on drive
	t_assertm(!system::IsAbsolutePath("d:"), "should not be absolute");
	// 'normal' WIN32 drive notation with absolute path
	t_assertm(system::IsAbsolutePath("d:\\"), "should be absolute");
	// 'normal' WIN32 drive notation with absolute path
	t_assertm(system::IsAbsolutePath("d:/"), "should be absolute");
	// some shells use the following absolute notation
	t_assertm(system::IsAbsolutePath("//d/"), "should be absolute");
	t_assertm(system::IsAbsolutePath("/d/"), "should be absolute");
#endif
}

void SystemFileTest::ResolvePathTest() {
	String result, expected;
	result = "";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "/";
	system::ResolvePath(result);
	expected = "/";
	assertEqual(expected, result);
	result = ".";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "./";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "..";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "../";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "/../";
	system::ResolvePath(result);
	expected = "/";
	assertEqual(expected, result);
	result = "/../.";
	system::ResolvePath(result);
	expected = "/";
	assertEqual(expected, result);
	result = "gugus";
	system::ResolvePath(result);
	expected = "gugus";
	assertEqual(expected, result);
	result = "./gugus";
	system::ResolvePath(result);
	expected = "gugus";
	assertEqual(expected, result);
	result = "././gugus";
	system::ResolvePath(result);
	expected = "gugus";
	assertEqual(expected, result);
	result = "././././././gugus";
	system::ResolvePath(result);
	expected = "gugus";
	assertEqual(expected, result);
	result = "././././././";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "../../../../abc";
	system::ResolvePath(result);
	expected = "abc";
	assertEqual(expected, result);
	result = "./gugus/..";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "./gugus/../";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "./gugus/../..";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "./gugus/../../blabla";
	system::ResolvePath(result);
	expected = "blabla";
	assertEqual(expected, result);
	result = "./gugus/../../blabla/..";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "./gugus/../../blabla/../";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "./gugus/../../..";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "/bla/../../huhu/./ga/";
	system::ResolvePath(result);
	expected = "/huhu/ga/";
	assertEqual(expected, result);
	result = "./gugus/hallo.txt/";
	system::ResolvePath(result);
	expected = "gugus/hallo.txt/";
	assertEqual(expected, result);
	result = "./gugus./hallo.txt./";
	system::ResolvePath(result);
	expected = "gugus./hallo.txt./";
	assertEqual(expected, result);
	result = "/gugus/../..";
	system::ResolvePath(result);
	expected = "/";
	assertEqual(expected, result);
	result = " .";
	system::ResolvePath(result);
	expected = " .";
	assertEqual(expected, result);
	result = "/gugus/../..";
	result.PutAt(4, '\0');
	system::ResolvePath(result);
	expected = "/gug";
	assertEqual(expected, result);
	result = "/gugus/../..";
	result.PutAt(0, '\0');
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "/gugus/../..";
	result.PutAt(result.Length() - 1L, '\0');
	system::ResolvePath(result);
	expected = "/";
	assertEqual(expected, result);
	result = "........";
	system::ResolvePath(result);
	expected = "........";
	assertEqual(expected, result);
	result = "./////";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "/foo/bla/stop";
	system::ResolvePath(result);
	expected = "/foo/bla/stop";
	assertEqual(expected, result);
#if defined(WIN32)
	result = "\\";
	system::ResolvePath(result);
	expected = "/";
	assertEqual(expected, result);
	result = ".\\";
	system::ResolvePath(result);
	expected = "";
	assertEqual(expected, result);
	result = "d:";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = " :";
	system::ResolvePath(result);
	expected = " :";
	assertEqual(expected, result);
	result = "d:/";
	system::ResolvePath(result);
	expected = "d:/";
	assertEqual(expected, result);
	result = "d:\\";
	system::ResolvePath(result);
	expected = "d:/";
	assertEqual(expected, result);
	result = "//d/";
	system::ResolvePath(result);
	expected = "d:/";
	assertEqual(expected, result);
	result = "d:/gugus";
	system::ResolvePath(result);
	expected = "d:/gugus";
	assertEqual(expected, result);
	result = "d:./";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = "d:./gugus";
	system::ResolvePath(result);
	expected = "d:gugus";
	assertEqual(expected, result);
	result = "d:./gugus/..";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = "d:./gugus/../..";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = "d:./gugus/bla/hu";
	system::ResolvePath(result);
	expected = "d:gugus/bla/hu";
	assertEqual(expected, result);
	result = "d:./gugus\\bla/hu\\";
	system::ResolvePath(result);
	expected = "d:gugus/bla/hu/";
	assertEqual(expected, result);
	result = "d:./\\/\\";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = "d:./\\/\\/";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = "d:..";
	system::ResolvePath(result);
	expected = "d:";
	assertEqual(expected, result);
	result = "d:/..";
	system::ResolvePath(result);
	expected = "d:/";
	assertEqual(expected, result);
	result = "d:./gugus/../gugus";
	system::ResolvePath(result);
	expected = "d:gugus";
	assertEqual(expected, result);
	result = "d::";
	system::ResolvePath(result);
	expected = "d::";
	assertEqual(expected, result);
	result = "d:/:";
	system::ResolvePath(result);
	expected = "d:/:";
	assertEqual(expected, result);
	result = "d:........";
	system::ResolvePath(result);
	expected = "d:........";
	assertEqual(expected, result);
	result = "d:d:";
	system::ResolvePath(result);
	expected = "d:d:";
	assertEqual(expected, result);
	result = ":";
	system::ResolvePath(result);
	expected = ":";
	assertEqual(expected, result);
	result = ":.";
	system::ResolvePath(result);
	expected = ":.";
	assertEqual(expected, result);
#endif
}

void SystemFileTest::OpenStreamTest() {
	std::iostream *Ios = system::OpenStream("Tracer.any");
	t_assert(Ios == NULL);	// should not be found!

	delete Ios;

	// open file with relative path
	Ios = system::OpenStream("config/Tracer.any");
	t_assert(Ios != NULL);

	delete Ios;

	// deprecated:

	// search file with path
	Ios = system::OpenStream("Tracer", "any");
	t_assert(Ios != NULL);	// should be found

	delete Ios;

	// open file with relative path
	Ios = system::OpenStream("config/Tracer", "any");
	t_assert(Ios != NULL);

	delete Ios;

	// open file with relative path for writing
	Ios = system::OpenStream("tmp/Test1", "tst", std::ios::out);
	t_assert(Ios != NULL);
	if (Ios != 0) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
	}
}

void SystemFileTest::OpenStreamWithSearchTest() {
	std::iostream *Ios = system::OpenStreamWithSearch("Tracer.any");
	t_assert(Ios != NULL);

	delete Ios;

	// open file with relative path
	Ios = system::OpenStreamWithSearch("config/Tracer.any");
	t_assert(Ios != NULL);

	delete Ios;

	// open file with relative path for writing
	Ios = system::OpenStreamWithSearch("tmp/Test1.tst", std::ios::out);
	t_assert(Ios != NULL);
	if (Ios != 0) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
	}
}

void SystemFileTest::OpenIStreamTest() {
	// open file with relative path
	std::iostream *Ios = system::OpenIStream("config/Tracer.any");

	t_assert(Ios != NULL);
	if (Ios != 0) {
		Anything dbgTest;
		(*Ios) >> dbgTest;
		t_assert(!!(*Ios));
		t_assert(dbgTest.GetSize() > 0);
		delete Ios;
	}

	// deprecated:

	// open file with relative path
	Ios = system::OpenIStream("config/Tracer", "any");

	t_assert(Ios != NULL);
	if (Ios != 0) {
		Anything dbgTest;
		(*Ios) >> dbgTest;
		t_assert(!!(*Ios));
		t_assert(dbgTest.GetSize() > 0);
		delete Ios;
	}
}

void SystemFileTest::OpenOStreamTest() {
	// open file with relative path for writing
	std::iostream *Ios = system::OpenOStream("tmp/Test.tst");

	t_assert(Ios != NULL);
	if (Ios != 0) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
	}

	// deprecated:

	// open file with relative path for writing
	Ios = system::OpenOStream("tmp/Test", "tst");

	t_assert(Ios != NULL);
	if (Ios != 0) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
	}
}

void SystemFileTest::testGetFilePath(boost::function<String()> func, const String &notFoundResult) {
	StartTrace(SystemFileTest.testGetFilePath);
	String subPath("./config/Tracer.any");

	system::Chmod(subPath, 0400);  // set it read only

	String path(func());

	system::ResolvePath(subPath);
	assertEqual(subPath, path.SubString(path.Length() - subPath.Length()));

#if defined(WIN32)
	// because NT lacks easy hiding of files or directories the file is renamed
	String hiddenSubPath;
	long filenamePos = subPath.StrRChr(coast::system::Sep());
	if (filenamePos > -1) {
		hiddenSubPath = subPath.SubString(0L, filenamePos + 1).Append(".").Append(subPath.SubString(filenamePos + 2));
	} else {
		hiddenSubPath = subPath;
	}

	int status = system::io::rename(subPath, hiddenSubPath);
#else
	system::Chmod(subPath, 0000);  // set it to no access
#endif

	path = func();
	// should fail now.... therefore path is equal
	assertEqual(notFoundResult, path);

#if defined(WIN32)
	system::io::rename(hiddenSubPath, subPath);	 // clean up to make it usable again
#else
	system::Chmod(subPath, 0640);  // clean up to make it usable again
#endif

	path = func();
	subPath = "./Tracer.any";
	system::ResolvePath(subPath);
	assertEqual(subPath, path.SubString(path.Length() - subPath.Length()));
}

void SystemFileTest::GetFilePathTest() {
	testGetFilePath(boost::bind(&coast::system::GetFilePath, "Tracer", "any"), "");	 // deprecated
	testGetFilePath(boost::bind(&coast::system::GetFilePath, "Tracer.any"), "");
	testGetFilePath(boost::bind(&coast::system::GetFilePathOrInput, "Tracer.any"), "Tracer.any");
}

void SystemFileTest::dirFileListTest() {
	StartTrace(SystemFileTest.dirFileListTest);
	Anything dir(system::DirFileList("."));

	assertEqual(0L, dir.GetSize());

	dir = system::DirFileList("config", "any");
	t_assert(dir.GetSize() > 0L);

	dir = system::DirFileList("..", "");
	t_assert(dir.GetSize() > 0L);
	TraceAny(dir, "entries of [..]");
}

//==============================================================================================
//        O P . I N P U T             Beginn
//==============================================================================================
void SystemFileTest::IStreamTest() {
	// istream& operator>>(istream& is, String& s)
	// String noch nicht belegt: Capacity gesetzt (groesser als die Dateilaenge) und Inhalt gesetzt.
	// String schon belegt:  Der Inhalt des Strings wird reset (Capacity NICHT) und neu gesetzt.
	//-------------------------------------------------------------------------------------------------

	String str0;
	std::istream *is0 = system::OpenStream("len5", "tst");
	if (is0 != 0) {
		*is0 >> str0;
		// PS? t_assert ( str0.Capacity() == 66 );
		t_assert(str0.Length() == 5);
		t_assert(str0.Capacity() >= str0.Length());
		t_assert((long)strlen((const char *)str0) == 5);
		t_assert(strcmp((const char *)str0, "01234") == 0);

		*is0 >> str0;
		// PS? t_assert ( str0.Capacity() == 66 );
		t_assert(str0.Length() == 0);
		t_assert(str0.Capacity() >= str0.Length());
		t_assert((long)strlen((const char *)str0) == 0);
		t_assert(strcmp((const char *)str0, "") == 0);

#if !defined(WIN32)
		// win32 istream hangs if read at file end is attempted! (FIXME?)
		*is0 >> str0;
		// PS? t_assert ( str0.Capacity() == 66 );
		t_assert(str0.Length() == 0);
		t_assert(str0.Capacity() >= str0.Length());
		t_assert((long)strlen((const char *)str0) == 0);
		t_assert(strcmp((const char *)str0, "") == 0);

		*is0 >> str0;
		// PS? t_assert ( str0.Capacity() == 66 );
		t_assert(str0.Length() == 0);
		t_assert(str0.Capacity() >= str0.Length());
		t_assert((long)strlen((const char *)str0) == 0);
		t_assert(strcmp((const char *)str0, "") == 0);
#endif
		delete is0;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}

	// Kombination von 2 ">>":  das zweite ">>" loescht das Resultat vom ersten ">>" und berechnet es neu.
	//---------------------------------------------------------------------------------------------------------
	String str1;
	std::istream *is1 = system::OpenStream("len5", "tst");
	if (is1 != 0) {
		*is1 >> str1;
		// PS? t_assert (str1.Capacity() == 66 );
		t_assert(str1.Length() == 5);
		t_assert(str1.Capacity() >= str1.Length());
		t_assert(strlen((const char *)str1) == 5);
		t_assert(memcmp((const char *)str1, "01234", (long)strlen((const char *)str1)) == 0);

		delete is1;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}
	std::istream *is2 = system::OpenStream("len5", "tst");
	if (is2 != 0) {
		*is2 >> str1;
		// PS? t_assert (str1.Capacity() == 66 );
		t_assert(str1.Length() == 5);
		t_assert(str1.Capacity() >= str1.Length());
		t_assert(strlen((const char *)str1) == 5);
		t_assert(memcmp((const char *)str1, "01234", strlen((const char *)str1)) == 0);
		// und nicht 0123401234.  Ist es gewuenscht ????
		delete is2;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}

	String str3 = "qwertzuiopasdfghjklyxcvbnm";
	std::istream *is3 = system::OpenStream("len5", "tst");
	if (is3 != 0) {
		*is3 >> str3;
		// PS? t_assert (str3.Capacity() == 54 );
		t_assert(str3.Length() == 5);
		t_assert(str3.Capacity() >= str3.Length());
		t_assert(memcmp((const char *)str3, "01234", strlen((const char *)str3)) == 0);
		t_assert(strlen((const char *)str3) == 5);
		t_assert(memcmp((const char *)str3, "01234", strlen((const char *)str3)) == 0);
		// und nicht qwe...bnm91234.  Ist es gewuenscht ????
		delete is3;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}
}
//==============================================================================================
//        O P . I N P U T             Ende
//==============================================================================================

//==============================================================================================
//        O P . O U T P U T             Beginn
//==============================================================================================
void SystemFileTest::OStreamTest() {
	//	ostream& operator<<(ostream& os, const String &s)
	//-------------------------------------------------------------
	String str0 = "", str00 = "";
	String sname("tmp/emptyStr");
	std::ostream *os0 = system::OpenOStream(sname, "tst");
	TString msg("couldn't open ");
	msg << system::GetFilePath(sname, "tst");

	t_assertm(os0 != 0, msg);
	if (os0 != 0) {
		*os0 << str0;
		delete os0;
	} else {
		assertEqual("'write to file tmp/emptyStr.tst'", "'could not write tmp/emptyStr.tst'");
	}

	std::istream *is0 = system::OpenStream("tmp/emptyStr", "tst");

	if (is0 != 0) {
		*is0 >> str00;
		delete is0;
		t_assert(str0 == str00);
		assertCompare(str0.Length(), equal_to, str00.Length());
		// t_assert( str0.Capacity() == str00.Capacity() );
		t_assert(str0.Capacity() >= str0.Length());
		t_assert(str00.Capacity() >= str00.Length());
		t_assert(memcmp((const char *)str0, (const char *)str00, str0.Length()) == 0);
	} else {
		assertEqual("'read file tmp/emptyStr.tst'", "'could not read tmp/emptyStr.tst'");
	}

	String str1, str11;
	str1.Append("0123456789");
	std::ostream *os1 = system::OpenOStream("tmp/zahlen", "tst");

	if (os1 != 0) {
		*os1 << str1;
		delete os1;
	} else {
		assertEqual("'write to file tmp/zahlen.tst'", "'could not write tmp/zahlen.tst'");
	}
	std::istream *is1 = system::OpenStream("tmp/zahlen", "tst");

	if (is1 != 0) {
		*is1 >> str11;
		delete is1;
		t_assert(str1 == str11);
		assertCompare(str1.Length(), equal_to, str11.Length());
		// t_assert( str1.Capacity() == str11.Capacity() );
		t_assert(str1.Capacity() >= str1.Length());
		t_assert(str11.Capacity() >= str11.Length());
		t_assert(memcmp((const char *)str1, (const char *)str11, str1.Length()) == 0);
	} else {
		assertEqual("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	String str2, str22;
	str2.Append("qwertzuiopasdfghjklyxcvbnm");
	std::ostream *os2 = system::OpenOStream("tmp/buchst", "tst");

	if (os2 != 0) {
		*os2 << str2;
		delete os2;
	} else {
		assertEqual("'write to file tmp/buchst.tst'", "'could not write tmp/buchst.tst'");
	}
	std::istream *is2 = system::OpenStream("tmp/buchst", "tst");

	if (is2 != 0) {
		*is2 >> str22;
		delete is2;
		t_assert(str2 == str22);
		assertCompare(str2.Length(), equal_to, str22.Length());
		// t_assert( str2.Capacity() == str22.Capacity() );
		t_assert(str2.Capacity() >= str2.Length());
		t_assert(str22.Capacity() >= str22.Length());
		t_assert(memcmp((const char *)str2, (const char *)str22, str2.Length()) == 0);
	} else {
		assertEqual("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}

	//	unsafe_ostream& operator<<(unsafe_ostream& os, const String &s)
	//-----------------------------------------------------------------
	String str0u = "", str00u;
	std::ostream *os0u = system::OpenOStream("tmp/emptyStrU", "tst");

	if (os0u != 0) {
		*os0u << str0u;
		delete os0u;
	} else {
		assertEqual("'write to file tmp/emptyStrU.tst'", "'could not write tmp/emptyStrU.tst'");
	}

	std::istream *is0u = system::OpenStream("tmp/emptyStrU", "tst");

	if (is0u != 0) {
		*is0u >> str00u;
		delete is0u;
		t_assert(str0u == str00u);
		assertCompare(str0u.Length(), equal_to, str00u.Length());
		// t_assert( str0u.Capacity() == str00u.Capacity() );
		t_assert(str0u.Capacity() >= str0u.Length());
		t_assert(str00u.Capacity() >= str00u.Length());
		t_assert(memcmp((const char *)str0u, (const char *)str00u, str0u.Length()) == 0);
	} else {
		assertEqual("'read file tmp/emptyStrU.tst'", "'could not read tmp/emptyStrU.tst'");
	}

	String str1u;
	str1u.Append("0123456789");
	std::ostream *os1u = system::OpenOStream("tmp/zahlenU", "tst");

	if (os1u != 0) {
		*os1u << str1u;
		delete os1u;
	} else {
		assertEqual("'write to file tmp/zahlenU.tst'", "'could not write tmp/zahlenU.tst'");
	}
	std::istream *is1u = system::OpenStream("tmp/zahlenU", "tst");

	if (is1u != 0) {
		String str11u;
		*is1u >> str11u;
		delete is1u;
		t_assert(str1u == str11u);
		assertCompare(str1u.Length(), equal_to, str11u.Length());
		// t_assert( str1u.Capacity() == str11u.Capacity() );
		t_assert(str1u.Capacity() >= str1u.Length());
		t_assert(str11u.Capacity() >= str11u.Length());
		t_assert(memcmp((const char *)str1u, (const char *)str11u, str1u.Length()) == 0);
	} else {
		assertEqual("'read file tmp/zahlenU.tst'", "'could not read tmp/zahlenU.tst'");
	}

	String str2u;
	str2u.Append("qwertzuiopasdfghjklyxcvbnm");
	std::ostream *os2u = system::OpenOStream("tmp/buchstU", "tst");

	if (os2u != 0) {
		*os2u << str2u;
		delete os2u;
	} else {
		assertEqual("'write to file tmp/buchstU.tst'", "'could not write tmp/buchstU.tst'");
	}

	std::istream *is2u = system::OpenStream("tmp/buchstU", "tst");

	if (is2u != 0) {
		String str22u;
		*is2u >> str22u;
		delete is2u;
		t_assert(str2u == str22u);
		assertCompare(str2u.Length(), equal_to, str22u.Length());
		// t_assert( str2u.Capacity() == str22u.Capacity() );
		t_assert(str2u.Capacity() >= str2u.Length());
		t_assert(str22u.Capacity() >= str22u.Length());
		t_assert(memcmp((const char *)str2u, (const char *)str22u, str2u.Length()) == 0);
	} else {
		assertEqual("'read file tmp/buchstU.tst'", "'could not read tmp/buchstU.tst'");
	}
}
//==============================================================================================
//        O P . O U T P U T             Ende
//==============================================================================================

void SystemFileTest::TimeTest() {
	time_t now = ::time(0);

	struct tm agmtime;
	agmtime.tm_year = 0;
	system::GmTime(&now, &agmtime);
	t_assert(agmtime.tm_year > 0);

	struct tm alocaltime;
	alocaltime.tm_year = 0;
	system::LocalTime(&now, &alocaltime);
	t_assert(alocaltime.tm_year > 0);

	assertEqual(agmtime.tm_sec, alocaltime.tm_sec);
	assertEqual(agmtime.tm_min, alocaltime.tm_min);
}

//==============================================================================================
//        O P . I N O U T P U T S             Beginn
//==============================================================================================
void SystemFileTest::IOStreamTest() {
	// Interaktion von >> und <<  :  Mit dem Operator "<<" wird der Inhalt gespeichert und mit ">>" wird der Inhalt gelesen.
	// Die Capacity nicht!!
	//----------------------------------------------------------------------------------------------------------------------

	String str0;
	std::istream *is0 = system::OpenStream("tmp/zahlen", "tst");

	if (is0 != 0) {
		*is0 >> str0;
		t_assert(str0.Capacity() >= str0.Length());
		t_assert((long)strlen((const char *)str0) == str0.Length());
		t_assert((long)strlen("0123456789") == str0.Length());
		t_assert(memcmp((const char *)str0, "0123456789", strlen((const char *)str0)) == 0);
		delete is0;
	} else {
		assertEqual("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	String str1;
	std::istream *is1 = system::OpenStream("tmp/buchst", "tst");

	if (is1 != 0) {
		*is1 >> str1;
		t_assert(str1.Capacity() >= str1.Length());
		t_assert(str1.Length() == (long)strlen((const char *)str1));
		t_assert(str1.Length() == (long)strlen("qwertzuiopasdfghjklyxcvbnm"));
		t_assert(memcmp((const char *)str1, "qwertzuiopasdfghjklyxcvbnm", strlen((const char *)str1)) == 0);
		delete is1;
	} else {
		assertEqual("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}

	// Konkatenation von 2 "<<"
	// Das zweite "<<" loescht das Resultat des ersten "<<" ( ???? ist das gewuenscht ???? )
	//-------------------------------------------------------------------------------------------
	String str2 = "qwertzuiopasdfghjklyxcvbnm";
	std::ostream *os2 = system::OpenOStream("tmp/dummy", "tst", std::ios::trunc);

	if (os2 != 0) {
		*os2 << str2;
		t_assert(str2.Capacity() >= str2.Length());
		t_assert((long)strlen((const char *)str2) == str2.Length());
		t_assert((long)strlen("qwertzuiopasdfghjklyxcvbnm") == str2.Length());
		t_assert(memcmp((const char *)str2, "qwertzuiopasdfghjklyxcvbnm", strlen((const char *)str2)) == 0);
		delete os2;
	} else {
		assertEqual("'write to file dummy.tst'", "'could not write dummy.tst'");
	}
	String str3 = "0123456789";
	std::ostream *os3 = system::OpenOStream("tmp/dummy", "tst", std::ios::app);

	if (os3 != 0) {
		*os3 << str3;
		t_assert(str3.Capacity() >= str3.Length());
		t_assert(str3.Length() == (long)strlen("0123456789"));
		t_assert((long)strlen((const char *)str3) == str3.Length());
		t_assert(memcmp((const char *)str3, "0123456789", strlen((const char *)str3)) == 0);
		delete os3;
	} else {
		assertEqual("'write to file dummy.tst'", "'could not write dummy.tst'");
	}
	std::istream *isHlp = system::OpenStream("tmp/dummy", "tst");

	if (isHlp != 0) {
		String strHlp;
		String compare;
		compare << str2 << str3;
		*isHlp >> strHlp;
		delete isHlp;
		assertEqual(compare, strHlp);
		assertEqual(compare.Length(), strHlp.Length());
	} else {
		assertEqual("'read file dummy.tst'", "'could not read dummy.tst'");
	}

	// Mehrfaches "<<"-Operator
	// Bei jedem Aufruf wird die Datei tmp/strChain.tst geloescht und neu geschrieben
	String str4 = "StringA", str5 = "StringB";
	std::ostream *os4 = system::OpenOStream("tmp/strChain", "tst");

	if (os4 != 0) {
		// You must check if the file is OK   ( ???? )
		*os4 << str4 << std::endl
			 << str5 << 'a' << 'b' << 123456789L << ' ' << -123456789L << ' ' << true << ' ' << false << " qqq"
			 << " " << 123 << " " << -123.234 << " " << 0.2345e-7 << ' ' << 1.123456789e9;
		delete os4;
	} else {
		assertEqual("'write to file tmp/strChain.tst'", "'could not write tmp/strChain.tst'");
	}
	// Mehrfaches ">>"-Operator
	// Bei jedem Aufruf wird die Datei tmp/strChain.tst geloescht und neu geschrieben
	String str6;
	std::istream *is2 = system::OpenStream("tmp/buchst", "tst");
	std::istream *is3 = system::OpenStream("tmp/zahlen", "tst");

	if ((is2 != 0) && (is3 != 0)) {
		// Resultat ist wie erwartet oder falsch  ????
		// str6 << *is2;  Compilierbar aber falsches Resultat
		*is2 >> str6;
		assertEqual("qwertzuiopasdfghjklyxcvbnm", str6);
		t_assert(str6.Length() == (long)strlen("qwertzuiopasdfghjklyxcvbnm"));
		t_assert(str6.Capacity() >= str6.Length());
		*is3 >> str6;
		assertEqual("0123456789", str6);
		t_assert(str6.Length() == (long)strlen("0123456789"));
		t_assert(str6.Capacity() >= str6.Length());
		delete is2;
		delete is3;
	} else {
		if (is2 == 0) {
			assertEqual("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
		}
		if (is3 == 0) {
			assertEqual("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
		}
	}

	// Einfluss von (char)0:  Length() = 5 ABER strlen = 2
	String str7;
	char bufHlp[5] = {'a', 'b', (char)0, 'c', 'd'};
	str7.Append((void *)bufHlp, 5);
	std::ostream *os5 = system::OpenOStream("tmp/strMit0", "tst");

	if (os5 != 0) {
		*os5 << str7;
		t_assert(str7.Capacity() >= str7.Length());
		t_assert(str7.Length() == 5);
		t_assert(strlen((const char *)str7) == 2);
		t_assert(memcmp((const char *)str7, "ab", strlen((const char *)str7)) == 0);
		t_assert(memcmp((const char *)str7, bufHlp, str7.Length()) == 0);
		delete os5;
	} else {
		assertEqual("'write to file strMit0.tst'", "'could not write strMit0.tst'");
	}
	String str8;
	std::istream *is4 = system::OpenStream("tmp/strMit0", "tst");

	if (is4 != 0) {
		*is4 >> str8;
		t_assert(str8.Capacity() >= str8.Length());
		t_assert(str8.Length() == 5);
		t_assert(strlen((const char *)str8) == 2);
		t_assert(memcmp((const char *)str8, "ab", strlen((const char *)str8)) == 0);
		t_assert(memcmp((const char *)str8, bufHlp, str8.Length()) == 0);
		delete is4;
	} else {
		assertEqual("'read file strMit0.tst'", "'could not read strMit0.tst'");
	}

	// Test automatic creation of files for std::ios::app and std::ios::ate
	{
		StartTrace(SystemFileTest.IOStreamTest);
		// precondition: files should not exist already!!
		String strAppFile("tmp/ios_app.tst"), strAteFile("tmp/ios_ate.tst");
		if (system::IsRegularFile(strAppFile)) {
			system::io::unlink(strAppFile);
		}
		if (system::IsRegularFile(strAteFile)) {
			system::io::unlink(strAteFile);
		}
		String strOut("FirstEntryInAppFile"), strOutApp("-AppendedContent"), strReadIn;
		std::iostream *pStream = system::OpenStream(strAppFile, NULL, std::ios::app);
		if (t_assertm(pStream != NULL, "expected file to be created")) {
			*pStream << strOut;
			delete pStream;
		}
		pStream = system::OpenStream(strAppFile, NULL, std::ios::app);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream << strOutApp;
			delete pStream;
		}
		pStream = system::OpenIStream(strAppFile, NULL);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			assertCharPtrEqual(strExpected, strReadIn);
		}
		pStream = system::OpenStream(strAppFile, NULL, std::ios::app);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			// can position in file but content gets still appended at the end
			pStream->seekp(strOut.Length());
			//??			assertEqual(strOut.Length(), (long)pStream->tellp()); // foo: should this one work on append-only
			// streams?
			*pStream << strOut;
			delete pStream;
		}
		pStream = system::OpenIStream(strAppFile, NULL);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			strExpected << strOut;
			assertCharPtrEqual(strExpected, strReadIn);
		}
		pStream = system::OpenStream(strAteFile, NULL, std::ios::ate);
		if (!t_assertm(pStream == NULL, "expected file not to be opened")) {
			delete pStream;
		}
		Trace("before first app");
		pStream = system::OpenStream(strAteFile, NULL, std::ios::app);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream << strOut;
			delete pStream;
		}
		Trace("testing appended content");
		pStream = system::OpenIStream(strAteFile, NULL);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream >> strReadIn;
			delete pStream;
			assertCharPtrEqual(strOut, strReadIn);
		}
		Trace("before second ate");
		pStream = system::OpenStream(strAteFile, NULL, std::ios::ate);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream << strOutApp;
			delete pStream;
		}
		Trace("testing ate");
		pStream = system::OpenIStream(strAteFile, NULL);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			assertCharPtrEqual(strExpected, strReadIn);
		}
		Trace("before third ate");
		pStream = system::OpenStream(strAteFile, NULL, std::ios::ate);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			// can position in file, contents get appended beginning from this location then
			pStream->seekp(strOut.Length());
			assertEqual(strOut.Length(), (long)pStream->tellp());
			*pStream << strOut;
			delete pStream;
		}
		Trace("testing ate");
		pStream = system::OpenIStream(strAteFile, NULL);
		if (t_assertm(pStream != NULL, "expected file to be opened")) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOut;
			assertCharPtrEqual(strExpected, strReadIn);
		}
	}
}

//==============================================================================================
//        O P . I N O U T P U T S             Ende
//==============================================================================================

void SystemFileTest::LoadConfigFileTest() {
	Anything dbgany;
	t_assert(system::LoadConfigFile(dbgany, "Tracer"));	 // any extension automatic
	t_assert(!dbgany.IsNull());
	t_assert(dbgany.IsDefined("LowerBound"));

	Anything dbg2;
	String realfilename;
	t_assert(system::LoadConfigFile(dbg2, "Tracer", "any", realfilename));
	assertEqual("Tracer.any", realfilename.SubString(realfilename.StrRChr(system::Sep()) + 1L));
	assertAnyEqual(dbgany, dbg2);

	Anything dbg3;
	t_assert(!system::LoadConfigFile(dbg3, "NotExisting", "any", realfilename));
	assertEqual("", realfilename);
	t_assert(dbg3.IsNull());
}

void SystemFileTest::MkRmDirTest() {
	StartTrace(SystemFileTest.MkRmDirTest);
	String strTmpDir = GetConfig()["TmpDir"].AsString("/tmp");
	String str1LevelRel(name());
	String str2LevelRel(str1LevelRel);
	str2LevelRel.Append(system::Sep()).Append("Level2");
	String str1Level(strTmpDir), str2Level(strTmpDir);
	str1Level << system::Sep() << str1LevelRel;
	str2Level << system::Sep() << str2LevelRel;
	Trace("strTmpDir [" << strTmpDir << "]");
	Trace("str1LevelRel [" << str1LevelRel << "]");
	Trace("str2LevelRel [" << str2LevelRel << "]");
	Trace("str1Level [" << str1Level << "]");
	Trace("str2Level [" << str2Level << "]");
	// assume that we have a tmp-directory to access and to play with
	if (t_assertm(system::IsDirectory(strTmpDir), "expected an accessible directory")) {
		t_assertm(system::io::mkdir(strTmpDir, 0755) == -1, "expected creation of directory to fail");
		if (t_assertm(system::io::mkdir(str1Level, 0755) == 0, "expected creation of directory to succeed")) {
			t_assertm(system::IsDirectory(str1Level), "expected an accessible directory");
			t_assertm(system::io::mkdir(str1Level, 0755) == -1, "expected creation of existing directory to fail");
			t_assertm(system::io::rmdir(str1Level) == 0, "expected deletion of directory to succeed");
			t_assertm(system::io::rmdir(str1Level) == -1, "expected deletion of nonexisting directory to fail");
		}
		t_assertm(system::io::mkdir(str2Level, 0755) == -1, "expected creation of multiple directory levels at once to fail");
		// relative tests
		String wd;
		system::GetCWD(wd);
		if (t_assert(system::ChangeDir(strTmpDir))) {
			t_assertm(system::io::mkdir(str1LevelRel, 0755) == 0, "expected creation of relative directory to succeed");
			t_assertm(system::io::mkdir(str2LevelRel, 0755) == 0,
					  "expected creation of two level relative directory to succeed");
			t_assertm(system::io::rmdir(str1LevelRel) == -1, "expected deletion of non-empty relative directory to fail");
			t_assertm(system::io::rmdir(str2LevelRel) == 0, "expected deletion of relative directory to succeed");
			t_assertm(system::io::rmdir(str1LevelRel) == 0, "expected deletion of relative directory to succeed");
		}
		system::ChangeDir(wd);
	}
}

void SystemFileTest::MakeRemoveDirectoryTest() {
	StartTrace(SystemFileTest.MakeRemoveDirectoryTest);
	String strTmpDir = GetConfig()["TmpDir"].AsString("/tmp");
	String str1LevelRel(name());
	String str2LevelRel(str1LevelRel);
	str2LevelRel.Append(system::Sep()).Append("Level2");
	String str1Level(strTmpDir), str2Level(strTmpDir);
	str1Level << system::Sep() << str1LevelRel;
	str2Level << system::Sep() << str2LevelRel;
	Trace("strTmpDir [" << strTmpDir << "]");
	Trace("str1LevelRel [" << str1LevelRel << "]");
	Trace("str2LevelRel [" << str2LevelRel << "]");
	Trace("str1Level [" << str1Level << "]");
	Trace("str2Level [" << str2Level << "]");
	// assume that we have a tmp-directory to access and to play with
	if (t_assertm(system::IsDirectory(strTmpDir), "expected an accessible directory")) {
		String strSaveParam(strTmpDir);
		// one level tests
		assertComparem(system::eExists, equal_to, system::MakeDirectory(strTmpDir, 0755, false),
					   "expected creation of directory to fail");
		system::ResolvePath(strSaveParam);
		assertCharPtrEqual(strSaveParam, strTmpDir);
		strSaveParam = str1Level;
		if (assertComparem(system::eSuccess, equal_to, system::MakeDirectory(str1Level, 0755, false),
						   "expected creation of directory to succeed")) {
			system::ResolvePath(strSaveParam);
			assertCharPtrEqual(strSaveParam, str1Level);
			t_assertm(system::IsDirectory(str1Level), "expected an accessible directory");
			assertComparem(system::eExists, equal_to, system::MakeDirectory(str1Level, 0755, false),
						   "expected creation of existing directory to fail");
			system::ResolvePath(strSaveParam);
			assertCharPtrEqual(strSaveParam, str1Level);
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(str1Level, false),
						   "expected deletion of directory to succeed");
			t_assertm(!system::IsDirectory(str1Level), "expected directory to be deleted");
		}
		// multiple level tests
		strSaveParam = str2Level;
		assertComparem(system::eNotExists, equal_to, system::MakeDirectory(str2Level, 0755, false),
					   "expected creation of multiple directory levels at once to fail");
		assertCharPtrEqual(str1Level, str2Level);
		str2Level = strSaveParam;
		if (assertComparem(system::eSuccess, equal_to, system::MakeDirectory(str2Level, 0755, true),
						   "expected creation of multiple directory levels at once to succeed")) {
			system::ResolvePath(strSaveParam);
			assertCharPtrEqual(strSaveParam, str2Level);
			t_assertm(system::IsDirectory(str2Level), "expected an accessible directory tree");
			assertComparem(system::eRecurseDeleteNotAllowed, equal_to, system::RemoveDirectory(str2Level, true),
						   "expected deletion of multiple absolute dir levels to fail");
			assertComparem(system::eExists, equal_to, system::RemoveDirectory(str1Level, false),
						   TString("expected deletion of parent dir [") << str1Level << "] to fail");
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(str2Level, false),
						   "expected deletion of one absolute dir level to succeed");
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(str1Level, false),
						   "expected deletion of one absolute dir level to succeed");
			t_assertm(!system::IsDirectory(str2Level), "expected directory to be deleted");
			t_assertm(!system::IsDirectory(str1Level), "expected directory to be deleted");
		}
		// relative tests
		String wd;
		system::GetCWD(wd);
		if (t_assert(system::ChangeDir(strTmpDir))) {
			// one level tests
			Trace("str1LevelRel [" << str1LevelRel << "] str2LevelRel [" << str2LevelRel << "]");
			assertComparem(system::eSuccess, equal_to, system::MakeDirectory(str1LevelRel, 0755, false),
						   "expected creation of relative directory to succeed");
			assertComparem(system::eSuccess, equal_to, system::MakeDirectory(str2LevelRel, 0755, false),
						   "expected creation of second level relative directory to succeed");
			Trace("str1LevelRel [" << str1LevelRel << "] str2LevelRel [" << str2LevelRel << "]");
			assertComparem(system::eExists, equal_to, system::RemoveDirectory(str1LevelRel, false),
						   "expected deletion of parent relative directory to fail");
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(str2LevelRel, false),
						   "expected deletion of relative directory to succeed");
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(str1LevelRel, false),
						   "expected deletion of relative directory to succeed");
			// multiple level tests
			assertComparem(system::eSuccess, equal_to, system::MakeDirectory(str2LevelRel, 0755, true),
						   "expected creation of multiple level relative directories to succeed");
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(str2LevelRel, true),
						   "expected deletion of multiple relative directories to succeed");
		}
		system::ChangeDir(wd);
	}
}

void SystemFileTest::MakeDirectoryTest() {
	StartTrace(SystemFileTest.MakeDirectoryTest);
	String strStartDir = GetTestCaseConfig()["BasePath"].AsString("/tmp");
	long lNumDirsMax = GetTestCaseConfig()["MaxNumDirs"].AsLong(20L), lIdx = 0L;
	Trace("BasePath [" << strStartDir << "], MaxDirs: " << lNumDirsMax);
	String strDirToCreate;

	// assume that we have a tmp-directory to access and to play with
	if (!system::IsDirectory(strStartDir)) {
		assertComparem(system::eSuccess, equal_to, system::MakeDirectory(strStartDir, 0755, true),
					   "expected creation of directory to succeed");
	}
	if (t_assertm(system::IsDirectory(strStartDir), "expected start directory to be valid")) {
		for (; lIdx < lNumDirsMax; ++lIdx) {
			strDirToCreate.Trim(0L);
			strDirToCreate.Append(strStartDir).Append(system::Sep()).Append(lIdx);
			if (system::MakeDirectory(strDirToCreate, 0755, false) != system::eSuccess) {
				SYSERROR("failed at index: " << lIdx);
				break;
			}
		}
		assertComparem(lIdx, equal_to, lNumDirsMax, "expected given number of directories to be created");
		SYSINFO("last directory created [" << strDirToCreate << "] Idx: " << lIdx);
		while (--lIdx >= 0L) {
			strDirToCreate.Trim(0L);
			strDirToCreate.Append(strStartDir).Append(system::Sep()).Append(lIdx);
			system::RemoveDirectory(strDirToCreate, false);
		}
	}
	assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strStartDir, false),
				   "expected deletion of directory to succeed");
}

void SystemFileTest::MakeDirectoryExtendTest() {
	StartTrace(SystemFileTest.MakeDirectoryExtendTest);

	String strBaseDir(GetTestCaseConfig()["BaseDir"].AsString());
	const char pcFillerPrefix[] = "dummydir_";
	bool bCreatedDirs(false);
	if (strBaseDir.Length() > 0L) {
		long lIdx(1L);
		if (!system::IsDirectory(strBaseDir)) {
			system::MakeDirectory(strBaseDir, 0755, true, false);
		}
		String strFillerDir(strBaseDir);
		strFillerDir.Append(system::Sep()).Append(pcFillerPrefix);
		long lTrimLen(strFillerDir.Length());
		strFillerDir.Append(lIdx);
		while (system::MakeDirectory(strFillerDir, 0755, true, false) == system::eSuccess) {
			strFillerDir.Trim(lTrimLen);
			strFillerDir.Append(++lIdx);
			if ((lIdx % 1000) == 0) {
				SystemLog::WriteToStdout(String("directory [").Append(strFillerDir).Append("] created...\n"));
			}
			bCreatedDirs = true;
		}
		--lIdx;
		// add two directories to the number of created dirs: '.' and '..'
		long iNumLinks = lIdx + 2;
		Trace("number of dirs created: " << lIdx << ", num of hardlinks: " << iNumLinks);

		AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig()["Tests"]);
		ROAnything roaConfig;
		while (lIdx > 0L && aIterator(roaConfig)) {
			String strEnsureDir(roaConfig["EnsureDirExists"].AsString());
			String strExpectBaseDir(roaConfig["ExpectedBaseDir"].AsString());
			String strCreateDirRel(roaConfig["PathToCreate"].AsString());
			String strLinkName(strCreateDirRel);
			String strCreateDir(strBaseDir), strExpectDir(strExpectBaseDir);
			strCreateDir.Append(system::Sep()).Append(strCreateDirRel);
			strExpectDir.Append(system::Sep()).Append(strCreateDirRel);
			long lSep = strLinkName.StrChr(system::Sep());
			if (lSep > 0L) {
				strLinkName.Trim(lSep);
			}
			Trace("Dir to create [" << strCreateDir << "] ExpectedDir [" << strExpectDir << "] first seg [" << strLinkName
									<< "]");
			Trace("first seg [" << strLinkName << "] of rel path [" << strCreateDirRel << "]");
			if (strCreateDirRel.Length() > 0L && strCreateDir.Length() > 0L && strExpectDir.Length() > 0L) {
				bool bDidCreateDir(false);
				if (strEnsureDir.Length() != 0) {
					system::DirStatusCode aCode(system::MakeDirectory(strEnsureDir, 0755, true, false));
					bDidCreateDir = (aCode == system::eSuccess);
				}
				if (!system::IsDirectory(strCreateDir)) {
					String strTmpDir(strCreateDir);
					// test should fail without extend link option
					if (assertComparem(system::eNoMoreHardlinks, equal_to, system::MakeDirectory(strTmpDir, 0755, true, false),
									   TString("expected creation of directory to fail due to no more available hardlinks, is "
											   "the test-directory [")
										   << strTmpDir << "] full of subdirs?")) {
						strTmpDir.Trim(strTmpDir.StrRChr(system::Sep()));
						Trace("exhausted directory [" << strTmpDir << "]");
						assertCompare(iNumLinks, equal_to, (long)system::GetNumberOfHardLinks(strTmpDir));
						strTmpDir = strCreateDir;
						if (assertComparem(system::eSuccess, equal_to, system::MakeDirectory(strTmpDir, 0755, true, true),
										   "expected creation of extended directory to succeed")) {
							String wd;
							system::GetCWD(wd);
							t_assertm(system::IsDirectory(strExpectDir), "expected extension directory to be created");
							if ((strLinkName.Length() != 0) && t_assert(system::ChangeDir(strBaseDir))) {
								t_assertm(system::IsSymbolicLink(strLinkName), "expected directory (link) to be created");
								assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strLinkName),
											   "expected removal of symbolic link to succeed");
							}
							if (t_assert(system::ChangeDir(strExpectBaseDir))) {
								assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strCreateDirRel, true),
											   "expected removal of directory to succeed");
							}
							system::ChangeDir(wd);
						}
					}
				}
				if (bDidCreateDir && system::IsDirectory(strEnsureDir)) {
					assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strEnsureDir),
								   "failed to remove directory we created for testing");
				}
			}
		}
		if (bCreatedDirs) {
			Trace("deleting created directories");
			strFillerDir.Trim(lTrimLen);
			strFillerDir.Append(lIdx);
			while (system::IsDirectory(strFillerDir) && (system::RemoveDirectory(strFillerDir) != 0U)) {
				strFillerDir.Trim(lTrimLen);
				strFillerDir.Append(--lIdx);
				if ((lIdx % 1000) == 0) {
					SystemLog::WriteToStdout(String("directory [").Append(strFillerDir).Append("] deleted...\n"));
				}
			}
		}
	}
}

#if !defined(WIN32)
void SystemFileTest::SymbolicLinkTest() {
	StartTrace(SystemFileTest.SymbolicLinkTest);

	AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig());
	ROAnything roaConfig;
	while (aIterator(roaConfig)) {
		String strTmpDir = roaConfig["TmpDir"].AsString("/tmp");
		String strRelDir = roaConfig["PathToCreate"].AsString();
		String strCreateDir(strTmpDir);
		strCreateDir.Append(system::Sep()).Append(strRelDir);
		String strLinkRel = roaConfig["Link"].AsString(), strLinkAbs(strTmpDir);
		strLinkAbs.Append(system::Sep()).Append(roaConfig["Link"].AsString());
		Trace("Dir to create [" << strCreateDir << "] Link [" << strLinkRel << "]");

		if (strCreateDir.Length() > 0L) {
			// assume that we have a tmp-directory to access and to play with
			if (!system::IsDirectory(strCreateDir)) {
				assertComparem(system::eSuccess, equal_to, system::MakeDirectory(strCreateDir, 0755, false),
							   "expected creation of relative directory to succeed");
			}
			if (t_assert(system::IsDirectory(strCreateDir))) {
				String wd;
				system::GetCWD(wd);
				if (t_assert(system::ChangeDir(strTmpDir))) {
					Trace("creating relative link [" << strLinkRel << "] to relative dir [" << strRelDir << "]");
					assertComparem(system::eSuccess, equal_to, system::CreateSymbolicLink(strRelDir, strLinkRel),
								   "expected creation of relative symbolic link to succeed");
					t_assertm(system::IsSymbolicLink(strLinkRel), "expected link to be valid");
					assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strLinkRel),
								   "expected removal of relative symbolic link to succeed");
					t_assert(system::IsDirectory(strRelDir));
					if (t_assert(!system::IsDirectory(strLinkRel))) {
						Trace("creating relative link [" << strLinkRel << "] to absolute dir [" << strCreateDir << "]");
						assertComparem(system::eSuccess, equal_to, system::CreateSymbolicLink(strCreateDir, strLinkRel),
									   "expected creation of relative symbolic link to succeed");
						t_assertm(system::IsSymbolicLink(strLinkRel), "expected link to be valid");
						assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strLinkRel),
									   "expected removal of relative symbolic link to succeed");
						t_assert(system::IsDirectory(strCreateDir));
					}
				}
				system::ChangeDir(wd);
				Trace("creating absolute link [" << strLinkAbs << "] to dir [" << strCreateDir << "]");
				assertComparem(system::eSuccess, equal_to, system::CreateSymbolicLink(strCreateDir, strLinkAbs),
							   "expected creation of absolute symbolic link to succeed");
				t_assertm(system::IsSymbolicLink(strLinkAbs), "expected link to be valid");
				assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strLinkAbs),
							   "expected removal of absolute symbolic link to succeed");
				t_assert(system::IsDirectory(strCreateDir));
			}
			assertComparem(system::eSuccess, equal_to, system::RemoveDirectory(strCreateDir),
						   "expected removal of directory to succeed");
		}
	}
}
#endif

void SystemFileTest::GetFileSizeTest() {
	StartTrace(SystemFileTest.GetFileSizeTest);

	AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig());
	ROAnything roaConfig;
	while (aIterator(roaConfig)) {
		String path(system::GetFilePath(roaConfig["File"].AsString()));
		if (t_assertm(path.Length() > 0, "expected file path to be valid")) {
			ul_long ulSize = 0;
			if (t_assert(system::GetFileSize(path, ulSize))) {
				assertEqualm(roaConfig["ExpectedSize"].AsLong(), ulSize, "expected same size");
			}
		}
	}
}

void SystemFileTest::BlocksLeftOnFSTest() {
	StartTrace(SystemFileTest.BlocksLeftOnFSTest);
	// can only test that we have still some space left, nothing more for now
	ul_long ulBlocks = 0;
	unsigned long ulBlockSize = 0;
	String fsPath(GetConfig()["BlocksLeftOnFSTest"]["FS"].AsString("/"));
	if (t_assertm(system::BlocksLeftOnFS(fsPath, ulBlocks, ulBlockSize), "expected function call to succeed")) {
		t_assertm(ulBlocks > 0, "expected some blocks left on device");
		t_assertm(ulBlockSize > 0, "expected block size not to be 0");
		Trace("blocks: " << (l_long)ulBlocks << " blocksize: " << (long)ulBlockSize);
	}
}

Test *SystemFileTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SystemFileTest, initPathTest);
	ADD_CASE(testSuite, SystemFileTest, pathListTest);
	ADD_CASE(testSuite, SystemFileTest, rooDirTest);
	ADD_CASE(testSuite, SystemFileTest, IsAbsolutePathTest);
	ADD_CASE(testSuite, SystemFileTest, ResolvePathTest);
	ADD_CASE(testSuite, SystemFileTest, OpenStreamTest);
	ADD_CASE(testSuite, SystemFileTest, OpenStreamWithSearchTest);
	ADD_CASE(testSuite, SystemFileTest, OpenOStreamTest);
	ADD_CASE(testSuite, SystemFileTest, OpenIStreamTest);
	ADD_CASE(testSuite, SystemFileTest, GetFilePathTest);
	ADD_CASE(testSuite, SystemFileTest, dirFileListTest);
	ADD_CASE(testSuite, SystemFileTest, IStreamTest);
	ADD_CASE(testSuite, SystemFileTest, OStreamTest);
	ADD_CASE(testSuite, SystemFileTest, IOStreamTest);
	ADD_CASE(testSuite, SystemFileTest, CWDTests);
	ADD_CASE(testSuite, SystemFileTest, LoadConfigFileTest);
	ADD_CASE(testSuite, SystemFileTest, TimeTest);
	ADD_CASE(testSuite, SystemFileTest, MkRmDirTest);
	ADD_CASE(testSuite, SystemFileTest, MakeRemoveDirectoryTest);
	ADD_CASE(testSuite, SystemFileTest, MakeDirectoryTest);
#if !defined(WIN32)
	ADD_CASE(testSuite, SystemFileTest, SymbolicLinkTest);
#endif
	ADD_CASE(testSuite, SystemFileTest, MakeDirectoryExtendTest);
	ADD_CASE(testSuite, SystemFileTest, GetFileSizeTest);
	ADD_CASE(testSuite, SystemFileTest, BlocksLeftOnFSTest);
	ADD_CASE(testSuite, SystemFileTest, statTests);	 // needs to be last
	return testSuite;
}
