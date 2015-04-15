#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "BlockInfo.h"
#include "ItemInfo.h"
#include "Texture.h"
#include "TextFont.h"
#include "WorldGenerator.h"


class ResourceManager
{
public:
	BlockInfo air;
	std::map<std::string,BlockInfo>		block_info;
	std::map<std::string,ItemInfo>		item_info;
	std::map<std::string,Texture>		textures;
	std::map<std::string,TextFont>		fonts;
	std::vector<WorldGenerator*>		worldgens;

	ResourceManager();
	~ResourceManager();

	const BlockInfo &	getBlockInfo( std::string block_name );
	void				addBlockInfo( BlockInfo new_block_info );
	const ItemInfo &	getItemInfo( std::string item_name );
	void				addItemInfo( ItemInfo new_item_info );
	WorldGenerator &	getWorldGen( std::string worldgen_name );
	void				addWorldGen( WorldGenerator *new_worldgen );


	static ResourceManager *iResourceManager;
};
