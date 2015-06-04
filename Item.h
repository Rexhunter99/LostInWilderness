
#pragma once

#include "ItemInfo.h"
#include <string>
#include <cstdint>

class Item
{
protected:

public:

	ItemInfo&			info;
	uint16_t			id;

	float				x, y, z;

	Item( ItemInfo& item, id, x, y, z );
	~Item();

};
