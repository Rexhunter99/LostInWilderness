#pragma once

#ifndef __BIOME_H__
#define __BIOME_H__

#include <string>

#include "Block.h"
#include "BlockDirt.h"
#include "BlockSaltWater.h"
#include "ResourceManager.h"
namespace Noise { class Perlin; };
class Chunk;
class WorldGenerator;


class Biome
{
protected:
	friend class WorldGenerator;

	std::string		name;
	struct color_s {
		unsigned char r, g, b, a;
	} color;
	float			temperature,
					rainfall;
	unsigned char	y_min,
					y_max;
	WorldGenerator	*worldgen;

public:

	Biome( WorldGenerator * worldgen, std::string name = "biome_unknown", unsigned char y_min = 64, unsigned char y_max = 64, float temperature = 0.0f, float rainfall = 0.0f )
	{
		this->name		= "biome_default";
		this->y_min		= y_min;
		this->y_max		= y_max;
		this->temperature = temperature;
		this->rainfall	= rainfall;
		this->worldgen	= worldgen;
	}
	virtual ~Biome()
	{}

	virtual void simulateChunk( Chunk *chunk ) {}
	virtual void calcLight( Chunk *chunk ) {}
	virtual void generate( Chunk *chunk, Noise::Perlin *height ) {}
	virtual void decorate( Chunk *chunk ) {}

};

#endif // __BIOME_H__
