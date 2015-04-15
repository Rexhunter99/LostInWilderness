#pragma once

#include <exception>
#include <string>


class nullptr_exception : public std::exception
{
	const char * what() const throw()
	{
		return "A Null Pointer was passed but not expected.";
	}
};

class custom_exception : public std::exception
{
	std::string what_string;

	const char * what() const throw()
	{
		return what_string.c_str();
	}

public:

	custom_exception( std::string custom_what ) : what_string( custom_what )
	{}
};
