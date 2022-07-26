/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "URLUtils.h"

#include "Resolver.h"
#include "SystemFile.h"
#include "Tracer.h"

#include <cstring>

namespace coast {
	namespace urlutils {
		// replace '+' characters in-place into ' '
		void Convert(String &str) {
			if (str.Length() > 0) {
				for (long l = 0; l < str.Length(); ++l) {
					if (str[l] == '+') {
						str.PutAt(l, ' ');
					}
				}
			}
		}
		String &TrimChars(String &str, bool front, char c) {
			StartTrace(URLUtils.TrimChars);
			// remove leading blanks
			long at = 0;
			if (front) {
				at = 0;
				while (c == str[at]) {
					++at;
				}
				if (at != 0) {
					str.TrimFront(at);
				}
			} else {
				at = str.Length() - 1;
				while (c == str[at]) {
					--at;
				}
				if (at != (str.Length() - 1)) {
					str.Trim(at + 1);
				}
			}
			return str;
		}

		String &TrimBlanks(String &str, bool front) {
			StartTrace(URLUtils.TrimBlanks);
			// remove leading blanks
			return TrimChars(str, front);
		}

		String &TrimENDL(String &line) {
			long llen = line.Length();
			if (line[(long)(llen - 2)] == '\r' && line[(long)(llen - 1)] == '\n') {
				line.Trim(llen - 2);
			} else if (line[(long)(llen - 1)] == '\r') {
				line.Trim(llen - 1);
			} else if (line[(long)(llen - 1)] == '\n') {
				line.Trim(llen - 1);
			}
			return line;
		}

		char DecodeSpecialChars(const String &str, char c, long &lPos, long offset) {
			StartTrace(URLUtils.DecodeSpecialChars);
			if (lPos + offset < str.Length()) {
				switch (str[lPos + offset]) {
						// other special cases
					case ' ':
					case '"':
					case '%':
					case '&':
					case '?':
					case '/':
						lPos += offset;
						c = str[lPos];	// take next char literal
					default:;
				}
			}
			return c;
		}

		void ExtractHex(const String &str, String &res, long &lPos, long delta) {
			StartTrace(URLUtils.ExtractHex);
			res.AppendTwoHexAsChar(&((const char *)str)[lPos + (delta - 2L)]);
		}

		void ExtractDecimal(const String &str, String &res, long &lPos, long delta) {
			StartTrace(URLUtils.ExtractDecimal);
			long value(String(str + lPos + 2, (delta - 2L)).AsLong(0L));
			if (value <= 255) {
				res.Append((char)(unsigned char)value);
				Trace("unsigend char is:" << (unsigned char)value);
			}
		}

		void DecodeSpecialHTMLChars(const String &str, String &res, long &lPos) {
			StartTrace(URLUtils.DecodeSpecialHTMLChars);
			// character encodings accocrding to HTML4 W3C recommendation
			// Shortcut
			if (str.StrChr('&') == -1L) {
				res = str;
				return;
			}

			long endPos(str.StrChr(';', lPos));
			Trace("endPos: " << endPos << " lPos: " << lPos);
			if ((str[lPos] != '&' || str[lPos + 1] != '#') || (endPos == -1L) || (endPos - lPos > 7L)) {
				Trace("Appending: " << str[lPos] << " lPos: " << lPos);
				res.Append(str[lPos]);
				return;
			}

			long delta(endPos - lPos);
			Trace("Delta: " << delta << " P0: " << str[lPos + 0] << " P1: " << str[lPos + 1] << " P2: " << str[lPos + 2]
							<< " P3: " << str[lPos + 3] << " P4: " << str[lPos + 4] << " P5: " << str[lPos + 5]
							<< " P6: " << str[lPos + 6] << " P7: " << str[lPos + 7]);

			if (delta == 7) {
				// &#xffff;
				if ((str[lPos + 2] == 'x' || str[lPos + 2] == 'X') && (isxdigit(str[lPos + 3]) != 0) &&
					(isxdigit(str[lPos + 4]) != 0) && (isxdigit(str[lPos + 5]) != 0) && (isxdigit(str[lPos + 6]) != 0)) {
					if ((str[lPos + 3] == '0') && (str[lPos + 4] == '0')) {
						ExtractHex(str, res, lPos, delta);
					}
				}
			}
			if (delta == 6) {
				// &#xfff;
				if ((str[lPos + 2] == 'x' || str[lPos + 2] == 'X') && (isxdigit(str[lPos + 3]) != 0) &&
					(isxdigit(str[lPos + 4]) != 0) && (isxdigit(str[lPos + 5]) != 0)) {
					if (str[lPos + 3] == '0') {
						ExtractHex(str, res, lPos, delta);
					}
				}
				// &#dddd;
				else if ((isdigit(str[lPos + 2]) != 0) && (isdigit(str[lPos + 3]) != 0) && (isdigit(str[lPos + 4]) != 0) &&
						 (isdigit(str[lPos + 5]) != 0)) {
					ExtractDecimal(str, res, lPos, delta);
				}
			}
			if (delta == 5) {
				// &#xff;
				if ((str[lPos + 2] == 'x' || str[lPos + 2] == 'X') && (isxdigit(str[lPos + 3]) != 0) &&
					(isxdigit(str[lPos + 4]) != 0)) {
					ExtractHex(str, res, lPos, delta);
				}
				// &#ddd;
				else if ((isdigit(str[lPos + 2]) != 0) && (isdigit(str[lPos + 3]) != 0) && (isdigit(str[lPos + 4]) != 0)) {
					ExtractDecimal(str, res, lPos, delta);
				}
			}
			if (delta == 4) {
				// &xf
				if ((str[lPos + 2] == 'x' || str[lPos + 2] == 'X') && (isxdigit(str[lPos + 3]) != 0)) {
					char tmp[2] = {'0', str[lPos + (delta - 1L)]};
					res.AppendTwoHexAsChar(tmp);
				}
				// &#dd;
				else if ((isdigit(str[lPos + 2]) != 0) && (isdigit(str[lPos + 3]) != 0)) {
					ExtractDecimal(str, res, lPos, delta);
				}
			}
			if (delta == 3) {
				// &#d;
				if (isdigit(str[lPos + 2]) != 0) {
					ExtractDecimal(str, res, lPos, delta);
				}
			}
			lPos += delta;
			Trace("Result string is: [" << res << "]");
		}

		// encode the given char *p into res by expanding problematic characters into %XX escapes
		bool DoUrlEncode(const String &str, const String &exclusionSet, String &encoded, bool doCheck) {
			StartTrace(URLUtils.DoUrlEncode);
			char c = 0;

			// encoding scheme for HTTP (RFC1738):
			//	httpurl        = "http://" hostport [ "/" hpath [ "?" search ]]
			//	hpath          = hsegment *[ "/" hsegment ]
			//	hsegment       = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
			//	>> hsegment    = *[ "a-zA-Z0-9$-_.+!*'(),;:@&=" ]	// satisfy sniffparser '
			// superseeding as of RFC1808:
			//		URL         = ( absoluteURL | relativeURL ) [ "#" fragment ]
			//
			//		absoluteURL = generic-RL | ( scheme ":" *( uchar | reserved ) )
			//
			//		generic-RL  = scheme ":" relativeURL
			//
			//		relativeURL = net_path | abs_path | rel_path
			//
			//		net_path    = "//" net_loc [ abs_path ]
			//		abs_path    = "/"  rel_path
			//		rel_path    = [ path ] [ ";" params ] [ "?" query ]
			//		path        = fsegment *( "/" segment )
			//		fsegment    = 1*pchar					// 1* means: '1 or more characters of'
			//		segment     =  *pchar
			//
			//		params      = param *( ";" param )
			//		param       = *( pchar | "/" )
			//
			//		scheme      = 1*( alpha | digit | "+" | "-" | "." )
			//		net_loc     =  *( pchar | ";" | "?" )
			//		query       =  *( uchar | reserved )
			//		fragment    =  *( uchar | reserved )
			//
			//	this leads to a (f)segment: ';' is omitted compared to hsegment
			//	>> (f)segment   = *[ "a-zA-Z0-9$-_.+!*'(),:@&=" ]	// satisfy sniffparser '

			//	search         = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
			//	lowalpha       = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" |
			//					 "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" |
			//					 "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" |
			//					 "y" | "z"
			//	hialpha        = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
			//					 "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
			//					 "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
			//
			//	alpha          = lowalpha | hialpha
			//	digit          = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
			//					 "8" | "9"
			//	safe           = "$" | "-" | "_" | "." | "+"
			//	extra          = "!" | "*" | "'" | "(" | ")" | ","
			//	national       = "{" | "}" | "|" | "\" | "^" | "~" | "[" | "]" | "`"
			//	punctuation    = "<" | ">" | "#" | "%" | <">
			//
			//	reserved       = ";" | "/" | "?" | ":" | "@" | "&" | "="
			//	hex            = digit | "A" | "B" | "C" | "D" | "E" | "F" |
			//					 "a" | "b" | "c" | "d" | "e" | "f"
			//	escape         = "%" hex hex
			//
			//	unreserved     = alpha | digit | safe | extra
			//	uchar          = unreserved | escape
			//	>> uchar       = "a-zA-Z0-9$-_.+!*'(),"		// satisfy sniffparser '
			//	pchar          = uchar | ":" | "@" | "&" | "="
			//	>> pchar       = "a-zA-Z0-9$-_.+!*'(),:@&="		// satisfy sniffparser '

			// characters which always need escaping: 0x00-0x1F, 0x7F-0xFF
			// to be escaped too: "<>\"#%{}|\\^~[]`"
			// for path encoding we need to escape "?;" too
			if (str.Length() > 0) {
				for (long l = 0, sz = str.Length(); l < sz; ++l) {
					c = str[l];
					unsigned int ui = c;
					if (ui <= 0x20 || ui >= 0x7F || ui == '"' || ui == '#' || ui == '%' || ui == ';' || ui == '<' ||
						ui == '>' || ui == '?' || ui == '[' || ui == '\\' || ui == ']' || ui == '^' || ui == '`' || ui == '{' ||
						ui == '/' || ui == '}' || ui == '~') {
						if ((exclusionSet.StrChr(c) == -1L)) {
							if (doCheck) {
								Trace("failed at character [" << c << "]");
								return false;
							}
							encoded.Append('%');
							encoded.AppendAsHex(c);
							continue;
						}
					}
					encoded.Append(c);
				}
			}
			Trace("Result: " << encoded << " ExclusionSet: " << exclusionSet);
			return true;
		}

		void Pair(const char *buf, char delim, Anything &any, NormalizeTag normKey) {
			StartTrace1(URLUtils.Pair, "[" << NotNull(buf) << "]");
			if (buf != 0) {
				const char *p = strchr(buf, delim);
				if (p != 0) {
					String key(buf, p - buf);
					// will not work if \0 is contained in the string, on purpose
					++p;
					char *pclean = (char *)p;
					if (*pclean == '"' || *pclean == '\'') {
						pclean[strlen(p) - 1] = '\0';
						++p;
					}
					Normalize(key, normKey);

					AppendValueTo(any, key, p);
				} else if (*buf != '\0') {
					any.Append(buf);
				}
			}
		}

		void AppendValueTo(Anything &any, const String &key, const char *value) {
			StatTrace(URLUtils.AppendValueTo, "key [" << key << "] value [" << NotNull(value) << "]",
					  coast::storage::Current());
			if (key.Length() > 0) {
				if (value == 0) {
					any[key] = Anything();
				} else if (any.IsDefined(key)) {
					any[key].Append(value);
				} else {
					any[key] = value;
				}
			} else if ((value != 0) && (*value != 0)) {
				any.Append(value);
			}
		}

		void Normalize(String &str, NormalizeTag normKey) {
			StartTrace(URLUtils.Normalize);
			switch (normKey) {
				case eUpshift:
					str.ToUpper();
					break;
				case eDownshift:
					str.ToLower();
					break;
				case eUntouched:
					break;
				default:
					break;
			}
		}

		String Normalize(String const &str, NormalizeTag normKey) {
			String strKey(str);
			Normalize(strKey, normKey);
			return strKey;
		}

		void Split(const char *buf, char delim, Anything &out, char delim2, NormalizeTag norm) {
			StartTrace1(URLUtils.Split, "buffer [" << NotNull(buf) << "] token [" << delim2 << "]");
			StringTokenizer stok(buf, delim);
			String pair;
			while (stok.NextToken(pair)) {
				// remove leading blanks
				TrimBlanks(pair);
				Pair(pair, delim2, out, norm);
			}
		}

		void DecodeAll(Anything &a) {
			StartTrace(URLUtils.DecodeAll);
			String str;
			Anything at;

			for (long i = 0, sz = a.GetSize(); i < sz; ++i) {
				at = a[i];
				if (at.GetType() == AnyCharPtrType) {
					str = at.AsCharPtr("");
					a[i] = urlDecode(str);
				} else if (at.GetType() == AnyArrayType) {
					DecodeAll(at);
				}
			}
		}

		String ExhaustiveUrlDecode(const String &instr, URLCheckStatus &eUrlCheckStatus, bool replacePlusByBlank) {
			StartTrace1(URLUtils.ExhaustiveUrlDecode, "Url [" << instr << "]");
			String current(instr);
			String previous;
			eUrlCheckStatus = eOk;
			while (previous != current) {
				previous = current;
				URLCheckStatus eTmpUrlCheckStatus = coast::urlutils::eOk;
				current = urlDecode(previous, eTmpUrlCheckStatus, replacePlusByBlank);
				if (eTmpUrlCheckStatus == eSuspiciousChar) {
					eUrlCheckStatus = eSuspiciousChar;
				}
				Trace("Intermediate: " << current);
			}
			return current;
		}

		String ExhaustiveUrlDecode(const String &instr, bool replacePlusByBlank) {
			StartTrace1(URLUtils.ExhaustiveUrlDecode, "Url [" << instr << "]");
			String current(instr);
			String previous;
			while (previous != current) {
				previous = current;
				current = urlDecode(previous, replacePlusByBlank);
				Trace("Intermediate: " << current);
			}
			return current;
		}

		String ExhaustiveHTMLDecode(const String &instr) {
			StartTrace(URLUtils.ExhaustiveUrlDecode);
			String current(instr);
			String previous;
			while (previous != current) {
				previous = current;
				current = HTMLDecode(previous);
				Trace("Intermediate: " << current);
			}
			return current;
		}

		String HTMLDecode(const String &instr) {
			StartTrace(URLUtils.HTMLDecode);
			String res, str(instr);
			long length(str.Length());
			if (length > 0) {
				for (long lPos = 0; lPos < length; ++lPos) {
					DecodeSpecialHTMLChars(str, res, lPos);
				}
			}
			return res;
		}

		String RemoveUnwantedChars(const String &instr, const String &badOnes) {
			StartTrace1(URLUtils.RemoveUnwantedChars, "removing: " << badOnes << " from [" << instr << "]");
			String work(instr);
			long pos = 0;
			while ((pos = work.FirstCharOf(badOnes)) >= 0) {
				work.erase(pos, 1);
			}
			Trace("returning [" << work << "]");
			return work;
		}

		// decodes the given string into res by expanding %XX and %uXXXXX escapes
		String urlDecode(const String &instr, bool replacePlusByBlank) {
			long const tracePrefixLength = 80L;
			long const traceSuffixLength = 20L;
			StartTrace1(
				URLUtils.urlDecode,
				"dispatch [" << instr.SubString(0, tracePrefixLength)
									.Append(instr.Length() > tracePrefixLength
												? String("...").Append(instr.SubString(instr.Length() - traceSuffixLength))
												: "")
							 << "]");
			URLCheckStatus eUrlCheckStatus = coast::urlutils::eOk;
			return urlDecode(instr, eUrlCheckStatus, replacePlusByBlank);
		}

		// decodes the given string into res by expanding %XX and %uXXXXX escapes
		String urlDecode(const String &instr, URLCheckStatus &eUrlCheckStatus, bool replacePlusByBlank) {
			long const tracePrefixLength = 80L;
			long const traceSuffixLength = 20L;
			StartTrace1(URLUtils.urlDecode,
						"[" << instr.SubString(0, tracePrefixLength)
								   .Append(instr.Length() > tracePrefixLength
											   ? String("...").Append(instr.SubString(instr.Length() - traceSuffixLength))
											   : "")
							<< "]");
			String res, str(instr);
			eUrlCheckStatus = eOk;
			if (replacePlusByBlank) {
				Convert(str);  // converts browser inserted '+' to ' '
			}
			// Shortcut
			if (instr.StrChr('%') == -1L) {
				return replacePlusByBlank ? str : instr;
			}
			char c = 0;	 // current character
			long length(str.Length());
			if (length > 0) {
				for (long lPos = 0; lPos < length; ++lPos) {
					if ((c = str[lPos]) == '%') {
						// Escape: next 5 chars are %uxxxx representation of the actual character
						if ((c = str[lPos]) == '%' &&
							((lPos + 1L < length) && (str[lPos + 1L] == 'u' || str[lPos + 1L] == 'U'))) {
							if (((lPos + 5L < length) && (isxdigit(str[lPos + 2L]) != 0) && (isxdigit(str[lPos + 3L]) != 0) &&
								 (isxdigit(str[lPos + 4L]) != 0) && (isxdigit(str[lPos + 5L]) != 0))) {
								if (str[lPos + 2L] == '0' && (str[lPos + 3L]) == '0') {
									res.AppendTwoHexAsChar(&((const char *)str)[lPos + 4L]);
								} else {
									// char above FF
									eUrlCheckStatus = eSuspiciousChar;
								}
								lPos += 5L;
								continue;
							}
							c = DecodeSpecialChars(str, c, lPos, 2);
						} else {
							// Escape: next 2 chars are hex representation of the actual character
							if ((lPos + 1L < length) && (isxdigit(str[lPos + 1L]) != 0) && (lPos + 2L < length) &&
								(isxdigit(str[lPos + 2L]) != 0)) {
								res.AppendTwoHexAsChar(&((const char *)str)[lPos + 1L]);
								lPos += 2L;
								continue;
							}
							c = DecodeSpecialChars(str, c, lPos, 1);
						}
					}
					res.Append(c);
				}
			}
			return res;
		}

		String urlEncode(const String &str, const String &exclusionSet) {
			StartTrace(URLUtils.urlEncode);
			String encoded;
			DoUrlEncode(str, exclusionSet, encoded, false);
			return encoded;
		}

		bool CheckUrlEncoding(const String &str, const String &exclusionSet) {
			StartTrace(URLUtils.CheckUrlEncoding);
			String encoded;
			String exclusionSetModified(exclusionSet);
			exclusionSetModified.Append("%;?/");
			return DoUrlEncode(str, exclusionSetModified, encoded, true);
		}

		// Check URL for chars which should be encoded according to RFC1808
		bool CheckUrlArgEncoding(const String &str, const String &override) {
			StartTrace(URLUtils.CheckUrlArgEncoding);
			String base(
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789");
			String overrideDefault("%;/?:@&=$-_.+!*'(),");
			if (override.Length() == 0L) {
				base.Append(overrideDefault);
			} else {
				base.Append(override);
			}
			return (str.LastCharOf(base) == str.Length() || str.Length() == 0);
		}

		// Check URL contains only valid chars according to RFC1738
		bool CheckUrlPathContainsUnsafeChars(const String &str, const String &overrideUnsafe, const String &overrideAscii,
											 bool asciiExtended) {
			StartTrace(URLUtils.CheckUrlPathContainsUnsafeChars);
			String base("<>\"#%{}|\\^~[]`");
			if (overrideUnsafe.Length() > 0L) {
				base = overrideUnsafe;
			}
			long extendedCharIndex = str.ContainsCharAbove(127, overrideAscii);
			bool containsExtendedAscii = (asciiExtended && (extendedCharIndex != -1L));
			long unsaveCharIndex = str.FirstCharOf(base);
			bool containsUnsaveChar = (unsaveCharIndex != -1L);
			Trace("overrideAscii [" << overrideAscii << "] overrideUnsafe [" << base << "]");
			Trace("containsExtendedAscii: " << extendedCharIndex << " containsUnsaveChar: " << unsaveCharIndex);
			Trace("containsExtendedAscii: " << (containsExtendedAscii ? str.SubString(extendedCharIndex, 1) : "")
											<< " containsUnsaveChar: "
											<< (containsUnsaveChar ? str.SubString(unsaveCharIndex, 1) : ""));
			return (containsUnsaveChar || containsExtendedAscii);
		}

		// encode the given char *p into res by expanding problematic characters into %XX escapes suitable for msajax
		String MSUrlEncode(const String &str) {
			StartTrace(URLUtils.MSUrlEncode);
			String result;
			char c = 0;

			if (str.Length() > 0) {
				for (long l = 0; l < str.Length(); l++) {
					switch (c = str[l]) {
						case '\n':
							break;
						case ' ':
						case '"':
						case '%':
						case '&':
						case '?':
						case '/':
						case '\\':
						case '#':
						case '{':
						case '}':
						case '+':
						case '=':
							result.Append('%');
							result.AppendAsHex(c);
							break;
						default:
							result.Append(c);
							break;
					}
				}
			}
			return result;
		}

		void HandleURI(Anything &query, const String &uri) {
			StartTrace(URLUtils.HandleURI);
			// search for '?' defining a query
			long questMarkPos = uri.StrChr('?');
			String queryString;
			String pathString;

			if (questMarkPos != -1) {
				// its a query
				queryString = uri.SubString(questMarkPos + 1);
				pathString = uri.SubString(0, questMarkPos);
			} else {
				pathString = uri;
			}

			// remove first '/' from path expression
			if (pathString[0L] == '/') {
				pathString.TrimFront(1);
			}

			// prepare query anything
			Split(pathString, '/', query);
			DecodeAll(query);

			// prepare query anything
			if (queryString.Length() > 0) {
				// attention don't Decode the query twice
				Anything queryInfo;
				Split(queryString, '&', queryInfo);
				DecodeAll(queryInfo);
				for (long i = 0, sz = queryInfo.GetSize(); i < sz; ++i) {
					const char *slot = queryInfo.SlotName(i);
					if (slot != 0) {
						query[slot] = queryInfo[i];
					} else {
						query.Append(queryInfo[i]);
					}
				}
			}
		}

		void RemoveQuotes(String &str) {
			StatTrace(URLUtils.RemoveQuotes, "[" << str << "]", coast::storage::Current());
			// casts below needed for ANSI (VC++) conformance
			if ((str[0L] == '\'') && (str[(long)(str.Length() - 1)] == '\'')) {
				TrimChars(str, true, '\'');
				TrimChars(str, false, '\'');
			} else if (str[0L] == '"' && str[(long)(str.Length() - 1)] == '"') {
				TrimChars(str, true, '"');
				TrimChars(str, false, '"');
			}
		}

		void HandleURI2(Anything &query, const String &currentUri, const char *base) {
			// works with complete request uris that include http part etc.
			// "/abcdef" are returned as is - absolute paths
			// "abcdef" and "../abcdef" etc. strings are append to last"/" of existing Path - relative paths - no stripping of
			// ".." right now - although "real" browsers do this "http(s)://xxx/abcdef" strings are broken down into elements -
			// full URI
			StartTrace(URLUtils.HandleURI2);
			String uri(currentUri);
			String baseHREF(base);

			TraceAny(query, "Query at start") Trace("URI: " << uri << " : Base: " << baseHREF);

			RemoveQuotes(uri);
			RemoveQuotes(baseHREF);
			//--------------------------------------------------------------------------------------
			// handle BASE HREF - (KEM)
			// http://www.w3.org/TR/REC-html32:
			// The BASE element gives the base URL for dereferencing relative URLs, using the rules given by the URL
			// specification, e.g. <BASE href="http://www.acme.com/intro.html">
			//  ...
			// <IMG SRC="icons/logo.gif">
			// The image is deferenced to
			// http://www.acme.com/icons/logo.gif
			// In the absence of a BASE element the document URL should be used. Note that this is not necessarily the same as
			// the URL used to request the document, as the base URL may be overridden by an HTTP header accompanying the
			// document.
			//
			// Mike: baseHREF must always be a full URI, if not what do browsers do? ignore it??
			// That´s what we do here
			//--------------------------------------------------------------------------------------
			if (baseHREF.Length() > 0) {
				// baseHREF w/o http:// at start is ignored...
				if (baseHREF.Contains("://") > 0) {
					if (uri.Contains("://") < 0) {
						// does not contain "://, not a full URI, an absolute or relative URI
						if (uri.Contains("/") != 0) {
							String newURI = baseHREF;
							// added to last "/" in base URL
							long lastSlash = 0;
							if ((lastSlash = newURI.StrRChr('/')) > 0) {
								newURI.Trim(lastSlash + 1);
							}
							newURI << uri;
							uri = newURI;
						}
					}
				}
			}

			if (uri.Contains("://") < 0) {
				// does not contain "://, not a full URI, an absolute or relative URI
				if (uri.Contains("/") != 0) {
					// relative string
					String newPath = query["Path"].AsString();
					long Pos = newPath.StrRChr('/');
					if (Pos > 0) {
						newPath.Trim(Pos + 1);
					} else {  // can this ever happen?
						newPath = "/";
					}
					newPath << uri;
					query["Path"] = newPath;
				} else {
					// absolute string
					query["Path"] = uri;
				}
				TraceAny(query, "Query ");
				return;
			}

			Anything myTest1;
			HandleURI(myTest1, uri);
			//          this call delivers results of kind: {
			//				"https:"
			//	            "localhost:2023"
			//  	          "fda"
			//      	      /X
			//      "b64:YnMwOrtivW293ELBqhniXjVlDGsvi5N3k2eI$$bIM77lo2BprFMDPihqZX$6LjllTFHuVvS15YoihDnYPHagK-sVFJxx8gNV8o7HyaK5$RoW8E2vUGcXdfKRkwZmyxQ5WPPBknlPONT5MQim"
			//          }
			//
			int Pos = uri.Contains("//");

			if (Pos >= 0) {
				String myString1 = uri.SubString(0, Pos);

				int Pos2 = uri.StrChr('/', Pos + 2);
				if (Pos2 >= 0) {
					query["Path"] = uri.SubString(Pos2, uri.Length() - Pos2);
					Trace("Path contents are " << query["Path"].AsString(""));
				} else {
					query["Path"] = "/";
				}
			}

			String myProtocol = myTest1.At(0L).AsString("");
			// trim off last :
			myProtocol.Trim(myProtocol.StrChr(':'));
			myProtocol.ToUpper();
			query["Protocol"] = myProtocol;
			query["UseSSL"] = (myProtocol == "HTTPS");

			String domainAndPort = myTest1.At(1L).AsString("");
			String myServerStr = "";

			Pos = domainAndPort.StrChr(':');

			if (Pos >= 0) {
				myServerStr = domainAndPort.SubString(0, Pos);
				Trace("Server contents are " << myServerStr);
				query["Port"] = domainAndPort.SubString(Pos + 1, domainAndPort.Length() - Pos - 1);
				Trace("Port contents are " << query["Port"].AsString(""));
			} else {
				// No port in URL
				myServerStr = domainAndPort;
				Trace("Port contents are " << myServerStr);
				if (myProtocol.IsEqual("HTTP")) {
					// hard coded, ouch!
					query["Port"] = 80;
				} else {
					query["Port"] = 443;
				}
			}

			if (!myServerStr.IsEqual("localhost")) {
				// For gethostbyname - Form www.sbb.ch only, NOT localhost:2023 or
				// http://www.sbb.ch/pv/index_d.htm etc.
				query["Server"] = Resolver::DNS2IPAddress(myServerStr);
				query["ServerName"] = myServerStr;
			}

			TraceAny(query, "Query ");
		}

		String EncodeFormContent(Anything &kVPairs) {
			StartTrace(URLUtils.EncodeFormContent);
			TraceAny(kVPairs, "input key value pairs");
			String localString;
			for (int i = 0, sz = kVPairs.GetSize(); i < sz; ++i) {
				if (localString.Length() != 0)
					localString.Append('&');
				localString.Append(kVPairs.SlotName(i)).Append('=').Append(kVPairs[i].AsString());
			}
			Trace("kv-pairs [" << localString << "]");
			return localString;
		}

		String HTMLEscape(const String &toEscape) {
			StartTrace(URLUtils.HTMLEscape);
			String escPrefix("&#");
			String escPostfix(";");
			String escapedString;
			long length(toEscape.Length());
			for (long i = 0; i < length; ++i) {
				unsigned int work = (unsigned char)toEscape[i];
				if (isalnum(work) != 0) {
					escapedString.Append((char)work);
				} else {
					escapedString.Append(escPrefix);
					escapedString.Append(static_cast<long>(work));
					escapedString.Append(escPostfix);
				}
			}
			return escapedString;
		}

		String CleanUpUriPath(String name) {
			StartTrace(URLUtils.CleanUpUriPath);
			Trace("URLPath before: " << name);
			coast::system::ResolvePath(name);
			Trace("URLPath after: " << name);
			return name;
		}
	}  // namespace urlutils
}  // namespace coast
