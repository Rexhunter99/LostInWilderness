/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 *
 * Copyright (C) 2009 Pur3 Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* For a verison history, please see "TINYJS_HISTORY" log

    NOTE:
          Constructing an array with an initial length 'Array(5)' doesn't work
          Recursive loops of data such as a.foo = a; fail to be garbage collected
          length variable cannot be set
          The postfix increment operator returns the current value, not the previous as it should.
          There is no prefix increment operator
          Arrays are implemented as a linked list - hence a lookup time is O(n)

    TODO:
          Utility va-args style function in TinyJS for executing a function directly
          Merge the parsing of expressions/statements so eval("statement") works like we'd expect.
          Move 'shift' implementation into mathsOp

 */

#include "TinyJS.h"
#include "TinyJS_Lexer.h"

/* Frees the given link IF it isn't owned by anything else */
#define CLEAN(x) { VarLink *__v = x; if (__v && !__v->owned) { delete __v; } }
/* Create a LINK to point to VAR and free the old link.
 * BUT this is more clever - it tries to keep the old link if it's not owned to save allocations */
#define CREATE_LINK(LINK, VAR) { if (!LINK || LINK->owned) LINK = new VarLink(VAR); else LINK->replaceWith(VAR); }

#include <sstream>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#define ASSERT(X) assert(X)

using namespace std;
using namespace JS;

/*replacing or redefining placement new is not allowed; will throw compiler error*/
//#ifdef _WIN32
//#	ifdef _DEBUG
//#		ifndef DBG_NEW
//#			define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#			define new DBG_NEW
//#		endif
//#	endif
//#endif

#ifdef __GNUC__
#	define vsprintf_s vsnprintf
#	define sprintf_s snprintf
#	define _strdup strdup
#endif

// ----------------------------------------------------------------------------------- Memory Debug

#define DEBUG_MEMORY 0

#if DEBUG_MEMORY

vector<Var *> allocatedVars;
vector<VarLink *> allocatedLinks;

void mark_allocated(Var * v)
{
	allocatedVars.push_back(v);
}

void mark_deallocated(Var * v)
{
	for (size_t i=0; i<allocatedVars.size(); i++)
	{
		if (allocatedVars[i] == v)
		{
			allocatedVars.erase(allocatedVars.begin()+i);
			break;
		}
	}
}

void mark_allocated(VarLink * v)
{
	allocatedLinks.push_back(v);
}

void mark_deallocated(VarLink * v)
{
	for (size_t i=0; i<allocatedLinks.size(); i++)
	{
		if (allocatedLinks[i] == v)
		{
			allocatedLinks.erase(allocatedLinks.begin()+i);
			break;
		}
	}
}

void show_allocated()
{
	for (size_t i=0; i<allocatedVars.size(); i++)
	{
		printf("ALLOCATED, %d refs\n", allocatedVars[i]->getRefs());
		allocatedVars[i]->trace("  ");
	}
	for (size_t i=0; i<allocatedLinks.size(); i++)
	{
		printf("ALLOCATED LINK %s, allocated[%d] to \n", allocatedLinks[i]->name.c_str(), allocatedLinks[i]->var->getRefs());
		allocatedLinks[i]->var->trace("  ");
	}
	allocatedVars.clear();
	allocatedLinks.clear();
}
#endif

// ----------------------------------------------------------------------------------- Utils
bool isWhitespace(char ch)
{
	return (ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r');
}

bool isNumeric(char ch)
{
	return (ch>='0') && (ch<='9');
}
bool isNumber(const string & str)
{
	for (size_t i=0; i<str.size(); i++)
		if (!isNumeric(str[i])) return false;
	return true;
}
bool isHexadecimal(char ch)
{
	return ((ch>='0') && (ch<='9')) ||
		   ((ch>='a') && (ch<='f')) ||
		   ((ch>='A') && (ch<='F'));
}
bool isAlpha(char ch)
{
	return ((ch>='a') && (ch<='z')) || ((ch>='A') && (ch<='Z')) || ch=='_';
}

bool isIDString(const char * s)
{
	if (!isAlpha(*s))
		return false;
	while (*s)
	{
		if (!(isAlpha(*s) || isNumeric(*s)))
			return false;
		s++;
	}
	return true;
}

void replace(string & str, char textFrom, const char * textTo)
{
	int sLen = strlen(textTo);
	size_t p = str.find(textFrom);
	while (p != string::npos)
	{
		str = str.substr(0, p) + textTo + str.substr(p+1);
		p = str.find(textFrom, p+sLen);
	}
}

///<convert the given string into a quoted string suitable for javascript
std::string JS::getJSString(const std::string & str)
{
	std::string nStr = str;
	for (size_t i=0; i<nStr.size(); i++)
	{
		const char * replaceWith = "";
		bool replace = true;

		switch (nStr[i])
		{
		case '\\':
			replaceWith = "\\\\";
			break;
		case '\n':
			replaceWith = "\\n";
			break;
		case '\r':
			replaceWith = "\\r";
			break;
		case '\a':
			replaceWith = "\\a";
			break;
		case '"':
			replaceWith = "\\\"";
			break;
		default:
		{
			int nCh = ((int)nStr[i]) &0xFF;
			if (nCh<32 || nCh>127)
			{
				char buffer[5];
				sprintf_s(buffer, 5, "\\x%02X", nCh);
				replaceWith = buffer;
			}
			else replace=false;
		}
		}

		if (replace)
		{
			nStr = nStr.substr(0, i) + replaceWith + nStr.substr(i+1);
			i += strlen(replaceWith)-1;
		}
	}
	return "\"" + nStr + "\"";
}

/** Is the string alphanumeric */
bool isAlphaNum(const std::string & str)
{
	if (str.size()==0) return true;
	if (!isAlpha(str[0])) return false;
	for (size_t i=0; i<str.size(); i++)
		if (!(isAlpha(str[i]) || isNumeric(str[i])))
			return false;
	return true;
}

// ----------------------------------------------------------------------------------- Exception

Exception::Exception(const string & exceptionText)
{
	text = exceptionText;
}

const string & Exception::what() const
{
	return this->text;
}

// ----------------------------------------------------------------------------------- Lexer

Lexer::Lexer(const string & input)
{
	data = _strdup(input.c_str());
	dataOwned = true;
	dataStart = 0;
	dataEnd = strlen(data);
	reset();
}

Lexer::Lexer(Lexer * owner, int startChar, int endChar)
{
	data = owner->data;
	dataOwned = false;
	dataStart = startChar;
	dataEnd = endChar;
	reset();
}

Lexer::~Lexer(void)
{
	if (dataOwned)
		free((void *)data);
}

void Lexer::reset()
{
	dataPos = dataStart;
	tokenStart = 0;
	tokenEnd = 0;
	tokenLastEnd = 0;
	tk = 0;
	tkStr = "";
	getNextCh();
	getNextCh();
	getNextToken();
}

void Lexer::match(int expected_tk)
{
	if (tk!=expected_tk)
	{
		ostringstream errorString;
		errorString << "Got " << getTokenStr(tk) << " expected " << getTokenStr(expected_tk)
					<< " at " << getPosition(tokenStart);
		throw new Exception(errorString.str());
	}
	getNextToken();
}

string Lexer::getTokenStr(int token)
{
	if (token>32 && token<128)
	{
		char buf[4] = "' '";
		buf[1] = (char)token;
		return buf;
	}
	switch (token)
	{
	case LEX_EOF :
		return "EOF";
	case LEX_ID :
		return "ID";
	case LEX_INT :
		return "INT";
	case LEX_FLOAT :
		return "FLOAT";
	case LEX_STR :
		return "STRING";
	case LEX_EQUAL :
		return "==";
	case LEX_TYPEEQUAL :
		return "===";
	case LEX_NEQUAL :
		return "!=";
	case LEX_NTYPEEQUAL :
		return "!==";
	case LEX_LEQUAL :
		return "<=";
	case LEX_LSHIFT :
		return "<<";
	case LEX_LSHIFTEQUAL :
		return "<<=";
	case LEX_GEQUAL :
		return ">=";
	case LEX_RSHIFT :
		return ">>";
	case LEX_RSHIFTUNSIGNED :
		return ">>";
	case LEX_RSHIFTEQUAL :
		return ">>=";
	case LEX_PLUSEQUAL :
		return "+=";
	case LEX_MINUSEQUAL :
		return "-=";
	case LEX_PLUSPLUS :
		return "++";
	case LEX_MINUSMINUS :
		return "--";
	case LEX_ANDEQUAL :
		return "&=";
	case LEX_ANDAND :
		return "&&";
	case LEX_OREQUAL :
		return "|=";
	case LEX_OROR :
		return "||";
	case LEX_XOREQUAL :
		return "^=";
	// reserved words
	case LEX_R_IF :
		return "if";
	case LEX_R_ELSE :
		return "else";
	case LEX_R_DO :
		return "do";
	case LEX_R_WHILE :
		return "while";
	case LEX_R_FOR :
		return "for";
	case LEX_R_BREAK :
		return "break";
	case LEX_R_CONTINUE :
		return "continue";
	case LEX_R_FUNCTION :
		return "function";
	case LEX_R_RETURN :
		return "return";
	case LEX_R_VAR :
		return "var";
	case LEX_R_TRUE :
		return "true";
	case LEX_R_FALSE :
		return "false";
	case LEX_R_NULL :
		return "null";
	case LEX_R_UNDEFINED :
		return "undefined";
	case LEX_R_NEW :
		return "new";
	}

	ostringstream msg;
	msg << "?[" << token << "]";
	return msg.str();
}

void Lexer::getNextCh()
{
	currCh = nextCh;
	if (dataPos < dataEnd)
		nextCh = data[dataPos];
	else
		nextCh = 0;
	dataPos++;
}

void Lexer::getNextToken()
{
	tk = LEX_EOF;
	tkStr.clear();
	while (currCh && isWhitespace(currCh)) getNextCh();
	// newline comments
	if (currCh=='/' && nextCh=='/')
	{
		while (currCh && currCh!='\n') getNextCh();
		getNextCh();
		getNextToken();
		return;
	}
	// block comments
	if (currCh=='/' && nextCh=='*')
	{
		while (currCh && (currCh!='*' || nextCh!='/')) getNextCh();
		getNextCh();
		getNextCh();
		getNextToken();
		return;
	}
	// record beginning of this token
	tokenStart = dataPos-2;
	// tokens
	if (isAlpha(currCh))   //  IDs
	{
		while (isAlpha(currCh) || isNumeric(currCh))
		{
			tkStr += currCh;
			getNextCh();
		}
		tk = LEX_ID;
		if (tkStr=="if") tk = LEX_R_IF;
		else if (tkStr=="else") tk = LEX_R_ELSE;
		else if (tkStr=="do") tk = LEX_R_DO;
		else if (tkStr=="while") tk = LEX_R_WHILE;
		else if (tkStr=="for") tk = LEX_R_FOR;
		else if (tkStr=="break") tk = LEX_R_BREAK;
		else if (tkStr=="continue") tk = LEX_R_CONTINUE;
		else if (tkStr=="function") tk = LEX_R_FUNCTION;
		else if (tkStr=="return") tk = LEX_R_RETURN;
		else if (tkStr=="var") tk = LEX_R_VAR;
		else if (tkStr=="true") tk = LEX_R_TRUE;
		else if (tkStr=="false") tk = LEX_R_FALSE;
		else if (tkStr=="null") tk = LEX_R_NULL;
		else if (tkStr=="undefined") tk = LEX_R_UNDEFINED;
		else if (tkStr=="new") tk = LEX_R_NEW;
	}
	else if (isNumeric(currCh))     // Numbers
	{
		bool isHex = false;
		if (currCh=='0')
		{
			tkStr += currCh;
			getNextCh();
		}
		if (currCh=='x')
		{
			isHex = true;
			tkStr += currCh;
			getNextCh();
		}
		tk = LEX_INT;
		while (isNumeric(currCh) || (isHex && isHexadecimal(currCh)))
		{
			tkStr += currCh;
			getNextCh();
		}
		if (!isHex && currCh=='.')
		{
			tk = LEX_FLOAT;
			tkStr += '.';
			getNextCh();
			while (isNumeric(currCh))
			{
				tkStr += currCh;
				getNextCh();
			}
		}
		// do fancy e-style floating point
		if (!isHex && (currCh=='e'||currCh=='E'))
		{
			tk = LEX_FLOAT;
			tkStr += currCh;
			getNextCh();
			if (currCh=='-')
			{
				tkStr += currCh;
				getNextCh();
			}
			while (isNumeric(currCh))
			{
				tkStr += currCh;
				getNextCh();
			}
		}
	}
	else if (currCh=='"')
	{
		// strings...
		getNextCh();
		while (currCh && currCh!='"')
		{
			if (currCh == '\\')
			{
				getNextCh();
				switch (currCh)
				{
				case 'n' :
					tkStr += '\n';
					break;
				case '"' :
					tkStr += '"';
					break;
				case '\\' :
					tkStr += '\\';
					break;
				default:
					tkStr += currCh;
				}
			}
			else
			{
				tkStr += currCh;
			}
			getNextCh();
		}
		getNextCh();
		tk = LEX_STR;
	}
	else if (currCh=='\'')
	{
		// strings again...
		getNextCh();
		while (currCh && currCh!='\'')
		{
			if (currCh == '\\')
			{
				getNextCh();
				switch (currCh)
				{
				case 'n' :
					tkStr += '\n';
					break;
				case 'a' :
					tkStr += '\a';
					break;
				case 'r' :
					tkStr += '\r';
					break;
				case 't' :
					tkStr += '\t';
					break;
				case '\'' :
					tkStr += '\'';
					break;
				case '\\' :
					tkStr += '\\';
					break;
				case 'x' :   // hex digits
				{
					char buf[3] = "??";
					getNextCh();
					buf[0] = currCh;
					getNextCh();
					buf[1] = currCh;
					tkStr += (char)strtol(buf,0,16);
				}
				break;
				default:
					if (currCh>='0' && currCh<='7')
					{
						// octal digits
						char buf[4] = "???";
						buf[0] = currCh;
						getNextCh();
						buf[1] = currCh;
						getNextCh();
						buf[2] = currCh;
						tkStr += (char)strtol(buf,0,8);
					}
					else
						tkStr += currCh;
				}
			}
			else
			{
				tkStr += currCh;
			}
			getNextCh();
		}
		getNextCh();
		tk = LEX_STR;
	}
	else
	{
		// single chars
		tk = currCh;
		if (currCh) getNextCh();
		if (tk=='=' && currCh=='=')   // ==
		{
			tk = LEX_EQUAL;
			getNextCh();
			if (currCh=='=')   // ===
			{
				tk = LEX_TYPEEQUAL;
				getNextCh();
			}
		}
		else if (tk=='!' && currCh=='=')     // !=
		{
			tk = LEX_NEQUAL;
			getNextCh();
			if (currCh=='=')   // !==
			{
				tk = LEX_NTYPEEQUAL;
				getNextCh();
			}
		}
		else if (tk=='<' && currCh=='=')
		{
			tk = LEX_LEQUAL;
			getNextCh();
		}
		else if (tk=='<' && currCh=='<')
		{
			tk = LEX_LSHIFT;
			getNextCh();
			if (currCh=='=')   // <<=
			{
				tk = LEX_LSHIFTEQUAL;
				getNextCh();
			}
		}
		else if (tk=='>' && currCh=='=')
		{
			tk = LEX_GEQUAL;
			getNextCh();
		}
		else if (tk=='>' && currCh=='>')
		{
			tk = LEX_RSHIFT;
			getNextCh();
			if (currCh=='=')   // >>=
			{
				tk = LEX_RSHIFTEQUAL;
				getNextCh();
			}
			else if (currCh=='>')     // >>>
			{
				tk = LEX_RSHIFTUNSIGNED;
				getNextCh();
			}
		}
		else if (tk=='+' && currCh=='=')
		{
			tk = LEX_PLUSEQUAL;
			getNextCh();
		}
		else if (tk=='-' && currCh=='=')
		{
			tk = LEX_MINUSEQUAL;
			getNextCh();
		}
		else if (tk=='+' && currCh=='+')
		{
			tk = LEX_PLUSPLUS;
			getNextCh();
		}
		else if (tk=='-' && currCh=='-')
		{
			tk = LEX_MINUSMINUS;
			getNextCh();
		}
		else if (tk=='&' && currCh=='=')
		{
			tk = LEX_ANDEQUAL;
			getNextCh();
		}
		else if (tk=='&' && currCh=='&')
		{
			tk = LEX_ANDAND;
			getNextCh();
		}
		else if (tk=='|' && currCh=='=')
		{
			tk = LEX_OREQUAL;
			getNextCh();
		}
		else if (tk=='|' && currCh=='|')
		{
			tk = LEX_OROR;
			getNextCh();
		}
		else if (tk=='^' && currCh=='=')
		{
			tk = LEX_XOREQUAL;
			getNextCh();
		}
	}
	/* This isn't quite right yet */
	tokenLastEnd = tokenEnd;
	tokenEnd = dataPos-3;
}

string Lexer::getSubString(int lastPosition)
{
	int lastCharIdx = tokenLastEnd+1;
	if (lastCharIdx < dataEnd)
	{
		/* save a memory alloc by using our data array to create the
		   substring */
		char old = data[lastCharIdx];
		data[lastCharIdx] = 0;
		std::string value = &data[lastPosition];
		data[lastCharIdx] = old;
		return value;
	}
	else
	{
		return std::string(&data[lastPosition]);
	}
}


Lexer * Lexer::getSubLex(int lastPosition)
{
	int lastCharIdx = tokenLastEnd+1;
	if (lastCharIdx < dataEnd)
		return new Lexer(this, lastPosition, lastCharIdx);
	else
		return new Lexer(this, lastPosition, dataEnd );
}

string Lexer::getPosition(int pos)
{
	if (pos<0) pos=tokenLastEnd;
	int line = 1,col = 1;
	for (int i=0; i<pos; i++)
	{
		char ch;
		if (i < dataEnd)
			ch = data[i];
		else
			ch = 0;
		col++;
		if (ch=='\n')
		{
			line++;
			col = 0;
		}
	}
	char buf[256];
	sprintf_s(buf, 256, "(line: %d, col: %d)", line, col);
	return buf;
}

// ----------------------------------------------------------------------------------- VarLINK

VarLink::VarLink(Var * var, const std::string & name)
{
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	this->name = name;
	this->nextSibling = 0;
	this->prevSibling = 0;
	this->var = var->ref();
	this->owned = false;
}

VarLink::VarLink(const VarLink & link)
{
	// Copy constructor
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	this->name = link.name;
	this->nextSibling = 0;
	this->prevSibling = 0;
	this->var = link.var->ref();
	this->owned = false;
}

VarLink::~VarLink()
{
#if DEBUG_MEMORY
	mark_deallocated(this);
#endif
	var->unref();
}

void VarLink::replaceWith(Var * newVar)
{
	Var * oldVar = var;
	var = newVar->ref();
	oldVar->unref();
}

void VarLink::replaceWith(VarLink * newVar)
{
	if (newVar)
		replaceWith(newVar->var);
	else
		replaceWith(new Var());
}

int VarLink::getIntName()
{
	return atoi(name.c_str());
}
void VarLink::setIntName(int n)
{
	char sIdx[64];
	sprintf_s(sIdx, sizeof(sIdx), "%d", n);
	name = sIdx;
}

// ----------------------------------------------------------------------------------- Var

Var::Var()
{
	refs = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	flags = SCRIPTVAR_UNDEFINED;
}

Var::Var(const string & str)
{
	refs = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	flags = SCRIPTVAR_STRING;
	data = str;
}


Var::Var(const string & varData, int varFlags)
{
	refs = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	flags = varFlags;
	if (varFlags & SCRIPTVAR_INTEGER)
	{
		intData = strtol(varData.c_str(),0,0);
	}
	else if (varFlags & SCRIPTVAR_DOUBLE)
	{
		doubleData = strtod(varData.c_str(),0);
	}
	else
		data = varData;
}

Var::Var(double val)
{
	refs = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	setDouble(val);
}

Var::Var(int val)
{
	refs = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	setInt(val);
}

Var::~Var(void)
{
#if DEBUG_MEMORY
	mark_deallocated(this);
#endif
	removeAllChildren();
}

void Var::init()
{
	firstChild = 0;
	lastChild = 0;
	flags = 0;
	jsCallback = 0;
	jsCallbackUserData = 0;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
}

Var * Var::getReturnVar()
{
	return getParameter(TINYJS_RETURN_VAR);
}

void Var::setReturnVar(Var * var)
{
	findChildOrCreate(TINYJS_RETURN_VAR)->replaceWith(var);
}


Var * Var::getParameter(const std::string & name)
{
	return findChildOrCreate(name)->var;
}

VarLink * Var::findChild(const string & childName)
{
	VarLink * v = firstChild;
	while (v)
	{
		if (v->name.compare(childName)==0)
			return v;
		v = v->nextSibling;
	}
	return 0;
}

VarLink * Var::findChildOrCreate(const string & childName, int varFlags)
{
	VarLink * l = findChild(childName);
	if (l) return l;

	return addChild(childName, new Var(TINYJS_BLANK_DATA, varFlags));
}

VarLink * Var::findChildOrCreateByPath(const std::string & path)
{
	size_t p = path.find('.');
	if (p == string::npos)
		return findChildOrCreate(path);

	return findChildOrCreate(path.substr(0,p), SCRIPTVAR_OBJECT)->var->
		   findChildOrCreateByPath(path.substr(p+1));
}

VarLink * Var::addChild(const std::string & childName, Var * child)
{
	if (isUndefined())
	{
		flags = SCRIPTVAR_OBJECT;
	}
	// if no child supplied, create one
	if (!child)
		child = new Var();

	VarLink * link = new VarLink(child, childName);
	link->owned = true;
	if (lastChild)
	{
		lastChild->nextSibling = link;
		link->prevSibling = lastChild;
		lastChild = link;
	}
	else
	{
		firstChild = link;
		lastChild = link;
	}
	return link;
}

VarLink * Var::addChildNoDup(const std::string & childName, Var * child)
{
	// if no child supplied, create one
	if (!child)
		child = new Var();

	VarLink * v = findChild(childName);
	if (v)
	{
		v->replaceWith(child);
	}
	else
	{
		v = addChild(childName, child);
	}

	return v;
}

void Var::removeChild(Var * child)
{
	VarLink * link = firstChild;
	while (link)
	{
		if (link->var == child)
			break;
		link = link->nextSibling;
	}
	ASSERT(link);
	removeLink(link);
}

void Var::removeLink(VarLink * link)
{
	if (!link) return;
	if (link->nextSibling)
		link->nextSibling->prevSibling = link->prevSibling;
	if (link->prevSibling)
		link->prevSibling->nextSibling = link->nextSibling;
	if (lastChild == link)
		lastChild = link->prevSibling;
	if (firstChild == link)
		firstChild = link->nextSibling;
	delete link;
}

void Var::removeAllChildren()
{
	VarLink * c = firstChild;
	while (c)
	{
		VarLink * t = c->nextSibling;
		delete c;
		c = t;
	}
	firstChild = 0;
	lastChild = 0;
}

Var * Var::getArrayIndex(int idx)
{
	char sIdx[64];
	sprintf_s(sIdx, sizeof(sIdx), "%d", idx);
	VarLink * link = findChild(sIdx);
	if (link) return link->var;
	else return new Var(TINYJS_BLANK_DATA, SCRIPTVAR_NULL); // undefined
}

void Var::setArrayIndex(int idx, Var * value)
{
	char sIdx[64];
	sprintf_s(sIdx, sizeof(sIdx), "%d", idx);
	VarLink * link = findChild(sIdx);

	if (link)
	{
		if (value->isUndefined())
			removeLink(link);
		else
			link->replaceWith(value);
	}
	else
	{
		if (!value->isUndefined())
			addChild(sIdx, value);
	}
}

int Var::getArrayLength()
{
	int highest = -1;
	if (!isArray()) return 0;

	VarLink * link = firstChild;
	while (link)
	{
		if (isNumber(link->name))
		{
			int val = atoi(link->name.c_str());
			if (val > highest) highest = val;
		}
		link = link->nextSibling;
	}
	return highest+1;
}

int Var::getChildren()
{
	int n = 0;
	VarLink * link = firstChild;
	while (link)
	{
		n++;
		link = link->nextSibling;
	}
	return n;
}

int Var::getInt()
{
	/* strtol understands about hex and octal */
	if (isInt()) return intData;
	if (isNull()) return 0;
	if (isUndefined()) return 0;
	if (isDouble()) return (int)doubleData;
	return 0;
}

double Var::getDouble()
{
	if (isDouble()) return doubleData;
	if (isInt()) return intData;
	if (isNull()) return 0;
	if (isUndefined()) return 0;
	return 0; /* or NaN? */
}

const string & Var::getString()
{
	/* Because we can't return a string that is generated on demand.
	 * I should really just use char* :) */
	static string s_null = "null";
	static string s_undefined = "undefined";
	if (isInt())
	{
		char buffer[32];
		sprintf_s(buffer, sizeof(buffer), "%ld", intData);
		data = buffer;
		return data;
	}
	if (isDouble())
	{
		char buffer[32];
		sprintf_s(buffer, sizeof(buffer), "%f", doubleData);
		data = buffer;
		return data;
	}
	if (isNull()) return s_null;
	if (isUndefined()) return s_undefined;
	// are we just a string here?
	return data;
}

void Var::setInt(int val)
{
	flags = (flags&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_INTEGER;
	intData = val;
	doubleData = 0;
	data = TINYJS_BLANK_DATA;
}

void Var::setDouble(double val)
{
	flags = (flags&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_DOUBLE;
	doubleData = val;
	intData = 0;
	data = TINYJS_BLANK_DATA;
}

void Var::setString(const string & str)
{
	// name sure it's not still a number or integer
	flags = (flags&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_STRING;
	data = str;
	intData = 0;
	doubleData = 0;
}

void Var::setUndefined()
{
	// name sure it's not still a number or integer
	flags = (flags&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_UNDEFINED;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
	removeAllChildren();
}

void Var::setArray()
{
	// name sure it's not still a number or integer
	flags = (flags&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_ARRAY;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
	removeAllChildren();
}

bool Var::equals(Var * v)
{
	Var * resV = mathsOp(v, LEX_EQUAL);
	bool res = resV->getBool();
	delete resV;
	return res;
}

Var * Var::mathsOp(Var * b, int op)
{
	Var * a = this;
	// Type equality check
	if (op == LEX_TYPEEQUAL || op == LEX_NTYPEEQUAL)
	{
		// check type first, then call again to check data
		bool eql = ((a->flags & SCRIPTVAR_VARTYPEMASK) ==
					(b->flags & SCRIPTVAR_VARTYPEMASK));
		if (eql)
		{
			Var * contents = a->mathsOp(b, LEX_EQUAL);
			if (!contents->getBool()) eql = false;
			if (!contents->refs) delete contents;
		}
		;
		if (op == LEX_TYPEEQUAL)
			return new Var(eql);
		else
			return new Var(!eql);
	}
	// do maths...
	if (a->isUndefined() && b->isUndefined())
	{
		if (op == LEX_EQUAL) return new Var(true);
		else if (op == LEX_NEQUAL) return new Var(false);
		else return new Var(); // undefined
	}
	else if ((a->isNumeric() || a->isUndefined()) &&
			 (b->isNumeric() || b->isUndefined()))
	{
		if (!a->isDouble() && !b->isDouble())
		{
			// use ints
			int da = a->getInt();
			int db = b->getInt();
			switch (op)
			{
			case '+':
				return new Var(da+db);
			case '-':
				return new Var(da-db);
			case '*':
				return new Var(da*db);
			case '/':
				return new Var(da/db);
			case '&':
				return new Var(da&db);
			case '|':
				return new Var(da|db);
			case '^':
				return new Var(da^db);
			case '%':
				return new Var(da%db);
			case LEX_EQUAL:
				return new Var(da==db);
			case LEX_NEQUAL:
				return new Var(da!=db);
			case '<':
				return new Var(da<db);
			case LEX_LEQUAL:
				return new Var(da<=db);
			case '>':
				return new Var(da>db);
			case LEX_GEQUAL:
				return new Var(da>=db);
			default:
				throw new Exception("Operation "+Lexer::getTokenStr(op)+" not supported on the Int datatype");
			}
		}
		else
		{
			// use doubles
			double da = a->getDouble();
			double db = b->getDouble();
			switch (op)
			{
			case '+':
				return new Var(da+db);
			case '-':
				return new Var(da-db);
			case '*':
				return new Var(da*db);
			case '/':
				return new Var(da/db);
			case LEX_EQUAL:
				return new Var(da==db);
			case LEX_NEQUAL:
				return new Var(da!=db);
			case '<':
				return new Var(da<db);
			case LEX_LEQUAL:
				return new Var(da<=db);
			case '>':
				return new Var(da>db);
			case LEX_GEQUAL:
				return new Var(da>=db);
			default:
				throw new Exception("Operation "+Lexer::getTokenStr(op)+" not supported on the Double datatype");
			}
		}
	}
	else if (a->isArray())
	{
		/* Just check pointers */
		switch (op)
		{
		case LEX_EQUAL:
			return new Var(a==b);
		case LEX_NEQUAL:
			return new Var(a!=b);
		default:
			throw new Exception("Operation "+Lexer::getTokenStr(op)+" not supported on the Array datatype");
		}
	}
	else if (a->isObject())
	{
		/* Just check pointers */
		switch (op)
		{
		case LEX_EQUAL:
			return new Var(a==b);
		case LEX_NEQUAL:
			return new Var(a!=b);
		default:
			throw new Exception("Operation "+Lexer::getTokenStr(op)+" not supported on the Object datatype");
		}
	}
	else
	{
		string da = a->getString();
		string db = b->getString();
		// use strings
		switch (op)
		{
		case '+':
			return new Var(da+db, SCRIPTVAR_STRING);
		case LEX_EQUAL:
			return new Var(da==db);
		case LEX_NEQUAL:
			return new Var(da!=db);
		case '<':
			return new Var(da<db);
		case LEX_LEQUAL:
			return new Var(da<=db);
		case '>':
			return new Var(da>db);
		case LEX_GEQUAL:
			return new Var(da>=db);
		default:
			throw new Exception("Operation "+Lexer::getTokenStr(op)+" not supported on the string datatype");
		}
	}
	ASSERT(0);
	return 0;
}

void Var::copySimpleData(Var * val)
{
	data = val->data;
	intData = val->intData;
	doubleData = val->doubleData;
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | (val->flags & SCRIPTVAR_VARTYPEMASK);
}

void Var::copyValue(Var * val)
{
	if (val)
	{
		copySimpleData(val);
		// remove all current children
		removeAllChildren();
		// copy children of 'val'
		VarLink * child = val->firstChild;
		while (child)
		{
			Var * copied;
			// don't copy the 'parent' object...
			if (child->name != TINYJS_PROTOTYPE_CLASS)
				copied = child->var->deepCopy();
			else
				copied = child->var;

			addChild(child->name, copied);

			child = child->nextSibling;
		}
	}
	else
	{
		setUndefined();
	}
}

Var * Var::deepCopy()
{
	Var * newVar = new Var();
	newVar->copySimpleData(this);
	// copy children
	VarLink * child = firstChild;
	while (child)
	{
		Var * copied;
		// don't copy the 'parent' object...
		if (child->name != TINYJS_PROTOTYPE_CLASS)
			copied = child->var->deepCopy();
		else
			copied = child->var;

		newVar->addChild(child->name, copied);
		child = child->nextSibling;
	}
	return newVar;
}

void Var::trace(string indentStr, const string & name)
{
	TRACE("%s'%s' = '%s' %s\n",
		  indentStr.c_str(),
		  name.c_str(),
		  getString().c_str(),
		  getFlagsAsString().c_str());
	string indent = indentStr+" ";
	VarLink * link = firstChild;
	while (link)
	{
		link->var->trace(indent, link->name);
		link = link->nextSibling;
	}
}

string Var::getFlagsAsString()
{
	string flagstr = "";
	if (flags&SCRIPTVAR_FUNCTION) flagstr = flagstr + "FUNCTION ";
	if (flags&SCRIPTVAR_OBJECT) flagstr = flagstr + "OBJECT ";
	if (flags&SCRIPTVAR_ARRAY) flagstr = flagstr + "ARRAY ";
	if (flags&SCRIPTVAR_NATIVE) flagstr = flagstr + "NATIVE ";
	if (flags&SCRIPTVAR_DOUBLE) flagstr = flagstr + "DOUBLE ";
	if (flags&SCRIPTVAR_INTEGER) flagstr = flagstr + "INTEGER ";
	if (flags&SCRIPTVAR_STRING) flagstr = flagstr + "STRING ";
	return flagstr;
}

string Var::getParsableString()
{
	// Numbers can just be put in directly
	if (isNumeric())
		return getString();
	if (isFunction())
	{
		ostringstream funcStr;
		funcStr << "function (";
		// get list of parameters
		VarLink * link = firstChild;
		while (link)
		{
			funcStr << link->name;
			if (link->nextSibling) funcStr << ",";
			link = link->nextSibling;
		}
		// add function body
		funcStr << ") " << getString();
		return funcStr.str();
	}
	// if it is a string then we quote it
	if (isString())
		return JS::getJSString(getString());
	if (isNull())
		return "null";
	return "undefined";
}

void Var::getJSON(ostringstream & destination, const string linePrefix)
{
	if (isObject())
	{
		string indentedLinePrefix = linePrefix+"  ";
		// children - handle with bracketed list
		destination << "{ \n";
		VarLink * link = firstChild;
		while (link)
		{
			destination << indentedLinePrefix;
			destination  << getJSString(link->name);
			destination  << " : ";
			link->var->getJSON(destination, indentedLinePrefix);
			link = link->nextSibling;
			if (link)
			{
				destination  << ",\n";
			}
		}
		destination << "\n" << linePrefix << "}";
	}
	else if (isArray())
	{
		string indentedLinePrefix = linePrefix+"  ";
		destination << "[\n";
		int len = getArrayLength();
		if (len>10000) len=10000; // we don't want to get stuck here!

		for (int i=0; i<len; i++)
		{
			getArrayIndex(i)->getJSON(destination, indentedLinePrefix);
			if (i<len-1) destination  << ",\n";
		}

		destination << "\n" << linePrefix << "]";
	}
	else
	{
		// no children or a function... just write value directly
		destination << getParsableString();
	}
}


void Var::setCallback(JSCallback callback, void * userdata)
{
	jsCallback = callback;
	jsCallbackUserData = userdata;
}

Var * Var::ref()
{
	refs++;
	return this;
}

void Var::unref()
{
	if (refs<=0) printf("OMFG, we have unreffed too far!\n");
	if ((--refs)==0)
	{
		delete this;
	}
}

int Var::getRefs()
{
	return refs;
}


// ----------------------------------------------------------------------------------- CSCRIPT
extern void registerFunctions(Engine * tinyJS);
extern void registerMathFunctions(Engine * tinyJS);

Engine::Engine()
{
	l = 0;
	root = (new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();
	// Add built-in classes
	stringClass = (new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();
	arrayClass = (new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();
	objectClass = (new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();
	root->addChild("String", stringClass);
	root->addChild("Array", arrayClass);
	root->addChild("Object", objectClass);

	// Register all core features
	registerFunctions(this);
	registerMathFunctions(this);
}

Engine::~Engine()
{
	ASSERT(!l);
	scopes.clear();
	stringClass->unref();
	arrayClass->unref();
	objectClass->unref();
	root->unref();

#if DEBUG_MEMORY
	show_allocated();
#endif
}

void Engine::trace()
{
	root->trace();
}

void Engine::execute(const string & code)
{
	Lexer * oldLex = l;
	vector<Var *> oldScopes = scopes;
	l = new Lexer(code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif
	scopes.clear();
	scopes.push_back(root);
	try
	{
		bool execute = true;
		while (l->tk) statement(execute);
	}
	catch (Exception * e)
	{
		ostringstream msg;
		msg << "Error " << e->text;
#ifdef TINYJS_CALL_STACK
		for (int i=(int)call_stack.size()-1; i>=0; i--)
			msg << "\n" << i << ": " << call_stack.at(i);
#endif
		msg << " at " << l->getPosition();
		delete l;
		l = oldLex;

		throw new Exception(msg.str());
	}
	delete l;
	l = oldLex;
	scopes = oldScopes;
}

VarLink Engine::evaluateComplex(const string & code)
{
	Lexer * oldLex = l;
	vector<Var *> oldScopes = scopes;

	l = new Lexer(code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif
	scopes.clear();
	scopes.push_back(root);
	VarLink * v = 0;
	try
	{
		bool execute = true;
		do
		{
			CLEAN(v);
			v = base(execute);
			if (l->tk!=LEX_EOF) l->match(';');
		}
		while (l->tk!=LEX_EOF);
	}
	catch (Exception * e)
	{
		ostringstream msg;
		msg << "Error " << e->text;
#ifdef TINYJS_CALL_STACK
		for (int i=(int)call_stack.size()-1; i>=0; i--)
			msg << "\n" << i << ": " << call_stack.at(i);
#endif
		msg << " at " << l->getPosition();
		delete l;
		l = oldLex;

		throw new Exception(msg.str());
	}
	delete l;
	l = oldLex;
	scopes = oldScopes;

	if (v)
	{
		VarLink r = *v;
		CLEAN(v);
		return r;
	}
	// return undefined...
	return VarLink(new Var());
}

string Engine::evaluate(const string & code)
{
	return evaluateComplex(code).var->getString();
}

void Engine::parseFunctionArguments(Var * funcVar)
{
	l->match('(');
	while (l->tk!=')')
	{
		funcVar->addChildNoDup(l->tkStr);
		l->match(LEX_ID);
		if (l->tk!=')') l->match(',');
	}
	l->match(')');
}

void Engine::addNative(const string & funcDesc, JSCallback ptr, void * userdata)
{
	Lexer * oldLex = l;
	l = new Lexer(funcDesc);

	Var * base = root;

	l->match(LEX_R_FUNCTION);
	string funcName = l->tkStr;
	l->match(LEX_ID);
	/* Check for dots, we might want to do something like function String.substring ... */
	while (l->tk == '.')
	{
		l->match('.');
		VarLink * link = base->findChild(funcName);
		// if it doesn't exist, make an object class
		if (!link) link = base->addChild(funcName, new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
		base = link->var;
		funcName = l->tkStr;
		l->match(LEX_ID);
	}

	Var * funcVar = new Var(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	funcVar->setCallback(ptr, userdata);
	parseFunctionArguments(funcVar);
	delete l;
	l = oldLex;

	base->addChild(funcName, funcVar);
}

VarLink * Engine::parseFunctionDefinition()
{
	// actually parse a function...
	l->match(LEX_R_FUNCTION);
	string funcName = TINYJS_TEMP_NAME;
	/* we can have functions without names */
	if (l->tk==LEX_ID)
	{
		funcName = l->tkStr;
		l->match(LEX_ID);
	}
	VarLink * funcVar = new VarLink(new Var(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION), funcName);
	parseFunctionArguments(funcVar->var);
	int funcBegin = l->tokenStart;
	bool noexecute = false;
	block(noexecute);
	funcVar->var->data = l->getSubString(funcBegin);
	return funcVar;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normnal function).
 */
VarLink * Engine::functionCall(bool & execute, VarLink * function, Var * parent)
{
	if (execute)
	{
		if (!function->var->isFunction())
		{
			string errorMsg = "Expecting '";
			errorMsg = errorMsg + function->name + "' to be a function";
			throw new Exception(errorMsg.c_str());
		}
		l->match('(');
		// create a new symbol table entry for execution of this function
		Var * functionRoot = new Var(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION);
		if (parent)
			functionRoot->addChildNoDup("this", parent);
		// grab in all parameters
		VarLink * v = function->var->firstChild;
		while (v)
		{
			VarLink * value = base(execute);
			if (execute)
			{
				if (value->var->isBasic())
				{
					// pass by value
					functionRoot->addChild(v->name, value->var->deepCopy());
				}
				else
				{
					// pass by reference
					functionRoot->addChild(v->name, value->var);
				}
			}
			CLEAN(value);
			if (l->tk!=')') l->match(',');
			v = v->nextSibling;
		}
		l->match(')');
		// setup a return variable
		VarLink * returnVar = NULL;
		// execute function!
		// add the function's execute space to the symbol table so we can recurse
		VarLink * returnVarLink = functionRoot->addChild(TINYJS_RETURN_VAR);
		scopes.push_back(functionRoot);
#ifdef TINYJS_CALL_STACK
		call_stack.push_back(function->name + " from " + l->getPosition());
#endif

		if (function->var->isNative())
		{
			ASSERT(function->var->jsCallback);
			function->var->jsCallback(functionRoot, function->var->jsCallbackUserData);
		}
		else
		{
			/* we just want to execute the block, but something could
			 * have messed up and left us with the wrong ScriptLex, so
			 * we want to be careful here... */
			Exception * exception = 0;
			Lexer * oldLex = l;
			Lexer * newLex = new Lexer(function->var->getString());
			l = newLex;
			try
			{
				block(execute);
				// because return will probably have called this, and set execute to false
				execute = true;
			}
			catch (Exception * e)
			{
				exception = e;
			}
			delete newLex;
			l = oldLex;

			if (exception)
				throw exception;
		}
#ifdef TINYJS_CALL_STACK
		if (!call_stack.empty()) call_stack.pop_back();
#endif
		scopes.pop_back();
		/* get the real return var before we remove it from our function */
		returnVar = new VarLink(returnVarLink->var);
		functionRoot->removeLink(returnVarLink);
		delete functionRoot;
		if (returnVar)
			return returnVar;
		else
			return new VarLink(new Var());
	}
	else
	{
		// function, but not executing - just parse args and be done
		l->match('(');
		while (l->tk != ')')
		{
			VarLink * value = base(execute);
			CLEAN(value);
			if (l->tk!=')') l->match(',');
		}
		l->match(')');
		if (l->tk == '{')   // TODO: why is this here?
		{
			block(execute);
		}
		/* function will be a blank scriptvarlink if we're not executing,
		 * so just return it rather than an alloc/free */
		return function;
	}
}

VarLink * Engine::factor(bool & execute)
{
	if (l->tk=='(')
	{
		l->match('(');
		VarLink * a = base(execute);
		l->match(')');
		return a;
	}
	if (l->tk==LEX_R_TRUE)
	{
		l->match(LEX_R_TRUE);
		return new VarLink(new Var(1));
	}
	if (l->tk==LEX_R_FALSE)
	{
		l->match(LEX_R_FALSE);
		return new VarLink(new Var(0));
	}
	if (l->tk==LEX_R_NULL)
	{
		l->match(LEX_R_NULL);
		return new VarLink(new Var(TINYJS_BLANK_DATA,SCRIPTVAR_NULL));
	}
	if (l->tk==LEX_R_UNDEFINED)
	{
		l->match(LEX_R_UNDEFINED);
		return new VarLink(new Var(TINYJS_BLANK_DATA,SCRIPTVAR_UNDEFINED));
	}
	if (l->tk==LEX_ID)
	{
		VarLink * a = execute ? findInScopes(l->tkStr) : new VarLink(new Var());
		//printf("0x%08X for %s at %s\n", (unsigned int)a, l->tkStr.c_str(), l->getPosition().c_str());
		/* The parent if we're executing a method call */
		Var * parent = 0;

		if (execute && !a)
		{
			/* Variable doesn't exist! JavaScript says we should create it
			 * (we won't add it here. This is done in the assignment operator)*/
			a = new VarLink(new Var(), l->tkStr);
		}
		l->match(LEX_ID);
		while (l->tk=='(' || l->tk=='.' || l->tk=='[')
		{
			if (l->tk=='(')   // ------------------------------------- Function Call
			{
				a = functionCall(execute, a, parent);
			}
			else if (l->tk == '.')     // ------------------------------------- Record Access
			{
				l->match('.');
				if (execute)
				{
					const string & name = l->tkStr;
					VarLink * child = a->var->findChild(name);
					if (!child) child = findInParentClasses(a->var, name);
					if (!child)
					{
						/* if we haven't found this defined yet, use the built-in
						   'length' properly */
						if (a->var->isArray() && name == "length")
						{
							int l = a->var->getArrayLength();
							child = new VarLink(new Var(l));
						}
						else if (a->var->isString() && name == "length")
						{
							int l = a->var->getString().size();
							child = new VarLink(new Var(l));
						}
						else
						{
							child = a->var->addChild(name);
						}
					}
					parent = a->var;
					a = child;
				}
				l->match(LEX_ID);
			}
			else if (l->tk == '[')     // ------------------------------------- Array Access
			{
				l->match('[');
				VarLink * index = base(execute);
				l->match(']');
				if (execute)
				{
					VarLink * child = a->var->findChildOrCreate(index->var->getString());
					parent = a->var;
					a = child;
				}
				CLEAN(index);
			}
			else ASSERT(0);
		}
		return a;
	}
	if (l->tk==LEX_INT || l->tk==LEX_FLOAT)
	{
		Var * a = new Var(l->tkStr,
						  ((l->tk==LEX_INT)?SCRIPTVAR_INTEGER:SCRIPTVAR_DOUBLE));
		l->match(l->tk);
		return new VarLink(a);
	}
	if (l->tk==LEX_STR)
	{
		Var * a = new Var(l->tkStr, SCRIPTVAR_STRING);
		l->match(LEX_STR);
		return new VarLink(a);
	}
	if (l->tk=='{')
	{
		Var * contents = new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
		/* JSON-style object definition */
		l->match('{');
		while (l->tk != '}')
		{
			string id = l->tkStr;
			// we only allow strings or IDs on the left hand side of an initialisation
			if (l->tk==LEX_STR) l->match(LEX_STR);
			else l->match(LEX_ID);
			l->match(':');
			if (execute)
			{
				VarLink * a = base(execute);
				contents->addChild(id, a->var);
				CLEAN(a);
			}
			// no need to clean here, as it will definitely be used
			if (l->tk != '}') l->match(',');
		}

		l->match('}');
		return new VarLink(contents);
	}
	if (l->tk=='[')
	{
		Var * contents = new Var(TINYJS_BLANK_DATA, SCRIPTVAR_ARRAY);
		/* JSON-style array */
		l->match('[');
		int idx = 0;
		while (l->tk != ']')
		{
			if (execute)
			{
				char idx_str[16]; // big enough for 2^32
				sprintf_s(idx_str, sizeof(idx_str), "%d",idx);

				VarLink * a = base(execute);
				contents->addChild(idx_str, a->var);
				CLEAN(a);
			}
			// no need to clean here, as it will definitely be used
			if (l->tk != ']') l->match(',');
			idx++;
		}
		l->match(']');
		return new VarLink(contents);
	}
	if (l->tk==LEX_R_FUNCTION)
	{
		VarLink * funcVar = parseFunctionDefinition();
		if (funcVar->name != TINYJS_TEMP_NAME)
			TRACE("Functions not defined at statement-level are not meant to have a name");
		return funcVar;
	}
	if (l->tk==LEX_R_NEW)
	{
		// new -> create a new object
		l->match(LEX_R_NEW);
		const string & className = l->tkStr;
		if (execute)
		{
			VarLink * objClassOrFunc = findInScopes(className);
			if (!objClassOrFunc)
			{
				TRACE("%s is not a valid class name", className.c_str());
				return new VarLink(new Var());
			}
			l->match(LEX_ID);
			Var * obj = new Var(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
			VarLink * objLink = new VarLink(obj);
			if (objClassOrFunc->var->isFunction())
			{
				CLEAN(functionCall(execute, objClassOrFunc, obj));
			}
			else
			{
				obj->addChild(TINYJS_PROTOTYPE_CLASS, objClassOrFunc->var);
				if (l->tk == '(')
				{
					l->match('(');
					l->match(')');
				}
			}
			return objLink;
		}
		else
		{
			l->match(LEX_ID);
			if (l->tk == '(')
			{
				l->match('(');
				l->match(')');
			}
		}
	}
	// Nothing we can do here... just hope it's the end...
	l->match(LEX_EOF);
	return 0;
}

VarLink * Engine::unary(bool & execute)
{
	VarLink * a;
	if (l->tk=='!')
	{
		l->match('!'); // binary not
		a = factor(execute);
		if (execute)
		{
			Var zero(0);
			Var * res = a->var->mathsOp(&zero, LEX_EQUAL);
			CREATE_LINK(a, res);
		}
	}
	else
		a = factor(execute);
	return a;
}

VarLink * Engine::term(bool & execute)
{
	VarLink * a = unary(execute);
	while (l->tk=='*' || l->tk=='/' || l->tk=='%')
	{
		int op = l->tk;
		l->match(l->tk);
		VarLink * b = unary(execute);
		if (execute)
		{
			Var * res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}
		CLEAN(b);
	}
	return a;
}

VarLink * Engine::expression(bool & execute)
{
	bool negate = false;
	if (l->tk=='-')
	{
		l->match('-');
		negate = true;
	}
	VarLink * a = term(execute);
	if (negate)
	{
		Var zero(0);
		Var * res = zero.mathsOp(a->var, '-');
		CREATE_LINK(a, res);
	}

	while (l->tk=='+' || l->tk=='-' ||
			l->tk==LEX_PLUSPLUS || l->tk==LEX_MINUSMINUS)
	{
		int op = l->tk;
		l->match(l->tk);
		if (op==LEX_PLUSPLUS || op==LEX_MINUSMINUS)
		{
			if (execute)
			{
				Var one(1);
				Var * res = a->var->mathsOp(&one, op==LEX_PLUSPLUS ? '+' : '-');
				VarLink * oldValue = new VarLink(a->var);
				// in-place add/subtract
				a->replaceWith(res);
				CLEAN(a);
				a = oldValue;
			}
		}
		else
		{
			VarLink * b = term(execute);
			if (execute)
			{
				// not in-place, so just replace
				Var * res = a->var->mathsOp(b->var, op);
				CREATE_LINK(a, res);
			}
			CLEAN(b);
		}
	}
	return a;
}

VarLink * Engine::shift(bool & execute)
{
	VarLink * a = expression(execute);
	if (l->tk==LEX_LSHIFT || l->tk==LEX_RSHIFT || l->tk==LEX_RSHIFTUNSIGNED)
	{
		int op = l->tk;
		l->match(op);
		VarLink * b = base(execute);
		int shift = execute ? b->var->getInt() : 0;
		CLEAN(b);
		if (execute)
		{
			if (op==LEX_LSHIFT) a->var->setInt(a->var->getInt() << shift);
			if (op==LEX_RSHIFT) a->var->setInt(a->var->getInt() >> shift);
			if (op==LEX_RSHIFTUNSIGNED) a->var->setInt(((unsigned int)a->var->getInt()) >> shift);
		}
	}
	return a;
}

VarLink * Engine::condition(bool & execute)
{
	VarLink * a = shift(execute);
	VarLink * b;
	while (l->tk==LEX_EQUAL || l->tk==LEX_NEQUAL ||
			l->tk==LEX_TYPEEQUAL || l->tk==LEX_NTYPEEQUAL ||
			l->tk==LEX_LEQUAL || l->tk==LEX_GEQUAL ||
			l->tk=='<' || l->tk=='>')
	{
		int op = l->tk;
		l->match(l->tk);
		b = shift(execute);
		if (execute)
		{
			Var * res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a,res);
		}
		CLEAN(b);
	}
	return a;
}

VarLink * Engine::logic(bool & execute)
{
	VarLink * a = condition(execute);
	VarLink * b;
	while (l->tk=='&' || l->tk=='|' || l->tk=='^' || l->tk==LEX_ANDAND || l->tk==LEX_OROR)
	{
		bool noexecute = false;
		int op = l->tk;
		l->match(l->tk);
		bool shortCircuit = false;
		bool boolean = false;
		// if we have short-circuit ops, then if we know the outcome
		// we don't bother to execute the other op. Even if not
		// we need to tell mathsOp it's an & or |
		if (op==LEX_ANDAND)
		{
			op = '&';
			shortCircuit = !a->var->getBool();
			boolean = true;
		}
		else if (op==LEX_OROR)
		{
			op = '|';
			shortCircuit = a->var->getBool();
			boolean = true;
		}
		b = condition(shortCircuit ? noexecute : execute);
		if (execute && !shortCircuit)
		{
			if (boolean)
			{
				Var * newa = new Var(a->var->getBool());
				Var * newb = new Var(b->var->getBool());
				CREATE_LINK(a, newa);
				CREATE_LINK(b, newb);
			}
			Var * res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}
		CLEAN(b);
	}
	return a;
}

VarLink * Engine::ternary(bool & execute)
{
	VarLink * lhs = logic(execute);
	bool noexec = false;
	if (l->tk=='?')
	{
		l->match('?');
		if (!execute)
		{
			CLEAN(lhs);
			CLEAN(base(noexec));
			l->match(':');
			CLEAN(base(noexec));
		}
		else
		{
			bool first = lhs->var->getBool();
			CLEAN(lhs);
			if (first)
			{
				lhs = base(execute);
				l->match(':');
				CLEAN(base(noexec));
			}
			else
			{
				CLEAN(base(noexec));
				l->match(':');
				lhs = base(execute);
			}
		}
	}

	return lhs;
}

VarLink * Engine::base(bool & execute)
{
	VarLink * lhs = ternary(execute);
	if (l->tk=='=' || l->tk==LEX_PLUSEQUAL || l->tk==LEX_MINUSEQUAL)
	{
		/* If we're assigning to this and we don't have a parent,
		 * add it to the symbol table root as per JavaScript. */
		if (execute && !lhs->owned)
		{
			if (lhs->name.length()>0)
			{
				VarLink * realLhs = root->addChildNoDup(lhs->name, lhs->var);
				CLEAN(lhs);
				lhs = realLhs;
			}
			else
				TRACE("Trying to assign to an un-named type\n");
		}

		int op = l->tk;
		l->match(l->tk);
		VarLink * rhs = base(execute);
		if (execute)
		{
			if (op=='=')
			{
				lhs->replaceWith(rhs);
			}
			else if (op==LEX_PLUSEQUAL)
			{
				Var * res = lhs->var->mathsOp(rhs->var, '+');
				lhs->replaceWith(res);
			}
			else if (op==LEX_MINUSEQUAL)
			{
				Var * res = lhs->var->mathsOp(rhs->var, '-');
				lhs->replaceWith(res);
			}
			else ASSERT(0);
		}
		CLEAN(rhs);
	}
	return lhs;
}

void Engine::block(bool & execute)
{
	l->match('{');
	if (execute)
	{
		while (l->tk && l->tk!='}')
			statement(execute);
		l->match('}');
	}
	else
	{
		// fast skip of blocks
		int brackets = 1;
		while (l->tk && brackets)
		{
			if (l->tk == '{') brackets++;
			if (l->tk == '}') brackets--;
			l->match(l->tk);
		}
	}

}

void Engine::statement(bool & execute)
{
	if (l->tk==LEX_ID ||
			l->tk==LEX_INT ||
			l->tk==LEX_FLOAT ||
			l->tk==LEX_STR ||
			l->tk=='-')
	{
		/* Execute a simple statement that only contains basic arithmetic... */
		CLEAN(base(execute));
		l->match(';');
	}
	else if (l->tk=='{')
	{
		/* A block of code */
		block(execute);
	}
	else if (l->tk==';')
	{
		/* Empty statement - to allow things like ;;; */
		l->match(';');
	}
	else if (l->tk==LEX_R_VAR)
	{
		/* variable creation. TODO - we need a better way of parsing the left
		 * hand side. Maybe just have a flag called can_create_var that we
		 * set and then we parse as if we're doing a normal equals.*/
		l->match(LEX_R_VAR);
		while (l->tk != ';')
		{
			VarLink * a = 0;
			if (execute)
				a = scopes.back()->findChildOrCreate(l->tkStr);
			l->match(LEX_ID);
			// now do stuff defined with dots
			while (l->tk == '.')
			{
				l->match('.');
				if (execute)
				{
					VarLink * lastA = a;
					a = lastA->var->findChildOrCreate(l->tkStr);
				}
				l->match(LEX_ID);
			}
			// sort out initialiser
			if (l->tk == '=')
			{
				l->match('=');
				VarLink * var = base(execute);
				if (execute)
					a->replaceWith(var);
				CLEAN(var);
			}
			if (l->tk != ';')
				l->match(',');
		}
		l->match(';');
	}
	else if (l->tk==LEX_R_IF)
	{
		l->match(LEX_R_IF);
		l->match('(');
		VarLink * var = base(execute);
		l->match(')');
		bool cond = execute && var->var->getBool();
		CLEAN(var);
		bool noexecute = false; // because we need to be abl;e to write to it
		statement(cond ? execute : noexecute);
		if (l->tk==LEX_R_ELSE)
		{
			l->match(LEX_R_ELSE);
			statement(cond ? noexecute : execute);
		}
	}
	else if (l->tk==LEX_R_WHILE)
	{
		// We do repetition by pulling out the string representing our statement
		// there's definitely some opportunity for optimisation here
		l->match(LEX_R_WHILE);
		l->match('(');
		int whileCondStart = l->tokenStart;
		bool noexecute = false;
		VarLink * cond = base(execute);
		bool loopCond = execute && cond->var->getBool();
		CLEAN(cond);
		Lexer * whileCond = l->getSubLex(whileCondStart);
		l->match(')');
		int whileBodyStart = l->tokenStart;
		statement(loopCond ? execute : noexecute);
		Lexer * whileBody = l->getSubLex(whileBodyStart);
		Lexer * oldLex = l;
		int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		while (loopCond && loopCount-->0)
		{
			whileCond->reset();
			l = whileCond;
			cond = base(execute);
			loopCond = execute && cond->var->getBool();
			CLEAN(cond);
			if (loopCond)
			{
				whileBody->reset();
				l = whileBody;
				statement(execute);
			}
		}
		l = oldLex;
		delete whileCond;
		delete whileBody;

		if (loopCount<=0)
		{
			root->trace();
			TRACE("WHILE Loop exceeded %d iterations at %s\n", TINYJS_LOOP_MAX_ITERATIONS, l->getPosition().c_str());
			throw new Exception("LOOP_ERROR");
		}
	}
	else if (l->tk==LEX_R_FOR)
	{
		l->match(LEX_R_FOR);
		l->match('(');
		statement(execute); // initialisation
		//l->match(';');
		int forCondStart = l->tokenStart;
		bool noexecute = false;
		VarLink * cond = base(execute); // condition
		bool loopCond = execute && cond->var->getBool();
		CLEAN(cond);
		Lexer * forCond = l->getSubLex(forCondStart);
		l->match(';');
		int forIterStart = l->tokenStart;
		CLEAN(base(noexecute)); // iterator
		Lexer * forIter = l->getSubLex(forIterStart);
		l->match(')');
		int forBodyStart = l->tokenStart;
		statement(loopCond ? execute : noexecute);
		Lexer * forBody = l->getSubLex(forBodyStart);
		Lexer * oldLex = l;
		if (loopCond)
		{
			forIter->reset();
			l = forIter;
			CLEAN(base(execute));
		}
		int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		while (execute && loopCond && loopCount-->0)
		{
			forCond->reset();
			l = forCond;
			cond = base(execute);
			loopCond = cond->var->getBool();
			CLEAN(cond);
			if (execute && loopCond)
			{
				forBody->reset();
				l = forBody;
				statement(execute);
			}
			if (execute && loopCond)
			{
				forIter->reset();
				l = forIter;
				CLEAN(base(execute));
			}
		}
		l = oldLex;
		delete forCond;
		delete forIter;
		delete forBody;
		if (loopCount<=0)
		{
			root->trace();
			TRACE("FOR Loop exceeded %d iterations at %s\n", TINYJS_LOOP_MAX_ITERATIONS, l->getPosition().c_str());
			throw new Exception("LOOP_ERROR");
		}
	}
	else if (l->tk==LEX_R_RETURN)
	{
		l->match(LEX_R_RETURN);
		VarLink * result = 0;
		if (l->tk != ';')
			result = base(execute);
		if (execute)
		{
			VarLink * resultVar = scopes.back()->findChild(TINYJS_RETURN_VAR);
			if (resultVar)
				resultVar->replaceWith(result);
			else
				TRACE("RETURN statement, but not in a function.\n");
			execute = false;
		}
		CLEAN(result);
		l->match(';');
	}
	else if (l->tk==LEX_R_FUNCTION)
	{
		VarLink * funcVar = parseFunctionDefinition();
		if (execute)
		{
			if (funcVar->name == TINYJS_TEMP_NAME)
				TRACE("Functions defined at statement-level are meant to have a name\n");
			else
				scopes.back()->addChildNoDup(funcVar->name, funcVar->var);
		}
		CLEAN(funcVar);
	}
	else l->match(LEX_EOF);
}

/// Get the given variable specified by a path (var1.var2.etc), or return 0
Var * Engine::getScriptVariable(const string & path)
{
	// traverse path
	size_t prevIdx = 0;
	size_t thisIdx = path.find('.');
	if (thisIdx == string::npos) thisIdx = path.length();
	Var * var = root;
	while (var && prevIdx<path.length())
	{
		string el = path.substr(prevIdx, thisIdx-prevIdx);
		VarLink * varl = var->findChild(el);
		var = varl?varl->var:0;
		prevIdx = thisIdx+1;
		thisIdx = path.find('.', prevIdx);
		if (thisIdx == string::npos) thisIdx = path.length();
	}
	return var;
}

/// Get the value of the given variable, or return 0
const string * Engine::getVariable(const string & path)
{
	Var * var = getScriptVariable(path);
	// return result
	if (var)
		return &var->getString();
	else
		return 0;
}

/// set the value of the given variable, return trur if it exists and gets set
bool Engine::setVariable(const std::string & path, const std::string & varData)
{
	Var * var = getScriptVariable(path);
	// return result
	if (var)
	{
		if (var->isInt())
			var->setInt((int)strtol(varData.c_str(),0,0));
		else if (var->isDouble())
			var->setDouble(strtod(varData.c_str(),0));
		else
			var->setString(varData.c_str());
		return true;
	}
	else
		return false;
}

/// Finds a child, looking recursively up the scopes
VarLink * Engine::findInScopes(const std::string & childName)
{
	for (int s=scopes.size()-1; s>=0; s--)
	{
		VarLink * v = scopes[s]->findChild(childName);
		if (v) return v;
	}
	return NULL;

}

/// Look up in any parent classes of the given object
VarLink * Engine::findInParentClasses(Var * object, const std::string & name)
{
	// Look for links to actual parent classes
	VarLink * parentClass = object->findChild(TINYJS_PROTOTYPE_CLASS);
	while (parentClass)
	{
		VarLink * implementation = parentClass->var->findChild(name);
		if (implementation) return implementation;
		parentClass = parentClass->var->findChild(TINYJS_PROTOTYPE_CLASS);
	}
	// else fake it for strings and finally objects
	if (object->isString())
	{
		VarLink * implementation = stringClass->findChild(name);
		if (implementation) return implementation;
	}
	if (object->isArray())
	{
		VarLink * implementation = arrayClass->findChild(name);
		if (implementation) return implementation;
	}
	VarLink * implementation = objectClass->findChild(name);
	if (implementation) return implementation;

	return 0;
}
