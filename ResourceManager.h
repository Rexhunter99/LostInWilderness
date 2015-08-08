#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "BlockInfo.h"
#include "ItemInfo.h"
#include "Texture.h"
#include "TextFont.h"
#include "Shader.h"
#include "VertexFormat.h"
#include "WorldGenerator.h"


/** @class ResourceManager
 **
 **
 **/
class ResourceManager
{
public:
	BlockInfo air;
	ItemInfo none;
	std::map<std::string,BlockInfo>		block_info;
	std::map<std::string,ItemInfo>		item_info;
	std::map<std::string,Texture*>		textures;
	std::map<std::string,TextFont*>		fonts;
	std::map<std::string,Shader*>		shaders;
	std::map<std::string,VertexFormat*>	vertex_formats;
	std::vector<WorldGenerator*>		worldgens;

	ResourceManager();
	~ResourceManager();

	BlockInfo &			getBlockInfo( std::string block_name );
	BlockInfo &			getBlockInfo( size_t block_id );
	void				addBlockInfo( BlockInfo new_block_info );
	ItemInfo &			getItemInfo( std::string item_name );
	ItemInfo &			getItemInfo( size_t item_id);
	void				addItemInfo( ItemInfo new_item_info );
	WorldGenerator &	getWorldGen( std::string worldgen_name );
	void				addWorldGen( WorldGenerator *new_worldgen );
	Texture *			getTexture( std::string texture_name );
	void				addTexture( const std::string & name, Texture *new_texture );
	TextFont *			getFont( std::string font_name );
	void				addFont( const std::string & name, TextFont *new_font );
	Shader *			getShader( std::string name );
	void				addShader( const std::string & name, Shader *new_shader );
	VertexFormat *		getVertexFormat( const std::string & name );
	void				addVertexFormat( const std::string & name, VertexFormat *vf );


	static ResourceManager *iResourceManager;
	static ResourceManager * getInstance() { return iResourceManager; }
};
