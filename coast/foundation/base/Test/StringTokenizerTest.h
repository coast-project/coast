/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringTokenizerTest_H
#define _StringTokenizerTest_H

#ifndef _StringTokenizer_h_
#define _StringTokenizer_h_

#include "ITOString.h"
#include "TestCase.h"

class StringTokenizerTest : public testframework::TestCase {
protected:
	String fShort;
	String fLong;

public:
	StringTokenizerTest(TString tstrName);

	virtual void setUp();
	static Test *suite();
	void constrMethodsAll();
	void constrMethods0();
	void constrMethods1();
	void constrMethods2();
	void constrMethods3();
	void constrMethods4();
	void constrMethods5();
	void constrMethods6();
	void constrMethods7();
	void constrMethods8();
	void constrMethods9();
	void constrMethods10();
	void constrMethods11();
	void constrMethods12();
	void constructors();
	void nextTokenNormal();
	void nextTokenEmpty();
	void resetTest();
	void getRemainder();
};

#endif
#endif	// not defined _StringTokenizerTest_H
