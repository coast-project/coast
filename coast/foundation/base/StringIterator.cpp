/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringIterator.h"

#include "ITOString.h"

#include <limits>

bool String_iterator::operator==(const String_iterator &r) const {
	return *base() == *(r.base()) && pos() == r.pos();
}

bool String_iterator::operator<(const String_iterator &r) const {
	if (*base() == *(r.base())) {
		return pos() < r.pos();
	}
	return false;
}

String_iterator::reference String_iterator::operator*() const {
	return a->operator[](pos());
}

String_iterator::reference String_iterator::operator[](difference_type index) const {
	return a->operator[](pos() + index);
}

String_iterator::difference_type String_iterator::operator-(const String_iterator &r) const {
	if (*base() == *(r.base())) {
		return pos() - r.pos();
	}
	return std::numeric_limits<String_iterator::difference_type>::max();
}

String_const_iterator::reference String_const_iterator::operator*() const {
	return a->operator[](pos());
}

String_const_iterator::reference String_const_iterator::operator[](difference_type index) const {
	return a->operator[](pos() + index);
}

bool String_const_iterator::operator==(const String_const_iterator &r) const {
	return *base() == *(r.base()) && pos() == r.pos();
}

bool String_const_iterator::operator<(const String_const_iterator &r) const {
	if (*base() == *(r.base())) {
		return pos() < r.pos();
	}
	return false;
}

String_const_iterator::difference_type String_const_iterator::operator-(const String_const_iterator &r) const {
	if (*base() == *(r.base())) {
		return pos() - r.pos();
	}
	return std::numeric_limits<String_const_iterator::difference_type>::max();
}
