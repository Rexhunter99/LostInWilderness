#pragma once

#include <string>
#include <vector>
#include <stdint.h>


class ItemInfo
{
public:

  /** @var name
   ** The internal name of the item for use in string identification as opposed to using the numerical ID */
	std::string		        name;
	/** @var locale_name
   ** The name of the item in the locale that the player has chosen to play in. (Language) */
	std::wstring	        locale_name;
	/** @var id
	 ** The unique numerical ID of the item for use in identifiying it numerically or with data constraints */
	uint32_t		          id;
	/** @var data
	 ** A special area of data for use by the item author for whatever they please. */
  std::vector<uint8_t>  data;

  ItemInfo( std::string name, std::string locale_name, uint16_t id ) : name( name ), locale_name( locale_name ), id( id )
  {}

  ~ItemInfo();
};
