/*
 * TinyJS
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 * Updated By Jamie Breeze <jamie.c.breeze@gmail.com>
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
#pragma once

#if !defined(__TINYJS_LEXER_H__)
#define __TINYJS_LEXER_H__

namespace JS {

enum LEX_TYPES
{
	LEX_EOF = 0,
	LEX_ID = 256,
	LEX_INT,
	LEX_FLOAT,
	LEX_STR,

	LEX_EQUAL,
	LEX_TYPEEQUAL,
	LEX_NEQUAL,
	LEX_NTYPEEQUAL,
	LEX_LEQUAL,
	LEX_LSHIFT,
	LEX_LSHIFTEQUAL,
	LEX_GEQUAL,
	LEX_RSHIFT,
	LEX_RSHIFTUNSIGNED,
	LEX_RSHIFTEQUAL,
	LEX_PLUSEQUAL,
	LEX_MINUSEQUAL,
	LEX_PLUSPLUS,
	LEX_MINUSMINUS,
	LEX_ANDEQUAL,
	LEX_ANDAND,
	LEX_OREQUAL,
	LEX_OROR,
	LEX_XOREQUAL,
	// reserved words
#define LEX_R_LIST_START LEX_R_IF
	LEX_R_IF,
	LEX_R_ELSE,
	LEX_R_DO,
	LEX_R_WHILE,
	LEX_R_FOR,
	LEX_R_BREAK,
	LEX_R_CONTINUE,
	LEX_R_FUNCTION,
	LEX_R_RETURN,
	LEX_R_VAR,
	LEX_R_TRUE,
	LEX_R_FALSE,
	LEX_R_NULL,
	LEX_R_UNDEFINED,
	LEX_R_NEW,

	LEX_R_LIST_END /* always the last entry */
};


/**
 * \class Lexer
 * \brief A string lexer class for parsing the JavaScript
 *
 * A string lexer class used in parsing the JavaScript text into
 * useable 'program' code. */
class Lexer
{
public:
	Lexer(const std::string & input);
	Lexer(Lexer * owner, int startChar, int endChar);
	~Lexer(void);

	char currCh, nextCh;
	int tk; ///< The type of the token that we have
	int tokenStart; ///< Position in the data at the beginning of the token we have here
	int tokenEnd; ///< Position in the data at the last character of the token we have here
	int tokenLastEnd; ///< Position in the data at the last character of the last token
	std::string tkStr; ///< Data contained in the token we have here

	void match(int expected_tk); ///< Lexical match wotsit
	static std::string getTokenStr(int token); ///< Get the string representation of the given token
	void reset(); ///< Reset this lex so we can start again

	std::string getSubString(int pos); ///< Return a sub-string from the given position up until right now
	Lexer * getSubLex(int lastPosition); ///< Return a sub-lexer from the given position up until right now

	std::string getPosition(int pos=-1); ///< Return a string representing the position in lines and columns of the character pos given

protected:
	/* When we go into a loop, we use getSubLex to get a lexer for just the sub-part of the
	   relevant string. This doesn't re-allocate and copy the string, but instead copies
	   the data pointer and sets dataOwned to false, and dataStart/dataEnd to the relevant things. */
	char * data; ///< Data string to get tokens from
	int dataStart, dataEnd; ///< Start and end position in data string
	bool dataOwned; ///< Do we own this data string?

	int dataPos; ///< Position in data (we CAN go past the end of the string here)

	void getNextCh();
	void getNextToken(); ///< Get the text token from our text string
};


}; // namespace JS

#endif
