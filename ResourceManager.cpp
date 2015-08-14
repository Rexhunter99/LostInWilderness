
#include "ResourceManager.h"
#include "Renderer.h"

ResourceManager *ResourceManager::iResourceManager;


ResourceManager::ResourceManager()
{

	// Add block info to the dictionary
	this->addBlockInfo(new BlockInfo( "", 0, 0 ) );		// -- Air block
	this->addBlockInfo(new BlockInfo( "liw:soil", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "dirt-grass", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "grass", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "liw:leaves", 0, BI_OPACITY ) );
	this->addBlockInfo(new BlockInfo( "liw:wood", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "liw:stone", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "liw:sand", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "liw:salt_water", 0, BI_TRANSLUCENT ) );
	this->addBlockInfo(new BlockInfo( "glass", 0, BI_OPACITY ) );
	this->addBlockInfo(new BlockInfo( "brick", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "liw:bedrock", 0, 0 ) );
	this->addBlockInfo(new BlockInfo( "fresh_water", 0, BI_TRANSLUCENT ) );
	this->addBlockInfo(new BlockInfo( "liw:ore_coal", 0, 0 ) );
	// End block info

	// World Generators
	this->addWorldGen( new WorldGenerator() );
	// End World Generators

	// Textures
	Texture *t = nullptr;
	this->addTexture( "blocks", t = new Texture( "../data/textures/gc-textures.png", 16, 16 ) );

	Renderer::blocks_texture = this->getTexture("blocks");
	Renderer::blocks_texture->bind();
	// End Textures

	// Fonts
	TextFont *f = nullptr;
	this->addFont( "default", f = new TextFont( "../data/fonts/minecraft" ) );

	Renderer::font_texture = this->getFont("default");
	// End Fonts

	// Shaders
	Shader *s = nullptr;
	this->addShader( "default", new Shader() );
	s = this->getShader("default");
	s->addVertexShader( "../data/shaders/world.150.vs" );
	s->addFragmentShader( "../data/shaders/world.150.fs" );
	s->compileShaders();
	s->bind();

	// -- Make the shader object aware of attributes and uniforms
	s->addAttrib( "v_position" );
	s->addAttrib( "v_normal" );
	s->addAttrib( "v_texcoord" );
	s->addAttrib( "v_diffuse" );
	s->addUniform( "mvp" );
	s->addUniform( "texture" );
	s->addUniform( "b_lighting" );
	s->addUniform( "g_SunLightSource.position" );
	s->addUniform( "g_SunLightSource.diffuse" );
	s->addUniform( "g_SunLightSource.ambient" );
	s->addUniform( "g_SunLightSource.specular" );

	s->bindFragData( 0, "frag_color" );
	// End Shaders

	// Vertex Formats
	VertexFormat *vf = new VertexFormat();
	this->addVertexFormat( "default", vf );
	vf->bind();
	/*vf->enableVertexAttrib( s->getAttrib("a_Position") );
	vf->enableVertexAttrib( s->getAttrib("a_Normal") );
	vf->enableVertexAttrib( s->getAttrib("a_TexCoord") );
	vf->enableVertexAttrib( s->getAttrib("a_Diffuse") );*/
	vf->enableVertexAttrib( s->getAttrib( "v_position" ) );
	vf->enableVertexAttrib( s->getAttrib( "v_normal" ) );
	vf->enableVertexAttrib( s->getAttrib( "v_texcoord" ) );
	vf->enableVertexAttrib( s->getAttrib( "v_diffuse" ) );
	// End Vertex Formats

	Renderer::default_shader = this->getShader( "default" );
}

ResourceManager::~ResourceManager()
{
	for (auto i = this->block_info.begin(); i != this->block_info.end(); i++)
	{
		delete i->second;
	}

	for (auto i = this->item_info.begin(); i != this->item_info.end(); i++)
	{
		delete i->second;
	}

	for (auto i=this->textures.begin(); i!=this->textures.end(); i++)
	{
		delete i->second;
	}

	for (auto i=this->fonts.begin(); i!=this->fonts.end(); i++)
	{
		delete i->second;
	}

	for (auto i=this->shaders.begin(); i!=this->shaders.end(); i++)
	{
		delete i->second;
	}

	this->block_info.clear();
	this->item_info.clear();
	this->textures.clear();
	this->fonts.clear();
	this->shaders.clear();
}

BlockInfo & ResourceManager::getBlockInfo( std::string block_name )
{
	std::map<std::string,BlockInfo*>::iterator it = block_info.find( block_name );

	if ( it == block_info.end() )
		return air;
	else
		return *it->second;
}

BlockInfo & ResourceManager::getBlockInfo( size_t block_id )
{
	for ( auto it = block_info.begin(); it != block_info.end(); it++ )
	{
		if ( it->second->ID == block_id )
		{
			return *it->second;
		}
	}

	return air;
}

void ResourceManager::addBlockInfo( BlockInfo * new_block_info )
{
	// -- IDs will be set to the index that the instance is in the map
	if ( new_block_info->ID == 0 )
		new_block_info->ID = block_info.size();

	block_info.insert( std::make_pair( new_block_info->name, new_block_info ) );
}

ItemInfo & ResourceManager::getItemInfo( std::string item_name )
{
	std::map<std::string,ItemInfo*>::iterator it = item_info.find( item_name );

    // Item not found after reaching end of item_info map
	if ( it == item_info.end() )
		return none;
	else
		return *it->second;
}

ItemInfo & ResourceManager::getItemInfo( size_t item_id )
{
    for ( auto it = item_info.begin(); it != item_info.end(); it++ )
	{
		if ( it->second->ID == item_id )
		{
			return *it->second;
		}
	}
	return none;
}

void ResourceManager::addItemInfo( ItemInfo * new_item_info )
{
	// -- IDs will be set to the index that the instance is in the map
	if ( new_item_info->ID == 0 )
		new_item_info->ID = item_info.size();

	item_info.insert( std::make_pair( new_item_info->name, new_item_info ) );
}

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

void ResourceManager::addTexture(const std::string& name, Texture* new_texture)
{
	this->textures.insert( std::make_pair( name, new_texture ) );
}

Texture * ResourceManager::getTexture(std::string name)
{
	auto it = textures.find( name );

	if ( it == textures.end() )
		return nullptr;

	return it->second;
}

void ResourceManager::addFont(const std::string& name, TextFont* new_font)
{
	this->fonts.insert( std::make_pair( name, new_font ) );
}

TextFont * ResourceManager::getFont(std::string name)
{
	auto it = fonts.find( name );

	if ( it == fonts.end() )
		return nullptr;

	return it->second;
}

void ResourceManager::addShader(const std::string& name, Shader* new_shader)
{
	this->shaders.insert( std::make_pair( name, new_shader ) );
}

Shader * ResourceManager::getShader(std::string name)
{
	auto it = shaders.find( name );

	if ( it == shaders.end() )
		return nullptr;

	return it->second;
}

void ResourceManager::addVertexFormat(const std::string& name, VertexFormat* vf)
{
	this->vertex_formats.insert( std::make_pair(name, vf) );
}

VertexFormat * ResourceManager::getVertexFormat(const std::string& name)
{
	auto i = this->vertex_formats.find( name );

	if ( i == this->vertex_formats.end() )
	{
		return nullptr;
	}

	return i->second;
}
