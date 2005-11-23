/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -------------------------------------------------------
#include "AnyImpls.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "IFAObject.h"
#include "AnyVisitor.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <string.h>

static const String fgStrEmpty(Storage::Global()); //avoid temporary
static const Anything fgAnyEmpty(Storage::Global()); // avoid temporary

//---- AnyImpl --------------------------------------------------------------
void *AnyImpl::operator new(size_t size, Allocator *a)
{
	if (a) {
		return a->Calloc(1, size);
	} else {
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyImpl::operator delete(void *d, Allocator *a)
{
	if (d) {
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
	}
}
#endif

void AnyImpl::operator delete(void *d)
{
	if (d) {
		Allocator *a = ((AnyImpl *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyImpl::operator delete(d, a);
#else
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
#endif
	}
}

AnyImpl *AnyImpl::DeepClone(Allocator *a, Anything &xreftable)
{
	return this->DoDeepClone(a, xreftable);
}

//---- AnyLongImpl -----------------------------------------------------------------
AnyLongImpl::AnyLongImpl(long l, Allocator *a)
	: AnyImpl(a)
	, fLong(l)
	, fBuf(a)
{
	OStringStream out(fBuf);
	out << fLong;
}

String AnyLongImpl::AsString(const char *) const
{
	return fBuf;
}

const char *AnyLongImpl::AsCharPtr(const char *) const
{
	return (const char *)fBuf;
}

const char *AnyLongImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = fBuf.Length();
	return (const char *)fBuf;
}

void AnyLongImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	v.VisitLong(fLong, this, lIdx, slotname);
}

AnyImpl *AnyLongImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	AnyImpl *ret =  new ((a) ? a : Storage::Current()) AnyLongImpl(this->fLong, this->fBuf, a);
	return ret;
}

//---- AnyObjectImpl -----------------------------------------------------------------
static const char *gcObjectText = "IFAObject";

const char *AnyObjectImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = strlen(gcObjectText);
	return gcObjectText;
}

const char *AnyObjectImpl::AsCharPtr(const char *) const
{
	return gcObjectText;
}

String AnyObjectImpl::AsString(const char *) const
{
	return gcObjectText;
}

AnyImpl *AnyObjectImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyObjectImpl(this->fObject, a);
	return ret;
}

void AnyObjectImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	v.VisitObject(fObject, this, lIdx, slotname);
}

//---- AnyDoubleImpl -----------------------------------------------------------------
AnyDoubleImpl::AnyDoubleImpl(double d, Allocator *a)
	: AnyImpl(a)
	, fDouble(d)
	, fBuf(a)
{
	OStringStream out(fBuf);
	out.precision(20); // safety margin, 16 should be OK for doubles
	out << fDouble;
}

String AnyDoubleImpl::AsString(const char *) const
{
	return fBuf;
}

const char *AnyDoubleImpl::AsCharPtr(const char *dflt) const
{
	return (const char *)fBuf;
}

const char *AnyDoubleImpl::AsCharPtr(const char *dflt, long &buflen) const
{
	buflen = fBuf.Length();
	return (const char *)fBuf;
}

AnyImpl *AnyDoubleImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyDoubleImpl(this->fDouble, this->fBuf, a);
	return ret;
}

void AnyDoubleImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	v.VisitDouble(fDouble, this, lIdx, slotname);
}

//---- AnyBinaryBufImpl -----------------------------------------------------------------
const char *AnyBinaryBufImpl::AsCharPtr(const char *, long &buflen) const
{
	if (fBuf.Capacity() > 0) {
		buflen = fBuf.Length();
		return (const char *)fBuf;
	} else {
		buflen = 0;
		return 0;
	}
}

void AnyBinaryBufImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	v.VisitVoidBuf(fBuf, this, lIdx, slotname);
}

AnyImpl *AnyBinaryBufImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyBinaryBufImpl((void *)(const char *)this->fBuf, this->fBuf.Length(), a);
	return ret;
}

//---- AnyStringImpl -----------------------------------------------------------------
long AnyStringImpl::Compare(const char *other)
{
	if ( fString.Compare(other) == 0 ) {
		return 0;
	}
	return -1;
}

long AnyStringImpl::AsLong(long dflt)
{
	return fString.AsLong(dflt);
}

double AnyStringImpl::AsDouble(double dflt)
{
	return fString.AsDouble(dflt);
}

String AnyStringImpl::AsString(const char *) const
{
	return fString;
}

const char *AnyStringImpl::AsCharPtr(const char *) const
{
	return (const char *)fString; // PS: fString.AsCharPtr(dft); use operator const char * instead
}

const char *AnyStringImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = fString.Length();
	return (const char *)fString;
}

void AnyStringImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	v.VisitCharPtr(fString, this, lIdx, slotname);
}

AnyImpl *AnyStringImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyStringImpl(this->fString, a);
	return ret;
}

//---- AnyKeyTable --------------------------------------------------
#define LOADFACTOR 0.75

long IFANextPrime(long x)
{
	if (x <= 3) {
		return 3;
	}
	if ( !(x & 0x01) ) {
		++x;
	}

	for (;;) {
		long n;
		for (n = 3; (n *n <= x) && ((x % n) != 0); ++++n) {
		}
		if (n * n > x) {
			return x;
		}
		++++x;
	}
}

AnyKeyTable::AnyKeyTable(AnyArrayImpl *table, long initCapacity)
	: fKeyTable(table)
	, fHashTable(0)
	, fThreshold(0)
	, fCapacity(0)
	, fAllocator(fKeyTable->MyAllocator())
{
	InitTable(initCapacity);
}

AnyKeyTable::~AnyKeyTable()
{
	if (fHashTable) {
		Clear();
		fAllocator->Free(fHashTable);
		fKeyTable = 0;
		fHashTable = 0;
	}
}

void AnyKeyTable::InitTable(long cap)
{
	if (cap < cInitCapacity) {
		cap = cInitCapacity;
	}
	fCapacity = IFANextPrime(cap);
	fHashTable = (long *)fAllocator->Malloc(fCapacity * sizeof(long));
	fThreshold = (3 * fCapacity) / 4;
	Clear();
}

void AnyKeyTable::Clear()
{
	// only works with 2 complement binary arithmetic!
	memset(fHashTable, -1, sizeof(long) * fCapacity);
}

long AnyKeyTable::DoHash(const char *key, bool append, long sizehint, u_long hashhint) const
{
	// calculate some index into fHashTable
	long keylen = sizehint;
	long hashval = (hashhint) ? hashhint : IFAHash(key, keylen);
	long hash = hashval % fCapacity;

	// look for next free slot
	// do wrap around search
	long bufSz = fKeyTable->fBufSize;

	long i = hash;
	do {
		long lIdx = fHashTable[i];
		switch (lIdx) {
			case -1:
				return i;
			case -2:// slot is deleted
				if (append) {
					return i;
				} else {
					break;
				}
			default: {
				long at = fKeyTable->IntAt(lIdx);
				const String &keyAtVal = fKeyTable->fContents[at/bufSz][at%bufSz].Key(); // might be null
				if (keylen == keyAtVal.Length()) {
					const char *keyPtr = key, *keyAtValPtr = keyAtVal;
					const char *eptr = keyPtr + keylen;
					for ( ; keyPtr < eptr; ++keyPtr, ++keyAtValPtr) {
						if ( *keyPtr != *keyAtValPtr ) {
							goto loop;
						}
					}
					return i; // we found the key
				}
			}
		}
	loop:
		// increment and modulo capacity -- NO modulo is slow!
		// ensure we wrap arround in the table
		if (++i >= fCapacity) {
			i -= fCapacity;
		}
	} while ( i != hash ); // finish loop if wrapped around
	return -1;
}

long AnyKeyTable::Append(const char *key, long atIndex)
{
	if ( key ) {
		if ( fKeyTable->fSize > fThreshold) {
			// everything gets a new place in the hashtable
			// so we're done with it
			Rehash(fCapacity * 2);
		}
		// calculate hash index and put key table index
		// into it
		fHashTable[DoHash(key, true)] = atIndex;
	}
	return atIndex;
}
void AnyKeyTable::Update(long fromIndex)
{
	for (long i = 0; i < fCapacity; ++i) {
		long lIdx = fHashTable[i];
		if ( lIdx == fromIndex ) {
			fHashTable[i] = -2;    // mark as deleted
		} else if ( lIdx > fromIndex ) {
			fHashTable[i] = lIdx - 1;    // update position in keytable
		}
	}
}

long AnyKeyTable::At(const char *key, long sizehint, u_long hashhint) const
{
	// returns valid external index into key table
	// or -1= not found
	long lIdx = DoHash(key, false, sizehint, hashhint);
	if ( lIdx > -1 ) {
		return fHashTable[lIdx];
	}
	return lIdx;
}

void AnyKeyTable::Rehash(long newCap)
{
	long oldCapacity = fCapacity;
	long *ot = fHashTable;

	// allocate new table with new capacity
	// table may expand or shrink
	InitTable(newCap);

	// iterate over the old table and rehash
	// values
	for ( long i = 0; i < oldCapacity; ++i ) {
		register long slot = ot[i];

		if (slot > -1) {	// assumption: we found an index for a key
			long at = fKeyTable->IntAt(slot);
			register const char *key = fKeyTable->fContents[fKeyTable->IntAtBuf(at)][fKeyTable->IntAtSlot(at)].Key();
			Assert(key);
			long lIdx = DoHash(key);
			Assert(lIdx > -1 && lIdx < fCapacity);
			fHashTable[lIdx] = slot;
		}
	}
	// free old table
	fAllocator->Free(ot);
}

void AnyKeyTable::PrintHash()
{
	for (long i = 0; i < fCapacity; ++i) {
		if ( fHashTable[i] > -1 ) {
			String m;
			m << "[" << i << "]<" << fHashTable[i] << "> ";
			SysLog::WriteToStderr(m);
		}
	}
	SysLog::WriteToStderr("\n", 1);
}

void *AnyKeyTable::operator new(size_t size, Allocator *a)
{
	if (a) {
		return a->Calloc(1, size);
	} else {
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyKeyTable::operator delete(void *d, Allocator *a)
{
	if (d) {
//#if defined(PURE_LEAK) || defined(__linux__)
//		a = (a ? a : Storage::Current());
//#endif
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
	}
}
#endif

void AnyKeyTable::operator delete(void *d)
{
	if (d) {
		Allocator *a = ((AnyKeyTable *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyKeyTable::operator delete(d, a);
#else
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
#endif
	}
}

//---- AnyIndTable --------------------------------------------------
AnyIndTable::AnyIndTable(long initCapacity, Allocator *a)
	: fIndexTable(0)
	, fEmptyTable(0)
	, fCapacity(initCapacity)
	, fSize(0)
	, fAllocator(a)
{
	fEmptyTable = 0;
	InitTable(initCapacity);
	InitEmpty(0, fCapacity);
}

AnyIndTable::~AnyIndTable()
{
	if (fIndexTable) {
		Clear();
		fAllocator->Free(fIndexTable);
		fAllocator->Free(fEmptyTable);

		fIndexTable = 0;
	}
}

void AnyIndTable::InitTable(long cap)
{
	fCapacity = cap;
	fIndexTable = (long *)fAllocator->Malloc(fCapacity * sizeof(long));
	Clear();
}

void AnyIndTable::Clear()
{
	memset(fIndexTable, -1, sizeof(long) * fCapacity);
}

void AnyIndTable::Expand(long slot)
{
	Assert( slot >= fCapacity );

	// save old index table and capacity
	long *old = fIndexTable;
	long oldCap = fCapacity;

	// check for sufficient new capacity
	if ( slot >= fCapacity * 2 ) {
		fCapacity = slot;
	}

	// initalize index table with new capacity
	InitTable(fCapacity * 2);

	// initialize the empty table
	InitEmpty(oldCap, fCapacity);

	// initalize the indices in the index table
	InitIndices(slot, old);

	fAllocator->Free(old);
//	delete [] old;
}

void AnyIndTable::InitEmpty(long oldCap, long newCap)
{
	long *old = fEmptyTable;
	long sz = 0;
	long i = 0;

	// allocate the new size of the empty table
	fEmptyTable = (long *)fAllocator->Malloc(newCap * sizeof(long));

	// calculate the size to be copied
	if ( oldCap < newCap )	{
		sz = oldCap;
	} else	{
		sz = newCap;    // the size has shrunk
	}

	// copy the empty index array
	for (i = 0; i < sz; ++i)	{
		fEmptyTable[i] = old[i];
	}

	// fill in straight index (never used so far)
	if ( sz < newCap )
		for (i = sz; i < newCap; ++i) {
			fEmptyTable[i] = i;
		}

	fAllocator->Free(old);
}

void AnyIndTable::InitIndices(long slot, long *ot)
{
	Assert( fSize <= slot );

	long i = 0;

	// copy old indices if they exist
	if ( ot ) {
		for (i = 0; i < fSize; ++i) {
			fIndexTable[i] = ot[i];
		}
	}

	// reuse deleted slots if they exist
	for (i = fSize; i <= slot; ++i) {
		fIndexTable[i] = fEmptyTable[i];
	}
	fSize = slot + 1;
}

long AnyIndTable::At(long slot)
{
	Assert( slot >= 0 );

	// check for capacity overflow
	if ( slot >= fCapacity ) {
		Expand(slot);
	}

	// check for size overflow
	if ( slot >= fSize ) {
		InitIndices(slot, 0);
	}

	// return the index
	Assert( slot < fSize );
	return fIndexTable[slot];
}

long AnyIndTable::FindAt(long slot)
{
	Assert( slot >= 0 );

	// check for capacity overflow
	if ( slot < fCapacity && slot < fSize) {
		return fIndexTable[slot];
	}
	return -1L;
}

void AnyIndTable::Remove(long slot)
{
	Assert( slot >= 0 && slot < fSize );

	// save the index for later reuse
	fEmptyTable[fSize-1] = fIndexTable[slot];

	// shift the table down
	for (long i = slot; i < fSize - 1; ++i) {
		fIndexTable[i] = fIndexTable[i+1];
	}

	--fSize;
}
void AnyIndTable::Swap(long l, long r)
{
	Assert( l >= 0 && l < fSize );
	Assert( r >= 0 && r < fSize );
	long t = fIndexTable[l];
	fIndexTable[l] = fIndexTable[r];
	fIndexTable[r] = t;
}
void AnyIndTable::SetIndex(long slot, long index)
{
	Assert( slot >= 0 && slot < fSize );
	if (slot >= 0 && slot < fSize) {
		fIndexTable[slot] = index;
	} else {
		cerr << "OOPS, slot = " << slot << " index = " << index << " fSize = " << fSize << endl;
	}
}
void AnyIndTable::PrintTable()
{
	String m("IndexTable: \n");
	SysLog::WriteToStderr(m);
	for ( long i = 0; i < fSize; ++i) {
		String m1;
		m1 << "[" << i << "]<" << fIndexTable[i] << ">" << "\n";
		SysLog::WriteToStderr(m);
	}
}

void *AnyIndTable::operator new(size_t size, Allocator *a)
{
	if (a) {
		return a->Calloc(1, size);
	} else {
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyIndTable::operator delete(void *d, Allocator *a)
{
	if (d) {
//#if defined(PURE_LEAK) || defined(__linux__)
//		a = (a ? a : Storage::Current());
//#endif
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
	}
}
#endif

void AnyIndTable::operator delete(void *d)
{
	if (d) {
		Allocator *a = ((AnyIndTable *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyIndTable::operator delete(d, a);
#else
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
#endif
	}
}

//---- AnyArrayImpl -----------------------------------------------------------------
static const char *gcArrayText = "AnyArrayImpl";

AnyArrayImpl::AnyArrayImpl(Allocator *a)
	: AnyImpl(a)
	, fContents(0)
	, fKeys(0)
	, fInd(0)
	, fCapacity(4)
	, fSize(0)
	, fBufSize(4)
	, fNumOfBufs(0)
{
	// allocate a default size structure
	// without keys
	fCapacity = AdjustCapacity(fCapacity);
	AllocMemory();
}

AnyArrayImpl::~AnyArrayImpl()
{
	if (fContents) {
		// free the buffers themselves
		for (int j = 0; j < fNumOfBufs; ++j) {
#if defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
			for ( int k = 0; k < fBufSize; ++k) {
				fContents[j][k].~AnyKeyAssoc();
			}
			fAllocator->Free(fContents[j]);
#else
			delete [] (fContents[j]);
#endif
		}
		// free the buffer Ptr
		MyAllocator()->Free(fContents);

		// free the key array
		if ( fKeys ) {
			delete fKeys;
		}

		// free the index array
		delete fInd;

		// reset everything
		fInd = 0;
		fContents = 0;
		fKeys = 0;
		fSize = fCapacity = 0;
	}
}

const char *AnyArrayImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = strlen(gcArrayText);
	return gcArrayText;
}

Anything &AnyArrayImpl::At(long slot)
{
	// return an address of an anything
	// residing at slot
	// expand the buffers as necessary to fullfill
	// the request

	long newsz = slot + 1;

	// check for logical expansion
	if ( (newsz > fSize) ) {
		// reset the size
		fSize = newsz;

		// check for physical expansion
		if ( newsz >= fCapacity ) {
			Expand(newsz);
		}
	}

	// calculate the adress of the anything
	long at = IntAt(slot);
	return fContents[IntAtBuf(at)][IntAtSlot(at)].Value();
}

Anything &AnyArrayImpl::At(const char *key)
{
	// calculate the adress of an anything
	// given its key

	long slot = -1;
	if ( !fKeys ) {
		// no keys exist yet
		fKeys = new(MyAllocator()) AnyKeyTable(this);
	} else {
		// find index of key or return -1
		slot = fKeys->At(key);
	}

	long at;

	if (slot < 0) {
		// key doesn't exist so append this key in the key array
		// with the according slot
		slot = fKeys->Append(key, fSize);
		at = IntAt(slot);
		// set the key in the any key assoc structure
		fContents[IntAtBuf(at)][IntAtSlot(at)].SetKey(key);

		// return the element found
		// this creates a new element
		return At(fSize);
	}
	// the element already exists the slot is an internal slot
	at = IntAt(slot);
	return fContents[IntAtBuf(slot)][IntAtSlot(slot)].Value();
}

long AnyArrayImpl::FindIndex(const char *key, long sizehint, u_long hashhint)
{
	// find the index of an anything given
	// its key. It returns -1 if not defined

	if ( !fKeys ) {
		return -1;
	}

	// return index from keyarray
	return fKeys->At(key, sizehint, hashhint);
}

long AnyArrayImpl::FindIndex(const long lIdx)
{
	// find the index of an anything given
	// its index. It returns -1 if not defined
	if ( !fInd ) {
		return -1L;
	}

	// return index from indexarray
	return fInd->FindAt(lIdx);
}

long AnyArrayImpl::Contains(const char *k)
{
	// search the value in the array
	// assume an array of simple strings
	// otherwise the result could be misleading
	// e.g. an AnyArrayImpl returns always AnyArrayImpl

	Assert(k);	// require a valid search string

	long i; 	// external slot index
	long at;	// internal buffer index
	for (i = 0; i < fSize; ++i) {
		at = IntAt(i);	// calculate the internal index
		Assert(at >= 0 && at < fCapacity);
		if ( strcmp(fContents[IntAtBuf(at)][IntAtSlot(at)].Value().AsCharPtr(""), k) == 0 ) {
			return i;
		}
	}
	return -1;
}

void AnyArrayImpl::Remove(long slot)
{
	// remove an anything from the internal
	// structures

	// check the slot range
	if (slot >= 0 && slot < fSize) {
		// delete the internal key assoc
		// at slot index
		long at = IntAt(slot);
//#if defined(PURE_LEAK) || defined(__linux__)
//		MyAllocator() ? MyAllocator() : Storage::Current();
//#endif
		fContents[IntAtBuf(at)][IntAtSlot(at)] = AnyKeyAssoc(MyAllocator());	// reset it to initial empty assoc

		// remove the slot from the index array
		fInd->Remove(slot);

		// update the key array if it exists
		if ( fKeys ) {
			fKeys->Update(slot);
		}

		// update the size
		--fSize;
	} else {
		String msg;
		SYSERROR(msg.Append("index ").Append(slot).Append(" out of range"));
	}
}
const String &AnyArrayImpl::Key(long slot)
{
	if (slot >= 0 && slot < fSize) {
		long at = IntAt(slot);
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}
const String &AnyArrayImpl::IntKey(long at)
{
	if (at >= 0 && at < fCapacity) {
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}
const Anything &AnyArrayImpl::IntValue(long at)
{
	if (at >= 0 && at < fCapacity) {
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Value();
	}
	return fgAnyEmpty;
}
const char *AnyArrayImpl::SlotName(long slot)
{
	// calculate the slot name given an
	// index
	const String &k = Key(slot);
	return (k.Length() > 0) ? (const char *)k : (const char *)0;
}
const String &AnyArrayImpl::VisitSlotName(long slot)
{
	// calculate the slot name given an
	// index

	// first check the range
	if (slot >= 0 && slot < fSize) {
		long at = IntAt(slot);
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}

void AnyArrayImpl::Expand(long newsize)
{
	bool allocOk = true;
	long oldCap = fCapacity;

	// set the new capacity
	if (newsize > fCapacity * 2) {
		fCapacity = AdjustCapacity(newsize);
	} else {
		fCapacity = AdjustCapacity(fCapacity * 2);
	}

	// check for the range of the capacity
	Assert((fCapacity % fBufSize) == 0);

	// calculate the number of buffers needed for the expansion
	long numOfExistingBufs = fNumOfBufs;
	long numOfNewBufs = fCapacity / fBufSize; //fCapacity / fBufSize + 1;

	Assert(numOfNewBufs *fBufSize >= newsize);

	// allocate new ptr buffer if necessary
	if ( numOfNewBufs > fNumOfBufs ) {
		// expand the number of needed ptr buffers
		fNumOfBufs = numOfNewBufs;

		// allocate the new size
		AnyKeyAssoc **old = fContents;
		fContents = (AnyKeyAssoc **)MyAllocator()->Calloc(fNumOfBufs, sizeof(AnyKeyAssoc *));
		if (fContents) {
			for (long bufs = 0; bufs < numOfExistingBufs; ++bufs) {
				fContents[bufs] = old[bufs];
			}
			MyAllocator()->Free(old); // frees the old ptr buffer array not the contents buffer
		} else {
			static const char crashmsg[] = "FATAL: AnyArrayImpl::Expand calloc failed (increasing pointer buffer). I will crash :-(\n";
			SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));

			fContents = old;
			fCapacity = oldCap;
			allocOk = false;
		}
	}

	// allocate the needed buffers
	if (allocOk) {
		for (long i = numOfExistingBufs; i < fNumOfBufs && allocOk; ++i) {
			Assert(MyAllocator() != 0);
			//#if defined(PURE_LEAK) || defined(__linux__)
			//			MyAllocator() = MyAllocator() ? MyAllocator() : Storage::Current();
			//#endif
#if defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
			fContents[i] = (AnyKeyAssoc *)MyAllocator()->Calloc(fBufSize, sizeof(AnyKeyAssoc));
#else
			fContents[i] = new (MyAllocator()) AnyKeyAssoc[fBufSize];
#endif
			if (fContents[i] == 0) {
				static const char crashmsg[] = "FATAL: AnyArrayImpl::Expand calloc failed (assigning memory to increased pointer buffers).\nI will crash :-(\n";
				SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));

				allocOk = false;
			} else {
				for ( long keyAssocKeyCnt = 0L; keyAssocKeyCnt < fBufSize; ++keyAssocKeyCnt ) {
					fContents[i][keyAssocKeyCnt].Init(MyAllocator());
				}
			}
		}
	}
}

void AnyArrayImpl::AllocMemory()
{
	// calculate the number of needed buffers
	// allocate at least fBufSize buffers of the size fBufSize
	fNumOfBufs = fCapacity / fBufSize; // round to the next multiple
	//(fCapacity / fBufSize > fBufSize) ? fCapacity/fBufSize : fBufSize;
	fContents = (AnyKeyAssoc **) MyAllocator()->Calloc(fNumOfBufs, sizeof(AnyKeyAssoc *));

	// allocate the index table
	fInd = new (MyAllocator()) AnyIndTable(fCapacity, MyAllocator());

	// allocate the buffers holding the
	// Any Key Assocs
	for (long i = 0; i < fNumOfBufs; ++i) {
		// must not use calloc to ensure proper initialization of Anything instance variables
		Assert(MyAllocator() != 0);
//#if defined(PURE_LEAK) || defined(__linux__)
//		MyAllocator() = MyAllocator() ? MyAllocator() : Storage::Current();
//#endif

#if defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
		fContents[i] = (AnyKeyAssoc *)MyAllocator()->Calloc(fBufSize, sizeof(AnyKeyAssoc));
#else
		fContents[i] = new (MyAllocator()) AnyKeyAssoc[fBufSize];		// (MyAllocator());
#endif

		if ( fContents[i] == 0 ) {
			String msg("Memory allocation failed!");
			SYSERROR(msg);
		} else {
			for ( long keyAssocKeyCnt = 0L; keyAssocKeyCnt < fBufSize; ++keyAssocKeyCnt ) {
				fContents[i][keyAssocKeyCnt].Init(MyAllocator());
			}
		}
	}
}

void AnyArrayImpl::PrintKeys()
{
	long hash = -1;
	for (long i = 0; i < fSize; ++i) {
		long at = IntAt(i);
		if (fKeys) {
			hash = 	fKeys->At(fContents[IntAtBuf(at)][IntAtSlot(at)].Key());
		}
		String m;
		m << "[" << i << "]<" << NotNullStr(fContents[IntAtBuf(at)][IntAtSlot(at)].Key()) << ">(" << hash << ")" << "\n";
		SysLog::WriteToStderr(m);
	}
}

void AnyArrayImpl::PrintHash()
{
	if (fKeys) {
		fKeys->PrintHash();
	} else {
		SysLog::WriteToStderr("*", 1);
	}
}

AnyImpl *AnyArrayImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	String adr(ThisToHex(), a);
	Anything &refEntry = xreftable[adr];
	AnyImpl *res = (AnyImpl *)refEntry.AsIFAObject(0);
	if (res != NULL) {
		res->Ref(); // do not forget to count
		return res;
	}

	AnyArrayImpl *ret = new ((a) ? a : Storage::Current()) AnyArrayImpl(a);
	refEntry = (IFAObject *)ret;
	long count = this->GetSize();

	for (long i = 0 ; i < count; ++i) {
		ret->At(this->SlotName(i)) = this->At(i).DeepClone(a, xreftable);
	}
	return ret;
}
#if 0
void AnyArrayImpl::Qsort(long left, long right)
{
	// taken from wirth
	long i = left, j = right;
	long median = (left + right) / 2;
	const String &mediankey = Key(median);
	do {
		while (mediankey.Compare(Key(i)) > 0) {
			++i;
		}
		while (mediankey.Compare(Key(j)) < 0) {
			--j;
		}
		if (i <= j) {
			Swap(i, j);
			++i;
			--j;
		}
	} while (i <= j);
	if (left < j) {
		Qsort(left, j);
	}
	if (right > i) {
		Qsort(i, right);
	}
}
int AnyArrayImpl::CompareKeys(long i, long j)
{
	return Key(i).Compare(Key(j));
}
void AnyArrayImpl::DownHeap (long v, long n)
{
	long w = 2 * v + 1;           // first descendant of v
	while (w < n) {
		if (w + 1 < n)           // is there a second descendant?
			if (CompareKeys(w + 1, w) > 0) {
				++w;
			}
		// w is the descendant of v with maximum label

		if (CompareKeys(v, w) >= 0) {
			return;    // v has the heap property
		}
		// otherwise
		Swap(v, w);;      // exchange the labels of v and w
		v = w;
		w = 2 * v + 1;      // continue
	}
}
void AnyArrayImpl::BuildHeap ()
{
	long n = GetSize();
	for (long v = n / 2 - 1; v >= 0; --v) {
		DownHeap(v, n);
	}
}
void AnyArrayImpl::HeapSort()
{
	BuildHeap();
	long n = GetSize();
	for (long v = n - 1; v > 0; --v) {
		Swap(0, v);
		DownHeap(0, v);
	}
}
#endif

void AnyArrayImpl::MergeByComparer(long lo, long hi, long m, const AnyIntCompare &comparer)
{
	if (hi < m + 1 || lo > m) {
		return;    // nothing to merge
	}
	long i, j, k;
	const long sz = m - lo + 1;
#if defined(WIN32) && (_MSC_VER <= 1200) // VC6 or lower
	long *a = new long[sz];		// temporary array of lower half
#else
	long a[sz];					// temporary array of lower half
#endif
	for (k = 0, i = lo; i <= m && k < sz; ++i, ++k) {
		a[k] = IntAt(i);
	}
	Assert(k == sz);
	Assert(i > m);
	j = m + 1;
	k = 0;
	i = lo;
	while (k < sz && j <= hi) {
		Assert(i < j);
		if (comparer.Compare(*this, a[k], IntAt(j)) <= 0) {
			fInd->SetIndex(i, a[k]);
			++k;
		} else {
			fInd->SetIndex(i, IntAt(j));
			++j;
		}
		++i;
	}
	// copy the remainder
	while ( k < sz && i < j ) {
		Assert(j > hi);
		fInd->SetIndex(i, a[k]);
		++i;
		++k;
	}
	Assert(i == j);
	Assert(k == sz);
#if defined(WIN32) && (_MSC_VER <= 1200) // VC6 or lower
	delete[] a;
#endif
}

void AnyArrayImpl::MergeSortByComparer(long low, long high, const AnyIntCompare &comparer)
{
	if (low >= high) {
		return;
	}
	long middle = (low + high) / 2;
	MergeSortByComparer(low, middle, comparer);
	MergeSortByComparer(middle + 1, high, comparer);
	MergeByComparer(low, high, middle, comparer);
}

void AnyArrayImpl::SortByKey()
{
	//HeapSort();
	//Qsort(0,GetSize()-1);
	MergeSortByComparer(0, GetSize() - 1, theKeyComparer);
	RecreateKeyTabe();
}

void AnyArrayImpl::SortReverseByKey()
{
	MergeSortByComparer(0, GetSize() - 1, theReverseKeyComparer);
	RecreateKeyTabe();
}

void AnyArrayImpl::SortByAnyComparer(const AnyComparer &comparer)
{
	MergeSortByComparer(0, GetSize() - 1, AnyIntComparerCompare(comparer));
	RecreateKeyTabe();
}

void AnyArrayImpl::RecreateKeyTabe()
{
	if (fKeys) {
		fKeys->Clear();
		for (long i = 0; i < fSize; ++i) {
			const char *sn = SlotName(i);
			if (sn) {
				fKeys->Append(sn, i);
			}
		}
	}
}

const char *AnyArrayImpl::AsCharPtr(const char *) const
{
	return gcArrayText;
}

String AnyArrayImpl::AsString(const char *) const
{
	return gcArrayText;
}

void AnyArrayImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	ROAnything wrapit((AnyImpl *)this); //is there a nicer way
	Assert(wrapit.fAnyImp == (AnyImpl *)this);		// check for auto conversion by compiler
	v.VisitArray(wrapit, this, lIdx, slotname);
}

int	AnyArrayImpl::AnyIntComparerCompare::Compare(AnyArrayImpl &that, long leftInt, long rightInt) const
{
	return ac.Compare(that.IntValue(leftInt), that.IntValue(rightInt));
}

AnyArrayImpl::AnyIntKeyCompare AnyArrayImpl::theKeyComparer;
AnyArrayImpl::AnyIntReverseKeyCompare AnyArrayImpl::theReverseKeyComparer;
