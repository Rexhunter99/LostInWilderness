#pragma once

#include <stdint.h>
#include <map>
#include <string>


/** @class Config
 ** Create a configuration/settings structure in memory that can be
 ** saved directly to the file system and loaded in from the file
 ** system when necessary
 */
class Config
{
private:

	static Config __c;
	std::map<std::string,std::string>	m_items;

public:

	/** @fn getGlobal()
	 ** @return static Config*
	 ** Get a pointer to the 'global' configuration structure
	 **/
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
