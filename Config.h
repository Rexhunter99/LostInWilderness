#pragma once

#include <stdint.h>
#include <map>
#include <string>


class Config
{
private:

	static Config __c;
	std::map<std::string,std::string>	m_items;

public:

	static Config * getGlobal() { return &__c; }

	Config();
	~Config();

	bool		save( std::string file_name );
	bool		load( std::string file_name );

	uint64_t	getInteger( std::string key );
	double		getFloat( std::string key );
	std::string	getString( std::string key );

	void		setInteger( std::string key, uint64_t value );
	void		setFloat( std::string key, double value );
	void		setString( std::string key, std::string value );

};
