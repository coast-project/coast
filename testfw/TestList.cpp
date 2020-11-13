/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestList.h"

#include "Test.h"

//---- TestElt
TestElt::~TestElt() {
	delete fValue;

	fValue = 0;
}

//---- TestList ------------------------------------------------------
TestList::~TestList(void) {
	while (fList != 0) {
		TestElt *tmp = fList;
		fList = fList->fNext;
		delete tmp;
	}
}

void TestList::push_back(Test *aValue) {
	// create a new element of the list
	TestElt *tmp = new TestElt(aValue);
	// if the list already exists, bind the new element at the end of it
	if (fList != 0) {
		fIn->fNext = tmp;
	} else
	// if the list does not yet exist, the new element is the start point of the list
	{
		fList = tmp;
	}
	// keep in mind the address of the last inserted element
	fIn = tmp;
	// increment the number of elements in the list
	fSize += 1;
}

Test *TestList::first() {
	if (fList != 0) {
		// go the the first element
		fIt = fList;
		// return the result
		return (fIt->fValue);
	}
	return 0;
}

Test *TestList::next() {
	if (((fIt) != 0) && ((fIt->fNext) != 0)) {
		// go the the next element
		fIt = fIt->fNext;
		// return the result
		return (fIt->fValue);
	}
	// go the the first element
	fIt = fList;
	// return an empty result
	return 0;
}
