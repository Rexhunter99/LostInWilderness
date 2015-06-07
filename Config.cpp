
#include "Config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <memory.h>
#include <cstdlib>

typedef std::map<std::string,std::string> keyval_t;


Config::Config()
{
	this->m_items.clear();
}

Config::~Config()
{
	this->m_items.clear();
}

bool Config::save( std::string file_name )
{
	std::fstream f;
	f.open( file_name.c_str(), std::fstream::out );

	if ( f.is_open() )
	{
		for ( keyval_t::iterator i = this->m_items.begin(); i!=this->m_items.end(); i++ )
		{
			f << i->first << "=" << i->second << std::endl;
		}

		f.close();
		return true;
	}

	return false;
}

bool Config::load( std::string file_name )
{
	std::fstream f;
	f.open( file_name.c_str(), std::fstream::in );

	if ( f.is_open() )
	{
		while ( !f.eof() )
		{
			char key[256];
			char value[256];
			memset( key, 0, 255 );
			memset( value, 0, 255 );

			f.getline( key, 255, '=' );
			f.getline( value, 255 );

			// -- Abort this line if it has the comment pre-pends
			if ( key[0] == '#' ) continue;
			if ( key[0] == ';' ) continue;
			if ( key[0] == '/' && key[1] == '/' ) continue;

			this->m_items[ std::string(key) ] = std::string(value);
		}

		f.close();

		return true;
	}

	return false;
}


uint64_t Config::getInteger( std::string key )
{
	std::string s_value = this->getString( key );
	s_value = ( s_value=="" ) ? "0" : s_value;
	return std::strtol( s_value.c_str(), nullptr, 10 );
}

double Config::getFloat( std::string key )
{
	std::string s_value = this->getString( key );
	s_value = ( s_value=="" ) ? "0.0" : s_value;
	return std::strtod( s_value.c_str(), nullptr );
}

std::string Config::getString( std::string key )
{
	keyval_t::iterator i = this->m_items.find( key );

	if ( i == this->m_items.end() )
		return "";
	else
		return i->second;
}

void Config::setInteger( std::string key, uint64_t value )
{
	std::stringstream s_value;
	s_value << value;

	this->setString( key, s_value.str() );
}

void Config::setFloat( std::string key, double value )
{
	std::stringstream s_value;
	s_value << value;

	this->setString( key, s_value.str() );
}

void Config::setString( std::string key, std::string value )
{
	// TODO: Find if the key exists, if it does, replace the key value instead
	this->m_items.insert( keyval_t::value_type( key, value ) );
}
