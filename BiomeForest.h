#pragma once

#ifndef __BIOMEFOREST_H__
#define __BIOMEFOREST_H__

#include "Biome.h"


class BiomeForest : public Biome
{
protected:

public:

	BiomeForest( WorldGenerator *worldgen )
	: Biome( worldgen )
	{
		this->name		= "biome_forest";
		this->color.r	= 0;
		this->color.g	= 200;
		this->color.b	= 20;
		this->color.a	= 255;
		this->y_min		= 64;
		this->y_max		= 90;
		this->temperature = 50.0f;
		this->rainfall	= 65.0f;
	}
	~BiomeForest()
	{}

	void generate( Chunk *chunk, Noise::Perlin *height );

	void placePineTree( Chunk *chunk, int x, int y, int z );
	void placeBoulder( Chunk *chunk, int x, int y, int z );

};

#endif // __BIOMEFOREST_H__
