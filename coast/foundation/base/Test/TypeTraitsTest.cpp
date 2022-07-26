/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TypeTraitsTest.h"

#include "ITOTypeTraits.h"
#include "TestSuite.h"
#include "Tracer.h"

using namespace std;

void TypeTraitsTest::TraitsTest() {
	StartTrace(TypeTraitsTest.TraitsTest);
	{
		typedef String TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == false, "expected non-const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType),
				  "expected coast::typetraits::NullType");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String), "expected String-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false,
				  "expected non String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
	{
		typedef const String TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == true, "expected const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType),
				  "expected coast::typetraits::NullType");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String), "expected String-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false,
				  "expected non String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
	{
		typedef String *TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isPointer == true, "expected pointer type");
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == false, "expected non-const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(String), "expected String-type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String *),
				  "expected String-ptr-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == true,
				  "expected String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
	{
		typedef const String *TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isPointer == true, "expected pointer type");
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == false, "expected const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(String), "expected String-type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(const String *),
				  "expected String-ptr-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == true,
				  "expected String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
	{
		typedef String *const TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == true, "expected const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String *),
				  "expected String-ptr-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == true,
				  "expected String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
	{
		typedef String &TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == false, "expected non-const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType),
				  "expected coast::typetraits::NullType");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String &),
				  "expected String-ref-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false,
				  "expected non String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
	{
		typedef const String &TestType;
		t_assertm(coast::typetraits::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(coast::typetraits::TypeTraits<TestType>::isConst == true, "expected const type");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType),
				  "expected coast::typetraits::NullType");
		t_assertm(typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String &),
				  "expected String-ref-type");
		t_assertm(coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false,
				  "expected non String-ptr-type");
		t_assertm(typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
	}
}

template <typename InnerType>
class ClassTraits {
public:
	typedef typename coast::typetraits::fooTypeTraits<InnerType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename coast::typetraits::fooTypeTraits<InnerType>::PlainTypePtr PlainTypePtr;

	ClassTraits(ConstPlainTypeRef aParam) : fField(aParam) {
		StartTrace1(ClassTraits.ClassTraits, "ConstPlainTypeRef");
		SomeMethod(aParam);
	}
	ClassTraits(PlainTypePtr pParam) : fField(pParam) {
		StartTrace1(ClassTraits.ClassTraits, "PlainTypePtr");
		SomeMethod(pParam);
	}

	void SomeMethod(ConstPlainTypeRef aParam) { StartTrace1(ClassTraits.SomeMethod, "ConstPlainTypeRef"); }

	void SomeMethod(PlainTypePtr pParam) { StartTrace1(ClassTraits.SomeMethod, "PlainTypePtr"); }

	const type_info &GetTypeInfo() const { return typeid(fField); }

private:
	InnerType fField;
};

void TypeTraitsTest::ClassTraitsTest() {
	StartTrace(TypeTraitsTest.ClassTraitsTest);
	{
		typedef const String &TestType;
		String aTestString;
		ClassTraits<TestType> aClass(aTestString);
		t_assert(aClass.GetTypeInfo() == typeid(TestType));
	}
	{
		typedef String *TestType;
		String aTestString;
		ClassTraits<TestType> aClass(&aTestString);
		t_assert(aClass.GetTypeInfo() == typeid(TestType));
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *TypeTraitsTest::suite() {
	StartTrace(TypeTraitsTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, TypeTraitsTest, TraitsTest);
	ADD_CASE(testSuite, TypeTraitsTest, ClassTraitsTest);
	return testSuite;
}
