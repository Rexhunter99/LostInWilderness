
#include "WorldGenerator.h"
#include "Block.h"
#include "BlockDirt.h"
#include "BlockSaltWater.h"
#include "Chunk.h"
#include "Exceptions.h"
#include "LostInWilderness.h"
#include "Noise.h"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "Biome.h"
#include "BiomeForest.h"
#include "BiomeDesert.h"
#include "BiomeWoods.h"

using namespace Noise;

extern uint32_t chunk_gen_count;


WorldGenerator::WorldGenerator()
{
	this->name = "default";

	n_terrain_base.reseed( this->seed );
	n_terrain_alt.reseed( 632 + this->seed );
	n_height_select.reseed( 8192 + this->seed );
	n_temperature.reseed( 8375 + this->seed );
	n_humidity.reseed( 569 + this->seed );

	biome_list.push_back( new Biome( this ) );
	biome_list.push_back( new BiomeForest( this ) );
	biome_list.push_back( new BiomeDesert( this ) );
	biome_list.push_back( new BiomeWoods( this ) );
}

WorldGenerator::~WorldGenerator()
{
	for ( unsigned int i=0; i<this->biome_list.size(); i++ )
	{
		delete this->biome_list[i];
	}
}


void WorldGenerator::reseed( unsigned int seed )
{
	this->seed = seed;
	n_terrain_base.reseed( this->seed );
	n_terrain_alt.reseed( 632 + this->seed );
	n_height_select.reseed( 8192 + this->seed );
	n_temperature.reseed( 8375 + this->seed );
	n_humidity.reseed( 569 + this->seed );
}

void WorldGenerator::generate( Chunk *chunk )
{
	if ( chunk == nullptr )
	{
		throw nullptr_exception();
	}

	if ( chunk->generated )
	{
		return;
	}

	// -- Generate the temperature and rainfall values for this chunk
	float temperature	= n_temperature.perlin2D( ( chunk->ax ) / 31.0f, ( chunk->az ) / 31.0f, 3, 0.5f, 50.0f, 50.0f );
	float rainfall		= n_humidity.perlin2D( ( chunk->ax ) / 31.0f, ( chunk->az ) / 31.0f, 3, 0.5f, 50.0f, 50.0f );

	float	dist_min = FLT_MAX;
	Biome 	*biome = this->biome_list[0];
	int		biome_id = 0;

	// -- Iterate through all the biomes to find which is the closest to matching the climate
	for ( unsigned int i=1; i<this->biome_list.size(); i++ )
	{
		Biome *b = this->biome_list[i];
		if ( b == nullptr ) continue;

		float d_temperature = temperature - b->temperature;
		float d_rainfall = rainfall - b->rainfall;
		float dist = ( d_temperature * d_temperature ) + ( d_rainfall * d_rainfall );
		if ( dist < dist_min )
		{
			dist_min = dist;
			biome = b;
			biome_id = i;
		}
	}

	//printf( "chunk[%d,%d] t:%f r:%f b:%s\n", chunk->ax, chunk->az, temperature, rainfall, biome->name.c_str() );

	chunk_gen_count++;

	// -- Generate the chunk and any ores
	biome->generate( chunk, &n_terrain_base );
	biome->decorate( chunk );
	this->generateOres( chunk );

	chunk->biome = biome_id;
	chunk->needs_update = true;
	chunk->generated = true;
}

void WorldGenerator::generateOres( Chunk *chunk )
{
    Biome 	*biome = this->biome_list[0]; // Do we want to declare this local variable again?

    for ( int i=0; i<17; i++ )
	{
		int x = std::rand() % 16;
		int z = std::rand() % 16;
		int y = std::rand() % 72;

		// Do not spawn an ore unless there is a block here already.
		if ( chunk->get( x, y, z ) == nullptr )
			continue;

		// Check if the block is a "stone" block
		if ( !(chunk->blk[x][y][z]->info == ResourceManager::iResourceManager->getBlockInfo( "stone" )) )
			continue;

		// NOTE: Need to replace these two lines with a WorldGenMinable that should spawn a vein properly
		chunk->set(x, y, z, biome->coal );
	}
}

float WorldGenerator::getTerrainHeight( float x, float z, float y )
{
	float ftb = n_terrain_base.perlin2D( x / 300.0f, z / 300.0f, 6, 0.7f, 4.0f, 70.0f );
	float fta = n_terrain_alt.perlin2D( x / 500.0f, z / 500.0f, 5, 0.6f, 4.0f, 25.0f );
	float k   = n_height_select.perlin2D( x / 250.0f, z / 250.0f, 5, 0.69f, 0.6f, 1.1f );

	if ( k > 1.0f ) k = 1.0f;
	if ( k < 0.0f ) k = 0.0f;

	return ( ftb * k ) + ( fta * ( 1.0f - k ) );
}
