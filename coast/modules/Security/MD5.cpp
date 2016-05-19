/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MD5.h"

// ------------------- MD5 ---------------------------------------------
/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

#include <openssl/md5.h>

#include <cstring>		/* for memcpy() */

// Initially MD5Context was Colin Plumb's implementation of MD5
// now it's just a wrapper for openssl function calls.
class MD5Context {
	MD5Context(const MD5Context &);
	MD5Context &operator=(const MD5Context &);
public:
	MD5Context();

	void Init();
	void Update(const unsigned char *buf, unsigned len);
	// Final erases ctx! Call Init if you have to reuse this context.
	void Final(unsigned char digest[MD5_DIGEST_LENGTH]);

protected:
	MD5_CTX ctx;
};

MD5Context::MD5Context() {
	Init();
}

void MD5Context::Init() {
	MD5_Init(&ctx);
}

void MD5Context::Update(unsigned char const *buf, unsigned len) {
	MD5_Update(&ctx, buf, len);
}

void MD5Context::Final(unsigned char digest[MD5_DIGEST_LENGTH]) {
	MD5_Final(digest, &ctx);
}

RegisterSigner(MD5Signer);
RegisterAliasSecurityItem(md5, MD5Signer);

MD5Signer::MD5Signer(const char *name) :
		Signer(name) {
	InitKey(fgcLegacyMasterKey);
}

void MD5Signer::InitKey(const String &key) {
	StartTrace1(MD5Signer.InitKey, fName);
	Trace("Configured name: " << fName << " using key:" << key);
	fKey = key;
}

void MD5Signer::DoHash(const String &cleartext, String &hashvalue) {
	StartTrace(MD5Signer.DoHash);
	Trace("Cleartext: <" << cleartext << ">");

	MD5Context md5ctx;
	unsigned char digest[16];

	md5ctx.Update((const unsigned char *) (const char *) cleartext, cleartext.Length());
	md5ctx.Final(digest);

	hashvalue = "";
	hashvalue.Append((void *) digest, 16);
//	MD5 md5ctx;
//
//	md5ctx.Update((const unsigned char *)(const char *)cleartext, cleartext.Length());
//	md5ctx.Finalize(hashvalue);

	Trace("Hashvalue: <" << String().AppendAsHex((const unsigned char *)(const char *)hashvalue, hashvalue.Length(), ' ') << ">");
}
void MD5Signer::DoEncode(String &scrambledText, const String &cleartext) const {
	DoSign(fKey, scrambledText, cleartext);
}
bool MD5Signer::DoDecode(String &cleartext, const String &scrambledText) const {
	return DoCheck(fKey, cleartext, scrambledText);
}

void MD5Signer::DoSign(const String &strkey, String &scrambledText, const String &cleartext) {
	StartTrace(MD5Signer.DoSign);
	Trace("key: <" << strkey << ">");
	Trace("Cleartext: <" << cleartext << ">");

	MD5Context md5ctx;
	unsigned char digest[16];
	const char *msg = cleartext;
	const char *key = strkey;

	md5ctx.Update((const unsigned char *) msg, cleartext.Length());
	md5ctx.Update((const unsigned char *) key, strkey.Length());
	md5ctx.Final(digest);

	String strDigest((void *) digest, 16);

	scrambledText << strDigest << cleartext << '$'; // end marker

}

bool MD5Signer::DoCheck(const String &strkey, String &cleartext, const String &scrambledText) {
	StartTrace(MD5Signer.DoCheck);
	Trace("key: <" << strkey << ">");
	Trace("Scrambled: <" << String().AppendAsHex((const unsigned char *)(const char *)scrambledText, scrambledText.Length(), ' ') << ">");

	long effLength = scrambledText.StrRChr('$');
	Trace("EffLength: " << effLength);

	if (effLength == -1) {
		effLength = scrambledText.Length();
	}
	if (effLength <= 16) {
		return false;
	}Trace("EffLength: " << effLength);
	MD5Context md5ctx;
	unsigned char digest[16];
	const char *msg = scrambledText;
	msg += 16;

	const char *key = strkey;

	Trace("Msg: <" << msg << ">");
	md5ctx.Update(((const unsigned char *) msg), (effLength - 16));
	md5ctx.Update((const unsigned char *) key, strkey.Length());
	md5ctx.Final(digest);
	Trace("Msg: <" << msg << ">");

	if (::memcmp(digest, (const char *) scrambledText, 16) == 0) {
		cleartext = scrambledText.SubString(16, effLength - 16);
		Trace("Cleartext: <" << cleartext << ">");
		Trace("returning true");
		return true;
	}Trace("returning false");

	return false;
}

