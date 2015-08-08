
#include "BiomeForest.h"
#include "Block.h"
#include "BlockDirt.h"
#include "BlockSaltWater.h"
#include "Chunk.h"
#include "Noise.h"
#include "ResourceManager.h"

#include <cstdlib>
#include <cmath>

using namespace Noise;


void BiomeForest::generate( Chunk *chunk, Perlin *noise )
{
	// TODO: finish
	for(int64_t x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int64_t z = 0; z < CHUNK_LENGTH; z++)
		{
			// Land height
			int64_t rx = x + (chunk->ax * CHUNK_WIDTH);
			int64_t rz = z + (chunk->az * CHUNK_LENGTH);

			int height = 64 + worldgen->getTerrainHeight( rx, rz, 0.0f );

			// Land blocks
			for (int64_t y = 0; y < CHUNK_HEIGHT; y++ )
			{
				int actual_y = y + chunk->ay * CHUNK_HEIGHT;

				// -- Sea level
				if ( actual_y >= height && actual_y < 64 )
				{
					BlockSaltWater b(ResourceManager::iResourceManager->getBlockInfo("liw:salt_water"));
					chunk->set( x, y, z, &b );
					continue;
				}

				// Are we above "ground" level?
				if ( y + chunk->ay * CHUNK_HEIGHT >= height )
				{
					Block *b = chunk->get(x, y-1, z);

					// Place a tree if the block below is a dirt block
					if( b != nullptr &&
						b->info == ResourceManager::iResourceManager->getBlockInfo( "liw:soil" ) &&
						(rand() & 0xff) <= 3 )
					{
						this->placePineTree( chunk, x, y, z );
					}
					continue;
				}

				if ( y < 4 )
				{
					Block b(ResourceManager::iResourceManager->getBlockInfo("liw:bedrock"));
					chunk->set(x,y,z, &b ); // bedrock
				}
				else if ( y < height - 4 )
				{
					Block b(ResourceManager::iResourceManager->getBlockInfo("liw:stone"));
					chunk->set(x,y,z, &b );
				}
				else
				{
					chunk->set(x,y,z, &BlockDirt( ResourceManager::iResourceManager->getBlockInfo( "liw:soil" ) ) );
				}
			}
		}
	}
}

void BiomeForest::placePineTree( Chunk *chunk, int x, int y, int z )
{
	// Trunk
	int h = 2 + (rand() & 0x3);

	for ( int i = 0; i < h; i++ )
	{
		chunk->set(x, y + i, z, &Block( ResourceManager::iResourceManager->getBlockInfo( "liw:wood" ) ) );
	}

	// liw:leaves
	bool thin = true;

	for ( int iy = -1; iy <= 6; iy++ )
	{
		if ( iy == 6 )
		{
			chunk->set( x, y + h + iy, z, &Block( ResourceManager::iResourceManager->getBlockInfo( "liw:leaves" ) ) );
		}
		else if ( thin == false )
		{
			for ( int ix = -2; ix <= 2; ix++ )
			for ( int iz = -2; iz <= 2; iz++ )
			{
				if ( ix * ix + iz * iz < 8 )
				chunk->set( x + ix, y + h + iy, z + iz, &Block( ResourceManager::iResourceManager->getBlockInfo( "liw:leaves" ) ) );
			}
			chunk->set( x, y + h + iy, z, &Block( ResourceManager::iResourceManager->getBlockInfo( "liw:wood" ) ) );
			thin = true;
		}
		else
		{
			for ( int ix = -1; ix <= 1; ix++ )
			for ( int iz = -1; iz <= 1; iz++ )
			{
				chunk->set( x + ix, y + h + iy, z + iz, &Block( ResourceManager::iResourceManager->getBlockInfo( "liw:leaves" ) ) );
			}
			chunk->set( x, y + h + iy, z, &Block( ResourceManager::iResourceManager->getBlockInfo( "liw:wood" ) ) );
			thin = false;
		}
	}
}
