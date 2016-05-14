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

#if !defined(__TINYJS_H__)
#define __TINYJS_H__

#if defined(_DEBUG)
// If defined, this keeps a note of all calls and where from in memory. This is slower, but good for debugging
#define TINYJS_CALL_STACK
#endif //_DEBUG

#if defined(_WIN32)
#	if defined(_DEBUG)
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#	endif
#endif

#include <string>
#include <vector>

#if defined(_MSC_VER)
#	if defined(_DEBUG) && defined(_DEBUG_NEW)
#		define _AFXDLL
#		include <afx.h>
#		define new DEBUG_NEW
#	endif
#	define DEPRECATED(_Text) __declspec(deprecated(_Text))
#elif defined(__GNUC__)
#	define DEPRECATED(_Text) __attribute__ ((deprecated))
#else
#	define DEPRECATED(_Text)
#endif

#ifndef TRACE
#	define TRACE printf
#endif // TRACE


#ifndef TINYJS_LOOP_MAX_ITERATIONS
#	define TINYJS_LOOP_MAX_ITERATIONS 8192
#endif // TINYJS_LOOP_MAX_ITERATIONS

namespace JS {

enum SCRIPTVAR_FLAGS
{
	SCRIPTVAR_UNDEFINED   = 0,
	SCRIPTVAR_FUNCTION    = 1,
	SCRIPTVAR_OBJECT      = 2,
	SCRIPTVAR_ARRAY       = 4,
	SCRIPTVAR_DOUBLE      = 8,  // floating point double
	SCRIPTVAR_INTEGER     = 16, // integer number
	SCRIPTVAR_STRING      = 32, // string
	SCRIPTVAR_NULL        = 64, // it seems null is its own data type

	SCRIPTVAR_NATIVE      = 128, // to specify this is a native function

	SCRIPTVAR_NUMERICMASK = SCRIPTVAR_NULL |
							SCRIPTVAR_DOUBLE |
							SCRIPTVAR_INTEGER,
	SCRIPTVAR_VARTYPEMASK = SCRIPTVAR_DOUBLE |
							SCRIPTVAR_INTEGER |
							SCRIPTVAR_STRING |
							SCRIPTVAR_FUNCTION |
							SCRIPTVAR_OBJECT |
							SCRIPTVAR_ARRAY |
							SCRIPTVAR_NULL,

};

#define TINYJS_RETURN_VAR "return"
#define TINYJS_PROTOTYPE_CLASS "prototype"
#define TINYJS_TEMP_NAME ""
#define TINYJS_BLANK_DATA ""

class Lexer;
class Var;

///< Type definition for the JavaScript Callback to native code
typedef void (*JSCallback)(Var * var, void * userdata);

///< Convert the given string into a quoted string suitable for javascript
std::string getJSString(const std::string & str);


class Exception
{
protected:
	friend class Engine;

	std::string text;

public:
	Exception(const std::string & exceptionText);
	const std::string & what() const;
};


class VarLink
{
public:
	std::string name;
	VarLink * nextSibling;
	VarLink * prevSibling;
	Var * var;
	bool owned;

	VarLink(Var * var, const std::string & name = TINYJS_TEMP_NAME);
	VarLink(const VarLink & link); ///< Copy constructor
	~VarLink();
	void replaceWith(Var * newVar); ///< Replace the Variable pointed to
	void replaceWith(VarLink * newVar); ///< Replace the Variable pointed to (just dereferences)
	int getIntName(); ///< Get the name as an integer (for arrays)
	void setIntName(int n); ///< Set the name as an integer (for arrays)
};


/**
 * Variable class (containing a doubly-linked list of children) */
class Var
{
public:
	Var(); ///< Create undefined
	Var(const std::string & varData, int varFlags); ///< User defined
	Var(const std::string & str); ///< Create a string
	Var(double varData);
	Var(int val);
	~Var(void);

	Var * getReturnVar(); ///< If this is a function, get the result value (for use by native functions)
	void setReturnVar(Var * var); ///< Set the result value. Use this when setting complex return data as it avoids a deepCopy()
	Var * getParameter(const std::string & name); ///< If this is a function, get the parameter with the given name (for use by native functions)

	VarLink * findChild(const std::string & childName); ///< Tries to find a child with the given name, may return 0
	VarLink * findChildOrCreate(const std::string & childName, int varFlags=SCRIPTVAR_UNDEFINED); ///< Tries to find a child with the given name, or will create it with the given flags
	VarLink * findChildOrCreateByPath(const std::string & path); ///< Tries to find a child with the given path (separated by dots)
	VarLink * addChild(const std::string & childName, Var * child=NULL);
	VarLink * addChildNoDup(const std::string & childName, Var * child=NULL); ///< add a child overwriting any with the same name
	void removeChild(Var * child);
	void removeLink(VarLink * link); ///< Remove a specific link (this is faster than finding via a child)
	void removeAllChildren();
	Var * getArrayIndex(int idx); ///< The the value at an array index
	void setArrayIndex(int idx, Var * value); ///< Set the value at an array index
	int getArrayLength(); ///< If this is an array, return the number of items in it (else 0)
	int getChildren(); ///< Get the number of children

	int getInt();
	bool getBool()
	{
		return getInt() != 0;
	}
	double getDouble();
	const std::string & getString();
	std::string getParsableString(); ///< get Data as a parsable javascript string
	void setInt(int num);
	void setDouble(double val);
	void setString(const std::string & str);
	void setUndefined();
	void setArray();
	bool equals(Var * v);

	bool isInt()
	{
		return (flags&SCRIPTVAR_INTEGER)!=0;
	}
	bool isDouble()
	{
		return (flags&SCRIPTVAR_DOUBLE)!=0;
	}
	bool isString()
	{
		return (flags&SCRIPTVAR_STRING)!=0;
	}
	bool isNumeric()
	{
		return (flags&SCRIPTVAR_NUMERICMASK)!=0;
	}
	bool isFunction()
	{
		return (flags&SCRIPTVAR_FUNCTION)!=0;
	}
	bool isObject()
	{
		return (flags&SCRIPTVAR_OBJECT)!=0;
	}
	bool isArray()
	{
		return (flags&SCRIPTVAR_ARRAY)!=0;
	}
	bool isNative()
	{
		return (flags&SCRIPTVAR_NATIVE)!=0;
	}
	bool isUndefined()
	{
		return (flags & SCRIPTVAR_VARTYPEMASK) == SCRIPTVAR_UNDEFINED;
	}
	bool isNull()
	{
		return (flags & SCRIPTVAR_NULL)!=0;
	}
	bool isBasic()
	{
		return firstChild==0;    ///< Is this *not* an array/object/etc
	}

	Var * mathsOp(Var * b, int op); ///< do a maths op with another script variable
	void copyValue(Var * val); ///< copy the value from the value given
	Var * deepCopy(); ///< deep copy this node and return the result

	void trace(std::string indentStr = "", const std::string & name = ""); ///< Dump out the contents of this using trace
	std::string getFlagsAsString(); ///< For debugging - just dump a string version of the flags
	void getJSON(std::ostringstream & destination, const std::string linePrefix=""); ///< Write out all the JS code needed to recreate this script variable to the stream (as JSON)
	void setCallback(JSCallback callback, void * userdata); ///< Set the callback for native functions

	VarLink * firstChild;
	VarLink * lastChild;

	/// For memory management/garbage collection
	Var * ref(); ///< Add reference to this variable
	void unref(); ///< Remove a reference, and delete this variable if required
	int getRefs(); ///< Get the number of references to this script variable

protected:
	int refs; ///< The number of references held to this - used for garbage collection

	std::string data; ///< The contents of this variable if it is a string
	long intData; ///< The contents of this variable if it is an int
	double doubleData; ///< The contents of this variable if it is a double
	int flags; ///< the flags determine the type of the variable - int/double/string/etc
	JSCallback jsCallback; ///< Callback for native functions
	void * jsCallbackUserData; ///< user data passed as second argument to native functions

	void init(); ///< initialisation of data members

	/** Copy the basic data and flags from the variable given, with no
	  * children. Should be used internally only - by copyValue and deepCopy */
	void copySimpleData(Var * val);

	friend class Engine;
};


/**
 * \class VarPtr
 *
 * A scope-safe way of referencing a Var instance */
class VarPtr
{
private:
	Var * var;

public:

	// Constructors
	VarPtr() : var(nullptr) {}
	VarPtr(Var * v) : var(v) { if (var) var->ref(); }
	VarPtr & operator= (Var * v)
	{
		if (var != v ) {
			if (var) var->unref();
			var = v;
			if ( var ) var->ref();
		}
		return *this;
	}

	// Copy
	VarPtr(const VarPtr & src) : var(src.var) {
		if ( var )
			var->ref();
	}
	VarPtr & operator= (const VarPtr & src)
	{
		if (var != src.var ) {
			if (var) var->unref();
			var = src.var;
			if ( var ) var->ref();
		}
		return *this;
	}

	// Destructors
	~VarPtr() { if (var) var->unref(); }

	bool operator== (const VarPtr & r) const { return var == r.var; }
	bool operator== (const Var * v) const { return var == v; }

	operator bool() const { return var!=0; }

	// Access
	Var * operator ->() const { return var; }
	Var * getVar() const { return var; }

	void clear() { if (var) var->unref(); var = nullptr; }
};


class Engine
{
public:
	Engine();
	~Engine();

	/**
	 * \brief Execute a JavaScript application from a string stored in memory
	 * \param code A reference to a C++ string with the JavaScript code to execute
	 *
	 * Execute a JavaScript application from the string stored in memory,
	 * this operates on the engine's root Object as the global instance,
	 * retrieve any computed values or code from there. */
	void execute(const std::string & code);

	/**
	 * \brief Evaluate the given JavaScript application and return a link to the root Object
	 * \param code A reference to a C++ string with the JavaScript code to evaluate
	 *
	 * Evaluate the given code and return a link to a javascript Object,
	 * useful for (dangerous) JSON parsing. If nothing to return, will return
	 * \c undefined variable type. VarLink is returned as this will
	 * automatically unref the result as it goes out of scope. If you want to
	 * keep it, you must use \c ref() and \c unref() */
	VarLink evaluateComplex(const std::string & code);

	/**
	 * \brief Evaluate the given JavaScript code and return a string result
	 * \return Returns a string with the result of the code evaluation, or "undefined" if nothing to return
	 *
	 * Evaluate the given JavaScript code and return a string. If there is no result to
	 * return, this method will return the \c "undefined" string */
	std::string evaluate(const std::string & code);

	/**
	 * \brief Add a native function to be called from TinyJS
	 * \param funcDesc	A string literal of the function symbol
	 * \param ptr		A pointer to the native callback the JavaScript symbol is intended for
	 * \param userdata	A pointer to user-specific data for the callback
	 *
	 * Add a function symbol to the JavaScript engine that executes native code when called.
	 * This is useful when you need JavaScript to interface directly with elements in your C++
	 * application such as a real-time callback event. */
	void addNative(const std::string & funcDesc, JSCallback ptr, void * userdata);

	/**
	 * \brief Get the given variable specified by a path (var1.var2.etc), or return `nullptr`
	 * \param path The variable path + symbol to get
	 *
	 * Get the variable specified by the given argument \c */
	Var * getScriptVariable(const std::string & path);

	/// Get the value of the given variable, or return 0
	const std::string * getVariable(const std::string & path);

	/// set the value of the given variable, return trur if it exists and gets set
	bool setVariable(const std::string & path, const std::string & varData);

	/// Send all variables to stdout
	void trace();

	Var * root;  /// root of symbol table
private:
	Lexer * l;            /// current lexer
	std::vector<Var *> scopes; /// stack of scopes when parsing
#ifdef TINYJS_CALL_STACK
	std::vector<std::string> call_stack; /// Names of places called so we can show when erroring
#endif

	Var * stringClass; /// Built in string class
	Var * objectClass; /// Built in object class
	Var * arrayClass; /// Built in array class

	// parsing - in order of precedence
	VarLink * functionCall(bool & execute, VarLink * function, Var * parent);
	VarLink * factor(bool & execute);
	VarLink * unary(bool & execute);
	VarLink * term(bool & execute);
	VarLink * expression(bool & execute);
	VarLink * shift(bool & execute);
	VarLink * condition(bool & execute);
	VarLink * logic(bool & execute);
	VarLink * ternary(bool & execute);
	VarLink * base(bool & execute);
	void block(bool & execute);
	void statement(bool & execute);
	// parsing utility functions
	VarLink * parseFunctionDefinition();
	void parseFunctionArguments(Var * funcVar);

	VarLink * findInScopes(const std::string & childName); ///< Finds a child, looking recursively up the scopes
	/// Look up in any parent classes of the given object
	VarLink * findInParentClasses(Var * object, const std::string & name);
};

}; // namespace JS;

#endif
