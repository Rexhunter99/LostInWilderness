#pragma once

#include <string>
#include <cstdint>

enum BlockInfoPropertiesEnum
{
	BI_NONE					= 0,
	BI_TRANSLUCENT			= 1,
	BI_CAST_SHADOW			= 2,
	BI_OPACITY				= 4,
	BI_LIGHT_SOURCE			= 8,
};

class BlockInfo
{
protected:

public:

	BlockInfo()
	{
		// -- Default block infos are constructed as 'air' objects
		this->name			= "";
		this->locale_name	= "";
		this->ID			= 0;
		this->flags			= BI_NONE;
		this->hardness		= 0;
	}

	BlockInfo( std::string new_name, uint16_t new_ID, uint32_t new_flags )
	{
		this->name			= new_name;
		this->locale_name	= new_name;
		this->ID			= new_ID;
		this->flags			= new_flags;
	}

	std::string				name;
	std::string				locale_name;
	uint16_t				ID;
	uint32_t				flags;
	uint8_t					hardness;


	bool operator == ( BlockInfo block ) const
	{
		if ( block.name == this->name &&
			 block.locale_name == this->locale_name &&
			 block.ID == this->ID &&
			 block.flags == this->flags )
			return true;
		else
			return false;
	}
};
