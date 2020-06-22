/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RECompiler.h"

#include "RE.h"
#include "REBitSet.h"
// SOP: the following file is autogenerated by the testcases
// if something changes copy a new version from the Test subdir here
#include "Anything.h"
#include "REPosixBitSets.h"
#include "Tracer.h"

#include <cstring>

#include <ctype.h>

struct posixtable {
	posixtable(const char *pName, const REBitSet &aSet) : name(pName), set(aSet) {}
	const char *name;
	REBitSet set;
};
#if defined(WIN32)
static posixtable _posixtable[] = {
	{posixtable("alnum", gcSetIsAlnum)}, {posixtable("alpha", gcSetIsAlpha)}, {posixtable("blank", gcSetIsBlank)},
	{posixtable("cntrl", gcSetIsCntrl)}, {posixtable("digit", gcSetIsDigit)}, {posixtable("graph", gcSetIsGraph)},
	{posixtable("lower", gcSetIsLower)}, {posixtable("print", gcSetIsPrint)}, {posixtable("punct", gcSetIsPunct)},
	{posixtable("space", gcSetIsSpace)}, {posixtable("upper", gcSetIsUpper)}, {posixtable("xdigit", gcSetIsXDigit)}};
#else
static posixtable _posixtable[] = {
	posixtable("alnum", gcSetIsAlnum), posixtable("alpha", gcSetIsAlpha), posixtable("blank", gcSetIsBlank),
	posixtable("cntrl", gcSetIsCntrl), posixtable("digit", gcSetIsDigit), posixtable("graph", gcSetIsGraph),
	posixtable("lower", gcSetIsLower), posixtable("print", gcSetIsPrint), posixtable("punct", gcSetIsPunct),
	posixtable("space", gcSetIsSpace), posixtable("upper", gcSetIsUpper), posixtable("xdigit", gcSetIsXDigit)};
#endif
static long posixLookup(const String &cmp) {
	for (long i = 0; i < (long)(sizeof(_posixtable) / sizeof(struct posixtable)); ++i) {
		if (cmp == _posixtable[i].name) {
			return i;
		}
	}
	return -1;
}

// Special types of 'escapes' used within the translation
static const long ESC_MASK = 0xfff0;	 // Escape complexity mask
static const long ESC_BACKREF = 0xffff;	 // Escape is really a backreference
static const long ESC_COMPLEX = 0xfffe;	 // Escape isn't really a true character
static const long ESC_CLASS = 0xfffd;	 // Escape represents a whole class of characters
static const long ESC_ERROR = 0xfffc;	 // error in escape sequence (no exceptions)

void RECompiler::InsertNode(long opcode, long opdata, long insertAt) {
	StartTrace(RECompiler.InsertNode);
	for (long i = fInstruction.GetSize(); i > insertAt; --i) {
		fInstruction[i] = fInstruction[i - 1];
		fInstruction[i - 1] = Anything();  // otherwise reference semantic of AnyArrayImpl would kill value semantic needed here
	}

	fInstruction[insertAt][RE::offsetOpcode] = opcode;
	fInstruction[insertAt][RE::offsetOpdata] = opdata;
	fInstruction[insertAt][RE::offsetNext] = 0L;
}
void RECompiler::setNextOfEnd(long node, long pointTo) {
	StartTrace(RECompiler.setNextOfEnd);
	Assert(pointTo >= 0);
	// Traverse the chain until the next offset is 0
	long next = 0;
	while ((next = ROAnything(fInstruction)[node][RE::offsetNext].AsLong(0)) != 0) {
		node += next;
	}

	// Point the last node in the chain to pointTo.
	fInstruction[node][RE::offsetNext] = (long)(pointTo - node);
}

long RECompiler::AppendNode(long opcode, long opdata) {
	StartTrace(RECompiler.AppendNode);
	// Add new node at end
	Anything instr = Anything(Anything::ArrayMarker(), fInstruction.GetAllocator());
	instr[RE::offsetOpcode] = opcode;
	instr[RE::offsetOpdata] = opdata;
	instr[RE::offsetNext] = opcode == 0L;
	fInstruction.Append(instr);
	return fInstruction.GetSize() - 1;
}

void RECompiler::syntaxError(String s) {
	StartTrace(RECompiler.syntaxError);
	fErrorMsg << fPatternIdx << ":" << s;
	Trace(fErrorMsg);
}

long RECompiler::ReadNumber(long def) {
	StartTrace(RECompilerReadNumber);
	String number;
	// Next char must be a digit
	if (isdigit(fPattern.At(fPatternIdx)) == 0) {
		syntaxError("Expected digit");
		return def;
	}

	while (isdigit(fPattern.At(fPatternIdx)) != 0) {
		number.Append(fPattern.At(fPatternIdx++));
	}
	return number.AsLong(def);
}

static const int bracketUnbounded = -1;

bool RECompiler::bracket(long &min, long &optional) {
	StartTrace(RECompiler.bracket);
	min = -1;
	optional = 0;
	// Current character must be a '{'
	if (fPattern.At(fPatternIdx++) != '{') {  // } }
		Assert(fPattern.At(fPatternIdx++) == '{');
		return false;
	}
	// Get min ('m' of {m,n}) number
	min = ReadNumber(1);
	// If end of expr, optional limit is 0 {
	if (fPattern.At(fPatternIdx) == '}') {
		++fPatternIdx;
		return true;
	}
	// Must have at least {m,} and maybe {m,n}.
	if (fPattern.At(fPatternIdx++) != ',') {
		syntaxError("Expected comma");
		return false;
	}
	// If {m,} max is unlimited {
	if (fPattern.At(fPatternIdx) == '}') {
		++fPatternIdx;
		optional = bracketUnbounded;
		return true;
	}
	optional = ReadNumber(min) - min;
	// Optional repetitions must be >= 0
	if (optional < 0 || min < 0) {
		syntaxError("Bad range");
		return false;
	}

	// Must have close brace {
	if (fPattern.At(fPatternIdx++) != '}') {
		syntaxError("Missing close brace");
		return false;
	}
	return true;
}

/**
 * Match an escape sequence.  Handles quoted chars and octal escapes as well
 * as normal escape characters.  Always advances the input stream by the
 * right amount. This code "understands" the subtle difference between an
 * octal escape and a backref.  You can access the type of ESC_CLASS or
 * ESC_COMPLEX or ESC_BACKREF by looking at fPattern[fPatternIdx - 1].
 */
long RECompiler::escape() {
	StartTrace(RECompiler.escape);
	// "Shouldn't" happen
	Assert(fPattern.At(fPatternIdx) == '\\');

	// Escape shouldn't occur as last character in string!
	if (fPatternIdx + 1 >= fPattern.Length()) {
		syntaxError("Escape terminates string");
		return ESC_ERROR;
	}

	// Switch on character after backslash
	fPatternIdx += 2;
	char escapeChar = fPattern.At(fPatternIdx - 1);
	switch (escapeChar) {
		case RE::E_BOUND:
		case RE::E_NBOUND:
			return ESC_COMPLEX;

		case RE::E_ALNUM:
		case RE::E_NALNUM:
		case RE::E_SPACE:
		case RE::E_NSPACE:
		case RE::E_DIGIT:
		case RE::E_NDIGIT:
			return ESC_CLASS;

			// case 'u': //SOP: we do not support unicode only 8bit chars
		case 'x': {
			// Exact required hex digits for escape type
			long hexDigits = 2;	 //(escapeChar == 'u' ? 4 : 2);

			// Parse up to hexDigits characters from input
			long val = 0;
			for (; fPatternIdx < fPattern.Length() && hexDigits-- > 0; fPatternIdx++) {
				// Get char
				char c = fPattern.At(fPatternIdx);

				// If it's a hexadecimal digit (0-9)
				if (c >= '0' && c <= '9') {
					// Compute new value
					val = (val << 4) + c - '0';
				} else {
					// If it's a hexadecimal letter (a-f)
					c = tolower(c);
					if (c >= 'a' && c <= 'f') {
						// Compute new value
						val = (val << 4) + (c - 'a') + 10;
					} else {
						// If it's not a valid digit or hex letter, the escape must be invalid
						// because hexDigits of input have not been absorbed yet.
						syntaxError(String("Expected ") << hexDigits << " hexadecimal digits after \\" << escapeChar);
						return ESC_ERROR;
					}
				}
			}
			return val;
		}

		case 't':
			return '\t';

		case 'n':
			return '\n';

		case 'r':
			return '\r';

		case 'f':
			return '\f';

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
//		case '8':
//		case '9':
#define isodigit(c) ('0' <= (c) && '7' >= (c))
			// An octal escape starts with a 0 or has two OCTAL (SOP) digits in a row
			if (isodigit(fPattern.At(fPatternIdx)) || escapeChar == '0') {
				// Handle \nnn octal escapes
				long val = escapeChar - '0';
				if (isodigit(fPattern.At(fPatternIdx))) {
					val = ((val << 3) + (fPattern.At(fPatternIdx++) - '0'));
					if (isodigit(fPattern.At(fPatternIdx))) {
						val = ((val << 3) + (fPattern.At(fPatternIdx++) - '0'));
					}
				}
				return val;
			}
#undef isodigit
		case '8':
		case '9':

			// It's actually a backreference (\[1-9]), not an escape
			return ESC_BACKREF;

		default:

			// Simple quoting of a character
			return escapeChar;
	}
}

long RECompiler::PosixCharacterClass() {
	StartTrace(RECompiler.PosixCharacterClass);
	// POSIX character classes are denoted with lowercase ASCII strings
	int idxStart = fPatternIdx;
	while (islower(fPattern.At(fPatternIdx)) != 0) {
		++fPatternIdx;
	}

	// Should be a ":]" to terminate the POSIX character class
	if (fPattern.At(fPatternIdx) == ':' && fPattern.At(fPatternIdx + 1) == ']') {
		// Get character class
		String charClass = fPattern.SubString(idxStart, fPatternIdx - idxStart);

		// Select the POSIX class id
		long i = posixLookup(charClass);  // hashPOSIX.get(charClass);
		if (i >= 0) {
			// Move past colon and right bracket
			fPatternIdx += 2;
			int ret = AppendNode(RE::OP_ANYOF, 0);
			fInstruction[ret][RE::offsetOpdata] = Anything((void *)&(_posixtable[i].set), sizeof(REBitSet));

			return ret;
		}
		syntaxError(String("Invalid POSIX character class '") << charClass << "'");
	}
	syntaxError("Invalid POSIX character class syntax");
	return -1;
}
long RECompiler::characterClass() {
	StartTrace(RECompiler.characterClass);
	// Check for bad calling or empty class
	if (fPattern.At(fPatternIdx) != '[') {
		Assert(fPattern.At(fPatternIdx) == '[');
		return -1;
	}
	++fPatternIdx;
	// Check for unterminated //SOP: not possible or empty class
	if ((fPatternIdx) >= fPattern.Length()) {  // || fPattern.At(++fPatternIdx) == ']')
		syntaxError("Unterminated class");
		return -1;
	}

	// Check for POSIX character class
	if (fPattern.At(fPatternIdx) == ':') {
		// Skip colon
		++fPatternIdx;
		return PosixCharacterClass();
	}

	// Try to build a class.  Create OP_ANYOF node
	int ret = AppendNode(RE::OP_ANYOF, 0);

	// Parse class declaration
	unsigned char CHAR_INVALID = 255;  // Character.MAX_VALUE;
	unsigned char last = CHAR_INVALID;
	unsigned char simpleChar = 0;
	bool include = true;
	bool definingRange = false;
	int idxFirst = fPatternIdx;
	unsigned char rangeStart = 0;  // Character.MIN_VALUE;
	unsigned char rangeEnd = 0;
	REBitSet range;
	while (fPattern.Length() > fPatternIdx) {  //(fPattern.At(fPatternIdx) != ']')

		char ch = fPattern.At(fPatternIdx);
		// Switch on character
		switch (ch) {
			case ']':
				if (fPatternIdx == idxFirst || (fPatternIdx == idxFirst + 1 && fPattern.At(idxFirst) == '^')) {
					simpleChar = ch;
					++fPatternIdx;
					break;
				} else {
					// would be nice to have break 2,
					goto endClass;
				}
			case '^':
				if (fPatternIdx == idxFirst) {
					include = !include;
					++fPatternIdx;
					continue;
				} else {
					simpleChar = ch;
				}
				break;
			case '\\': {
				// Escape always advances the stream
				long c = 0;
				switch (c = escape()) {
					case ESC_ERROR:
					case ESC_COMPLEX:
					case ESC_BACKREF:
						// Word boundaries and backrefs not allowed in a character class!
						syntaxError("Bad character class");
						return -1;
					case ESC_CLASS:
						// Classes can't be an endpoint of a range
						if (definingRange) {
							syntaxError("Bad character class");
							return -1;
						}
						// Handle specific type of class (some are ok)
						switch (fPattern.At(fPatternIdx - 1)) {
							case RE::E_NSPACE:	// \S
							case RE::E_NDIGIT:	// \D
							case RE::E_NALNUM:	// \W
								syntaxError("Bad character class");
								return -1;
							case RE::E_SPACE:  // \s
								range |= gcSetIsSpace;
								break;

							case RE::E_ALNUM:  // \w
								range |= gcSetIsAlnum;
								break;
							case RE::E_DIGIT:  // \d
								range |= gcSetIsDigit;
								break;
						}
						// Make last char invalid (can't be a range start)
						last = CHAR_INVALID;
						break;

					default:
						// Escape is simple so treat as a simple char
						simpleChar = c;
						goto switchOnCharacter;
				}
			}
				continue;

			case '-':

				// Start a range if one isn't already started
				if (definingRange || last == CHAR_INVALID) {
					// make the range end with '-' or treat it as normal char
					simpleChar = '-';
					++fPatternIdx;
					break;
				}
				definingRange = true;

				// If no last character, start of range is 0
				rangeStart = last;
				// SOP: this seems to be wrong, most systems define - at the end
				// of a set expr to include - in the set
				if ((fPatternIdx + 1) < fPattern.Length() && fPattern.At(++fPatternIdx) == ']') {
					definingRange = false;	// SOP
					simpleChar = '-';		// SOP
					break;
				}
				continue;

			default:
				simpleChar = fPattern.At(fPatternIdx++);
				break;
		}
	switchOnCharacter:

		// Handle simple character simpleChar
		if (definingRange) {
			// if we are defining a range make it now
			rangeEnd = simpleChar;

			// Actually create a range if the range is ok
			if (rangeStart >= rangeEnd) {
				syntaxError("Bad character class");
				return -1;
			}
			range.Set(rangeStart, rangeEnd);

			// We are done defining the range
			last = CHAR_INVALID;
			definingRange = false;
		} else {
			// If simple character and not start of range, include it
			if (fPattern.At(fPatternIdx + 1) != '-') {
				range.Set(simpleChar);
			}
			last = simpleChar;
		}
	}
endClass:
	// Shouldn't be out of input
	if (fPatternIdx >= fPattern.Length()) {
		syntaxError("Unterminated character class");
		return -1;
	}
	if (!include) {
		range.Complement();
	}
	// Absorb the ']' end of class marker
	++fPatternIdx;

	// Emit character class definition
	fInstruction[ret][RE::offsetOpdata] = Anything((void *)&range, sizeof(range));

	return ret;
}

/**
 * Absorb an atomic character string.  This method is a little tricky because
 * it can un-include the last character of string if a closure operator follows.
 * This is correct because *+? have higher precedence than concatentation (thus
 * ABC* means AB(C*) and NOT (ABC)*).
 */
long RECompiler::atom() {
	StartTrace(RECompiler.atom);
	// Create a string node
	int ret = AppendNode(RE::OP_ATOM, 0);
	String strAtom;

	while (fPatternIdx < fPattern.Length()) {
		// Is there a next char?
		if ((fPatternIdx + 1) < fPattern.Length()) {
			char c = fPattern.At(fPatternIdx + 1);

			// If the next 'char' is an escape, look past the whole escape
			if (fPattern.At(fPatternIdx) == '\\') {
				int idxEscape = fPatternIdx;
				if (0 > escape()) {
					return -1;
				}
				if (fPatternIdx < fPattern.Length()) {
					c = fPattern.At(fPatternIdx);
				}
				fPatternIdx = idxEscape;
			}

			// Switch on next char
			switch (c) {
				case '{':  //}
				case '?':
				case '*':
				case '+':

					// If the next character is a closure operator and our atom is non-empty, the
					// current character should bind to the closure operator rather than the atom
					if (strAtom.Length() != 0) {
						goto atomLoopExit;
					}
			}
		}

		// Switch on current char
		switch (fPattern.At(fPatternIdx)) {
				//			case ']':
			case '^':
			case '$':
			case '.':
			case '[':
			case '(':
			case ')':
			case '|':
				goto atomLoopExit;

			case '{':  //}
			case '?':
			case '*':
			case '+':

				// We should have an atom by now
				if (strAtom.Length() == 0) {
					// No atom before closure
					syntaxError("Missing operand to closure");
					return -1;
				}
				goto atomLoopExit;

			case '\\':

			{
				// Get the escaped character (advances input automatically)
				int idxBeforeEscape = fPatternIdx;
				long c = escape();

				// Check if it's a simple escape (as opposed to, say, a backreference)
				if ((c & ESC_MASK) == ESC_MASK) {
					// Not a simple escape, so backup to where we were before the escape.
					fPatternIdx = idxBeforeEscape;
					goto atomLoopExit;
				}

				// Add escaped char to atom
				strAtom.Append((char)c);
			} break;

			default:

				// Add normal character to atom
				strAtom.Append(fPattern.At(fPatternIdx++));
				break;
		}
	}
atomLoopExit:

	// This "shouldn't" happen
	if (strAtom.Length() == 0) {
		Assert(strAtom.Length() > 0);
		return -1;
	}

	fInstruction[ret][RE::offsetOpdata] = strAtom;
	return ret;
}

long RECompiler::terminal(bool &isnullable) {
	StartTrace(RECompiler.terminal);
	switch (fPattern.At(fPatternIdx)) {
		case RE::OP_EOL:
		case RE::OP_BOL:
			// SOP: these are tricky if they are lonely, they do not advance input
			// this can lead to endless loops if they will be operands to repetition
			isnullable = true;
		case RE::OP_ANY:
			return AppendNode(fPattern.At(fPatternIdx++), 0);

		case '[':
			return characterClass();

		case '(':
			return expr(isnullable);

		case ')':
			syntaxError("Unexpected close paren");
			return -1;

		case '|':  // internal error
			Assert(fPattern.At(fPatternIdx) != '|');
			return -1;

		case 0:
			syntaxError("Unexpected end of input");
			return -1;
		case '?':
		case '+':
		case '{':
		case '*':
			syntaxError("Missing operand to closure");
			return -1;
		case '\\': {
			// Don't forget, escape() advances the input stream!
			int idxBeforeEscape = fPatternIdx;

			// Switch on escaped character
			switch (escape()) {
				case ESC_CLASS: {
					isnullable = false;

					REBitSet range;
					switch (fPattern.At(fPatternIdx - 1)) {
						case RE::E_ALNUM:
							range = gcSetIsAlnum;
							break;
						case RE::E_NALNUM:
							range = gcSetIsNoAlnum;
							break;
						case RE::E_SPACE:
							range = gcSetIsSpace;
							break;
						case RE::E_NSPACE:
							range = gcSetIsNoSpace;
							break;
						case RE::E_DIGIT:
							range = gcSetIsDigit;
							break;
						case RE::E_NDIGIT:
							range = gcSetIsNoDigit;
					}
					long ret = AppendNode(RE::OP_ANYOF, 0);
					fInstruction[ret][RE::offsetOpdata] = Anything((void *)&range, sizeof(range));
					return ret;
				}
				case ESC_COMPLEX:
					isnullable = true;
					return AppendNode(RE::OP_BOW, static_cast<long>(RE::E_BOUND == fPattern.At(fPatternIdx - 1)));

				case ESC_BACKREF: {
					long backreference = (fPattern.At(fPatternIdx - 1) - '0');
					if (0 == (fMaskOfClosedParentheses & (1 << backreference))) {
						syntaxError("Bad backreference");
						return -1;
					}
					isnullable = true;
					return AppendNode(RE::OP_BACKREF, backreference);
				}
				case ESC_ERROR:
					return -1;

				default:

					// We had a simple escape and we want to have it end up in
					// an atom, so we back up and fall though to the default handling
					fPatternIdx = idxBeforeEscape;
					isnullable = false;
					break;
			}
		}
	}

	// Everything above either fails or returns.
	// If it wasn't one of the above, it must be the start of an atom.
	isnullable = false;
	return atom();
}

Anything RECompiler::CopyProgram(long from, long len) {
	StartTrace(RECompiler.CopyProgram);
	Anything res = Anything(Anything::ArrayMarker(), fInstruction.GetAllocator());

	for (long i = from; i < from + len; ++i) {
		res.Append(fInstruction[i].DeepClone());
	}
	return res;
}

long RECompiler::AppendProgram(Anything prog) {
	StartTrace(RECompiler.AppendProgram);
	long res = fInstruction.GetSize();
	long len = prog.GetSize();
	Assert(len > 0);
	for (long i = 0; i < len; ++i) {
		Assert(prog[i].GetSize() >= 3);
		fInstruction.Append(prog[i].DeepClone());
	}
	return res;
}

void RECompiler::GenerateQuestions(Anything prog, long terminalNode, long repetitions) {
	StartTrace(RECompiler.GenerateQuestions);
	long terminalSize = prog.GetSize();
	Assert(terminalSize > 0);
	(void)terminalSize;
	GenerateQuestion(terminalNode);
	// now the original terminal starts at terminalNode + RE::nodesize
	long newTerminal = terminalNode;
	while (--repetitions > 0) {
		long chain = newTerminal;
		newTerminal = AppendProgram(prog);
		GenerateQuestion(newTerminal);
		setNextOfEnd(chain, newTerminal);  // concatenate stuff
	}
}
void RECompiler::GenerateQuestion(long terminalNode) {
	StartTrace(RECompiler.GenerateQuestion);
	// X? is compiled as (X|), should be just X, if X is nullable
	InsertNode(RE::OP_BRANCH, 0, terminalNode);				   // branch before X
	setNextOfEnd(terminalNode, AppendNode(RE::OP_BRANCH, 0));  // inserted branch to option
	int nothing = AppendNode(RE::OP_NOTHING, 0);			   // which is OP_NOTHING
	setNextOfEnd(terminalNode, nothing);					   // point (second) branch to OP_NOTHING
	setNextOfEnd(terminalNode + RE::nodeSize, nothing);		   // point the end of X to OP_NOTHING node
}
void RECompiler::GenerateStar(long terminalNode) {
	StartTrace(RECompiler.GenerateStar);
	// X* is compiled as (X{gotoX}|)
	InsertNode(RE::OP_BRANCH, 0, terminalNode);								  // branch before X
	setNextOfEnd(terminalNode + RE::nodeSize, AppendNode(RE::OP_BRANCH, 0));  // end of X points to an option
	setNextOfEnd(terminalNode + RE::nodeSize, AppendNode(RE::OP_GOTO, 0));	  // to goto
	setNextOfEnd(terminalNode + RE::nodeSize, terminalNode);				  // the start again
	setNextOfEnd(terminalNode, AppendNode(RE::OP_BRANCH, 0));				  // the other option is
	setNextOfEnd(terminalNode, AppendNode(RE::OP_NOTHING, 0));				  // OP_NOTHING
}
void RECompiler::GeneratePlus(long terminalNode) {
	StartTrace(RECompiler.GeneratePlus);
	// X+ is compiled as X({gotoX}|)
	long branch = AppendNode(RE::OP_BRANCH, 0);					// a new branch
	setNextOfEnd(terminalNode, branch);							// is added to the end of X
	setNextOfEnd(AppendNode(RE::OP_GOTO, 0), terminalNode);		// one option is to go back to the start
	setNextOfEnd(branch, AppendNode(RE::OP_BRANCH, 0));			// the other option
	setNextOfEnd(terminalNode, AppendNode(RE::OP_NOTHING, 0));	// is OP_NOTHING
}

long RECompiler::RepeatTerminal(Anything prog, long terminalNode, long repetitions) {
	StartTrace(RECompiler.RepeatTerminal);
	long last = terminalNode;
	while (repetitions > 0) {
		long chain = last;
		last = AppendProgram(prog);
		setNextOfEnd(chain, last);	// concatenate stuff
		--repetitions;
	}
	return last;
}
void RECompiler::RemoveTerminal(long terminalNode, long terminalEnd) {
	StartTrace(RECompiler.RemoveTerminal);
	Assert(terminalEnd == fInstruction.GetSize());
	while (--terminalEnd >= terminalNode) {
		fInstruction.Remove(terminalEnd);
	}
	Assert(terminalNode == fInstruction.GetSize());
}

long RECompiler::closure(bool &isnullable) {
	StartTrace(RECompiler.closure);
	// Values to pass by reference to terminal()
	bool terminalIsNullable = false;

	// Get terminal symbol
	long terminalNode = terminal(terminalIsNullable);
	long terminalLen = fInstruction.GetSize() - terminalNode;
	// Or in flags from terminal symbol
	isnullable = terminalIsNullable;

	if (fPatternIdx >= fPattern.Length() || terminalNode < 0) {
		return terminalNode;
	}
	bool greedy = true;
	char closureType = fPattern.At(fPatternIdx);
	switch (closureType) {
		case '?':
		case '*':
			isnullable = true;
		case '+':
			// Eat closure character
			++fPatternIdx;
		case '{':  // }
			if (terminalIsNullable) {
				syntaxError("Closure operand can't be nullable");
				return -1;
			}
			break;
	}

	// If the next character is a '?', make the closure non-greedy (reluctant)
	if (fPattern.At(fPatternIdx) == '?') {
		++fPatternIdx;
		greedy = false;
	}

	if (greedy) {
		// Actually do the closure now
		switch (closureType) {
			case '{': {	 //}
				long minimum = 0, optionalRepetition = 0;
				if (!bracket(minimum, optionalRepetition)) {
					return -1;
				}

				Anything terminalProg = CopyProgram(terminalNode, terminalLen);
				long lastTerminalNode = terminalNode;
				if (minimum > 1) {
					lastTerminalNode = RepeatTerminal(terminalProg, lastTerminalNode, minimum - 1);
					Assert(lastTerminalNode == terminalNode + (minimum - 1) * terminalLen);
				} else if (minimum == 0) {
					isnullable = true;
				}
				switch (optionalRepetition) {
					case bracketUnbounded:	//{n,}
						if (minimum > 0) {
							lastTerminalNode = RepeatTerminal(terminalProg, lastTerminalNode, 1);
						}
						GenerateStar(lastTerminalNode);
						break;
					case 0:	 // check for pathological case {0} or {0,0}
						if (minimum <= 0) {
							RemoveTerminal(terminalNode, terminalNode + terminalLen);
							syntaxError("repetition {0} or {0,0}");
							Assert(fInstruction.GetSize() == terminalNode);
							return -1;	// check if this is OK
						}
						break;
					default:
						Assert(optionalRepetition > 0);
						if (minimum > 0) {	// do this for the normal repetition
							lastTerminalNode = RepeatTerminal(terminalProg, lastTerminalNode, 1);
						}
						GenerateQuestions(terminalProg, lastTerminalNode, optionalRepetition);
						break;
				}
			}
				return terminalNode;
			case '?':
				GenerateQuestion(terminalNode);
				break;
			case '*':
				GenerateStar(terminalNode);
				break;
			case '+':
				GeneratePlus(terminalNode);
				break;
		}
	} else {
		// Add end after closured subexpr
		setNextOfEnd(terminalNode, AppendNode(RE::OP_END, 0));

		// Actually do the closure now
		switch (closureType) {
			case '?':
				InsertNode(RE::OP_RELUCTANTMAYBE, 0, terminalNode);
				break;

			case '*':
				InsertNode(RE::OP_RELUCTANTSTAR, 0, terminalNode);
				break;

			case '+':
				InsertNode(RE::OP_RELUCTANTPLUS, 0, terminalNode);
				break;
		}

		// Point to the expr after the closure
		setNextOfEnd(terminalNode, fInstruction.GetSize());
	}
	return terminalNode;
}

/**
 * Compile one branch of an or operator (implements concatenation)
 */
long RECompiler::Branch(bool &isnullable) {
	StartTrace(RECompiler.Branch);
	// Get each possibly closured piece and concat
	long ret = AppendNode(RE::OP_BRANCH, 0);
	long chain = -1;
	isnullable = true;
	while (fPatternIdx < fPattern.Length() && fPattern.At(fPatternIdx) != '|' && fPattern.At(fPatternIdx) != ')') {
		// Get new node
		bool closureIsNullable = false;
		long node = closure(closureIsNullable);
		if (node < 0) {
			return node;
		}
		if (!closureIsNullable) {
			isnullable = false;
		}
		if (chain != -1) {
			setNextOfEnd(chain, node);
		}
		chain = node;
	}
	// If we don't run loop, make a nothing node
	if (chain == -1) {
		AppendNode(RE::OP_NOTHING, 0);
	}
	return ret;
}

long RECompiler::expr(bool &isnullable) {
	StartTrace(RECompiler.expr);
	// Create open paren node unless we were called from the top level
	// (which has no fNofParentheses)
	bool paren = false;
	bool branchnullable = false;
	long ret = -1;
	long closeParens = fNofParentheses;
	if (fNofParentheses < 1) {
		fNofParentheses = 1;
	} else if (fPattern.At(fPatternIdx) == '(') {
		++fPatternIdx;
		paren = true;
		ret = AppendNode(RE::OP_OPEN, fNofParentheses++);
	}

	// Create a branch node
	long branch = Branch(branchnullable);
	if (branch < 0) {
		return branch;
	}
	if (ret == -1) {
		ret = branch;
	} else {
		setNextOfEnd(ret, branch);
	}
	isnullable = isnullable || branchnullable;
	// Loop through branches
	while (fPatternIdx < fPattern.Length() && fPattern.At(fPatternIdx) == '|') {
		++fPatternIdx;
		branch = this->Branch(branchnullable);
		if (branch < 0) {
			return branch;
		}
		setNextOfEnd(ret, branch);
		isnullable = isnullable || branchnullable;
	}

	long end = 0;
	if (paren) {
		if (fPattern.At(fPatternIdx) == ')') {
			++fPatternIdx;
		} else {
			syntaxError("Missing close paren");
			return -1;
		}
		fMaskOfClosedParentheses |= (1L << closeParens);
		end = AppendNode(RE::OP_CLOSE, closeParens);
	} else {
		end = AppendNode(RE::OP_END, 0);
	}

	// Append the ending node to the ret nodelist
	setNextOfEnd(ret, end);

	// Hook the ends of each branch to the end node
	for (long next = -1, i = ret; next != 0; next = fInstruction[i][RE::offsetNext].AsLong(0), i += next) {
		// If branch, make the end of the branch's operand chain point to the end node.
		if (fInstruction[i][RE::offsetOpcode].AsLong(-1) == RE::OP_BRANCH) {
			setNextOfEnd(i + 1, end);
		}
	}

	// Return the node list
	return ret;
}
Anything RECompiler::compile(const String &pat) {
	StartTrace(RECompiler.compile);
	// Initialize fields for compilation
	fErrorMsg = "";
	fPattern = pat;										// Save fPattern in instance variable
	fPatternIdx = 0;									// Set parsing index to the first character
	fNofParentheses = 0, fMaskOfClosedParentheses = 0;	// Set paren level to 1 (the implicit outer fNofParentheses)
	fInstruction = Anything(Anything::ArrayMarker());

	Trace("fPattern:" << fPattern);

	Anything res;
	// Initialize pass by reference flags value
	bool isnullable = false;
	if (0 <= expr(isnullable)) {
		// Should be at end of input
		if (fPatternIdx == fPattern.Length()) {
			String prefix;
			if (fInstruction.GetSize() > 0) {
				// If the first node is a branch
				if (fInstruction[0L][RE::offsetOpcode].AsLong(0) == RE::OP_BRANCH) {
					// to the end node
					// and the branch starts with an atom
					long next = fInstruction[0L][RE::offsetNext].AsLong(0);
					if (fInstruction[next][RE::offsetOpcode].AsLong(0) == RE::OP_END &&
						fInstruction[1L][RE::offsetOpcode].AsLong(0) == RE::OP_ATOM) {
						// then get that atom as an prefix because there's no other choice
						res["prefix"] = fInstruction[1][RE::offsetOpdata];
					}
				}
			}
			res["program"] = fInstruction;

		} else {
			if (fPattern.At(fPatternIdx) == ')') {
				syntaxError("Unmatched close paren");
			}
			syntaxError("Unexpected input remains");
		}
	}
	return res;
}
