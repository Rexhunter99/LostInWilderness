#pragma once

#include "Noise.h"

#include <string>
#include <vector>

class Biome;
class Chunk;


class WorldGenerator
{
private:

	void generateOres( Chunk *chunk );

	Noise::Perlin n_terrain_base;
	Noise::Perlin n_terrain_alt;
	Noise::Perlin n_height_select;
	Noise::Perlin n_temperature;
	Noise::Perlin n_humidity;

public:

	std::vector<Biome*>	biome_list;
	std::string			name;
	uint64_t			seed;

    WorldGenerator();
    ~WorldGenerator();

    void reseed( unsigned int seed );

    virtual void generate( Chunk *chunk );
	virtual float getTerrainHeight( float x, float y, float z );
};
