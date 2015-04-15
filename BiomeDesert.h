#pragma once

#ifndef __BIOMEDESERT_H__
#define __BIOMEDESERT_H__

#include "Biome.h"

#include <cstdlib>


class BiomeDesert : public Biome
{
protected:

public:

	BiomeDesert( WorldGenerator *worldgen )
	: Biome( worldgen )
	{
		this->name		= "biome_desert";
		this->color.r	= 0;
		this->color.g	= 200;
		this->color.b	= 20;
		this->color.a	= 255;
		this->y_min		= 64;
		this->y_max		= 90;
		this->temperature = 75.0f;
		this->rainfall	= 0.0f;
	}
	~BiomeDesert(){}

	void generate( Chunk *chunk, Noise::Perlin *height );

	void placeCactus( Chunk *chunk, int x, int y, int z );
	void placeBush( Chunk *chunk, int x, int y, int z );

};

#endif // __BIOMEDESERT_H__
