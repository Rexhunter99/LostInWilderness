
#include "ResourceManager.h"

ResourceManager *ResourceManager::iResourceManager;


ResourceManager::ResourceManager()
{
	// Add block info to the dictionary
	this->addBlockInfo( BlockInfo( "", 0, 0 ) );		// -- Air block
	this->addBlockInfo( BlockInfo( "dirt", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "dirt-grass", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "grass", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "leaves", 0, BI_OPACITY ) );
	this->addBlockInfo( BlockInfo( "wood", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "stone", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "sand", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "salt_water", 0, BI_TRANSLUCENT ) );
	this->addBlockInfo( BlockInfo( "glass", 0, BI_OPACITY ) );
	this->addBlockInfo( BlockInfo( "brick", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "ore", 0, 0 ) );
	this->addBlockInfo( BlockInfo( "fresh_water", 0, BI_TRANSLUCENT ) );
	// End block info

	// World Generators
	this->addWorldGen( new WorldGenerator() );
	// End World Generators
}

ResourceManager::~ResourceManager()
{

}

const BlockInfo & ResourceManager::getBlockInfo( std::string block_name )
{
	std::map<std::string,BlockInfo>::iterator it = block_info.find( block_name );

	if ( it == block_info.end() )
		return air;
	else
		return it->second;
}

const BlockInfo & ResourceManager::getBlockInfo( size_t block_id )
{
	for ( auto it = block_info.begin(); it != block_info.end(); it++ )
	{
		if ( it->second.ID == block_id )
		{
			return it->second;
		}
	}

	return air;
}

void ResourceManager::addBlockInfo( BlockInfo new_block_info )
{
	// -- IDs will be set to the index that the instance is in the map
	if ( new_block_info.ID == 0 )
		new_block_info.ID = block_info.size();

	block_info.insert( std::map<std::string,BlockInfo>::value_type( new_block_info.name, new_block_info ) );
}

// -- TODO: ItemInfo


WorldGenerator & ResourceManager::getWorldGen( std::string worldgen_name )
{
	WorldGenerator *wg = this->worldgens[0];

	for ( size_t i=1; i<this->worldgens.size(); i++ )
	{
		if ( this->worldgens[i]->name == worldgen_name )
		{
			wg = this->worldgens[i];
			break;
		}
	}

	return *wg;
}

void ResourceManager::addWorldGen( WorldGenerator *new_worldgen )
{
	if ( new_worldgen == nullptr ) return;
	this->worldgens.push_back( new_worldgen );
}
