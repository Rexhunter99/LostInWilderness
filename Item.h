
#pragma once

#include "ItemInfo.h"
#include <string>
#include <cstdint>

/** @class Item
 ** @brief World instance of the Item class.
 ** The world may have many items in it of one type, this class handles that abstraction.
 **/
class Item
{
protected:

public:

  /** @var info
   ** @brief Reference to ItemInfo instance.
   ** Reference to the item information and code that this instance of item draws from.
   **/
	ItemInfo        &info;

	/** @var name
   ** @brief Unicode string for this item instance
   ** The individual name of this particular item in the world space. This is unique to this instance.
   **/
	std::wstring		name;

  /** @fn Item( ItemInfo &info )
   ** The constructor that creates this item */
	Item( ItemInfo &info ) : info( info )
	{}

};
