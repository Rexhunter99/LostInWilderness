
#include "BiomeWoods.h"
#include "Block.h"
#include "BlockDirt.h"
#include "BlockSaltWater.h"
#include "Chunk.h"
#include "Noise.h"
#include "ResourceManager.h"

#include <cstdlib>
#include <cmath>

using namespace Noise;


void BiomeWoods::generate( Chunk *chunk, Perlin *noise )
{
	// TODO: finish
	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int z = 0; z < CHUNK_LENGTH; z++)
		{
			// Land height
			int rx = x + (chunk->ax * CHUNK_WIDTH);
			int rz = z + (chunk->az * CHUNK_LENGTH);

			int height = 64 + worldgen->getTerrainHeight( rx, rz, 0.0f );

			// Land blocks
			for ( int y = 0; y < CHUNK_HEIGHT; y++ )
			{
				int actual_y = y + chunk->ay * CHUNK_HEIGHT;

				// -- Sea level
				if ( actual_y >= height && actual_y < 64 )
				{
					chunk->set( x, y, z, new BlockSaltWater( ResourceManager::iResourceManager->getBlockInfo( "salt_water" ) ) );
					continue;
				}

				// Are we above "ground" level?
				if ( y + chunk->ay * CHUNK_HEIGHT >= height )
				{
					Block *b = chunk->get(x, y-1, z);

					// Place a tree if the block below is a dirt block
					if( b != nullptr &&
						b->info == ResourceManager::iResourceManager->getBlockInfo( "dirt" ) )
					{
						int rv = (rand() % 1024);
						if ( rv == 8)
							this->placeBoulder( chunk, x, y, z );
						else if ( rv <= 5 )
							this->placePineTree( chunk, x, y, z );
					}
					continue;
				}

				if ( y < 4 )
				{
					chunk->set(x,y,z, new Block( ResourceManager::iResourceManager->getBlockInfo( "ore" ) ) ); // bedrock
				}
				else if ( y < height - 4 )
				{
					chunk->set(x,y,z, new Block( ResourceManager::iResourceManager->getBlockInfo( "stone" ) ) );
				}
				else
				{
					chunk->set(x,y,z, new BlockDirt( ResourceManager::iResourceManager->getBlockInfo( "dirt" ) ) );
				}
			}
		}
	}
}

void BiomeWoods::placePineTree( Chunk *chunk, int x, int y, int z )
{
	// -- Set the height of the trunk
	int h = 2 + (rand() % 2);
	int mh = 4 + (rand() % 6);

	for ( int iy = 0; iy <  mh; iy++ )
	{
		for ( int ix = -2; ix <= 2; ix++ )
		for ( int iz = -2; iz <= 2; iz++ )
		{
			float l = 6.0f * ( 1.0f - ( float(iy) / float(mh) ) );
			if ( float(ix * ix + iz * iz) < l )
			chunk->set( x + ix, y + h + iy, z + iz, new Block( ResourceManager::iResourceManager->getBlockInfo( "leaves" ) ) );
		}
	}

	// -- Place the trunk
	for ( int iy = 0; iy <= h + (mh/2); iy++ )
	{
		chunk->set( x, y + iy, z, new Block( ResourceManager::iResourceManager->getBlockInfo( "wood" ) ) );
	}
}

void BiomeWoods::placeBoulder( Chunk *chunk, int x, int y, int z )
{
	for ( int iy = -3; iy <= 3; iy++ )
	{
		for ( int ix = -3; ix <= 3; ix++ )
		for ( int iz = -3; iz <= 3; iz++ )
		{
			if ( ix * ix + iz * iz + iy * iy < 6 )
			chunk->set( x + ix, y + iy, z + iz, new Block( ResourceManager::iResourceManager->getBlockInfo( "stone" ) ) );
		}
	}
}
