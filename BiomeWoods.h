#pragma once

#ifndef __BIOMEWOODS_H__
#define __BIOMEWOODS_H__

#include "Biome.h"


class BiomeWoods : public Biome
{
protected:

public:

	BiomeWoods( WorldGenerator *worldgen )
	: Biome( worldgen )
	{
		this->name		= "biome_woods";
		this->color.r	= 0;
		this->color.g	= 200;
		this->color.b	= 20;
		this->color.a	= 255;
		this->y_min		= 64;
		this->y_max		= 90;
		this->temperature = 50.0f;
		this->rainfall	= 40.0f;
	}
	~BiomeWoods()
	{}

	void generate( Chunk *chunk, Noise::Perlin *height );

	void placePineTree( Chunk *chunk, int x, int y, int z );
	void placeBoulder( Chunk *chunk, int x, int y, int z );

};

#endif // __BIOMEWOODS_H__

