
#include "BiomeDesert.h"
#include "Block.h"
#include "BlockDirt.h"
#include "BlockSaltWater.h"
#include "Chunk.h"
#include "Noise.h"
#include "ResourceManager.h"

#include <cstdlib>
#include <cmath>

using namespace Noise;


void BiomeDesert::generate( Chunk *chunk, Perlin *noise )
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
			for (int y = 0; y < CHUNK_HEIGHT; y++ )
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

					// Place a tree if the block below is a sand block
					if( b != nullptr &&
						b->info == ResourceManager::iResourceManager->getBlockInfo( "sand" ) &&
						(rand() & 0xff) <= 3 )
					{
						//this->placeCactus( chunk, x, y, z );
					}
					continue;
				}
                if ( y < 4 )
				{
					chunk->set(x,y,z, new Block( ResourceManager::iResourceManager->getBlockInfo( "ore" ) ) ); // bedrock
				}
				else if ( y < height - 4 )
				{
					chunk->set( x, y, z, new Block( ResourceManager::iResourceManager->getBlockInfo( "stone" ) ) );
                }
				else
				{
					chunk->set( x, y, z, new Block( ResourceManager::iResourceManager->getBlockInfo( "sand" ) ) );
                }
			}
		}
	}
}

void BiomeDesert::placeCactus( Chunk *chunk, int x, int y, int z )
{
	// Trunk
	int h = 1 + (rand() & 0x3);

	for ( int i = 0; i < h; i++ )
	{
		//chunk->set(x, y + i, z, new Block( ResourceManager::iResourceManager->getBlockInfo( "cactus" ) ) );
	}
}

