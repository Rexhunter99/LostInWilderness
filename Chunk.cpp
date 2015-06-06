
#include "Chunk.h"
#include "Block.h"
#include "BlockDirt.h"
#include "BlockSaltWater.h"
#include "Buffer.h"
#include "Exceptions.h"
#include "LostInWilderness.h"
#include "Shader.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "WorldGenerator.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>
#include <GL/glew.h>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <memory.h>
#include <cmath>
#include <stdint.h>

typedef glm::detail::tvec4<GLbyte, glm::precision::defaultp> byte4;

extern uint32_t chunk_update_count;

//static float noise2d(float x, float y, int seed, int octaves, float persistence);
//static float noise3d_abs(float x, float y, float z, int seed, int octaves, float persistence);


Chunk::Chunk()
{
	this->init();
	this->ax = 0;
	this->ay = 0;
	this->az = 0;
}

Chunk::Chunk( int x, int y, int z, bool manual_gen )
{
	this->init( manual_gen );
	this->ax = x;
	this->ay = y;
	this->az = z;
}

Chunk::~Chunk()
{
	// NOTE: Eww this is slow :c
	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int z = 0; z < CHUNK_LENGTH; z++)
		{
			for(int y = 0; y < CHUNK_HEIGHT; y++)
			{
				if ( blk[x][y][z] != nullptr )
					delete blk[x][y][z];
			}
		}
	}

	glDeleteBuffers( 1, &this->vbo );
}

void Chunk::init( bool manual_gen )
{
	//memset( blk, 0, sizeof(blk) );
	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int z = 0; z < CHUNK_LENGTH; z++)
		{
			for(int y = 0; y < CHUNK_HEIGHT; y++)
			{
				this->blk[x][y][z] = nullptr;
			}
		}
	}

	// -- Initialize the neighbour chunk pointers to nullptr
	left = right = below = above = front = back = nullptr;

	// -- Initialize the time the chunk was last used to 'now'
	this->time_last_saved	= std::time( nullptr );

	// -- Initialize the flags (1 bit each)
	needs_update			= true;
	initialized				= false;
	generated				= manual_gen;
	generating				= false;
	glGenBuffers(1, &this->vbo);
}

Block * Chunk::get(int x, int y, int z) const
{
	if (x < 0)
		return left ? left->blk[x + CHUNK_WIDTH][y][z] : 0;
	if (x >= CHUNK_WIDTH)
		return right ? right->blk[x - CHUNK_WIDTH][y][z] : 0;
	if (y < 0)
		return below ? below->blk[x][y + CHUNK_HEIGHT][z] : 0;
	if (y >= CHUNK_HEIGHT)
		return above ? above->blk[x][y - CHUNK_HEIGHT][z] : 0;
	if (z < 0)
		return front ? front->blk[x][y][z + CHUNK_LENGTH] : 0;
	if (z >= CHUNK_LENGTH)
		return back ? back->blk[x][y][z - CHUNK_LENGTH] : 0;
	return blk[x][y][z];
}

bool Chunk::isBlocked(int x1, int y1, int z1, int x2, int y2, int z2)
{
	Block *b = blk[x1][y1][z1];
	Block *b2 = this->get( x2, y2, z2 );

	// -- if itself is empty/air then it is blocked
	if ( b == nullptr )
		return true;

	// -- Air blocks/empty blocks do not block
	if ( b2 == nullptr )
		return false;

	// -- If both blocks are water blocks they
	if ( b->info.name == "salt_water" && b2->info.name == "salt_water" )
		return true;

	// Blocks that are see through (glass/leaves/etc) do not block
	if ( b2 != nullptr && (b2->info.flags & BI_TRANSLUCENT || b2->info.flags & BI_OPACITY) )
		return false;
	else // -- Non-transparent blocks always block line of sight
		return true;

	// Otherwise, LOS is only blocked by blocks if the same transparenCHUNK_HEIGHT type
	return ( b2->info.flags & BI_TRANSLUCENT || b2->info.flags & BI_OPACITY ) == ( b->info.flags & BI_TRANSLUCENT || b->info.flags & BI_OPACITY );
}

void Chunk::set( int x, int y, int z, Block *new_block )
{
	if ( new_block == nullptr )
	{
		throw nullptr_exception();
	}

	// -- If coordinates are outside this Chunk, find the right one.
	if(x < 0)
	{
		if(left)
			left->set(x + CHUNK_WIDTH, y, z, new_block);
		else
		{
			delete new_block;
		}
		return;
	}
	if(x >= CHUNK_WIDTH)
	{
		if(right)
			right->set(x - CHUNK_WIDTH, y, z, new_block);
		else
		{
			delete new_block;
		}
		return;
	}
	if(y < 0)
	{
		if(below)
			below->set(x, y + CHUNK_HEIGHT, z, new_block);
		else
		{
			delete new_block;
		}
		return;
	}
	if(y >= CHUNK_HEIGHT)
	{
		if(above)
			above->set(x, y - CHUNK_HEIGHT, z, new_block);
		else
		{
			delete new_block;
		}
		return;
	}
	if(z < 0)
	{
		if(front)
			front->set(x, y, z + CHUNK_LENGTH, new_block);
		else
		{
			delete new_block;
		}
		return;
	}
	if(z >= CHUNK_LENGTH)
	{
		if(back)
			back->set(x, y, z - CHUNK_LENGTH, new_block);
		else
		{
			delete new_block;
		}
		return;
	}

	// -- If a block exists here already, destroy it
	if ( this->blk[x][y][z] != nullptr )
	{
		delete this->blk[x][y][z];
	}

	// -- Set the block
	this->blk[x][y][z] = new_block;
	needs_update = true;

	// When updating blocks at the edge of this WorldChunk,
	//   visibility of blocks in the neighbouring WorldChunk might change.

	if(x == 0 && left)
		left->needs_update = true;
	if(x == CHUNK_WIDTH - 1 && right)
		right->needs_update = true;
	if(y == 0 && below)
		below->needs_update = true;
	if(y == CHUNK_HEIGHT - 1 && above)
		above->needs_update = true;
	if(z == 0 && front)
		front->needs_update = true;
	if(z == CHUNK_LENGTH - 1 && back)
		back->needs_update = true;
}

/// @deprecated deprecated noise functions
/*static float noise2d(float x, float y, int seed, int octaves, float persistence)
{
	float sum = 0;
	float strength = 1.0;
	float scale = 1.0;
	float offset = 0.0f;//( (float)rand() / (float)(seed % RAND_MAX) );

	for(int i = 0; i < octaves; i++)
	{
		sum += strength * glm::perlin(glm::vec2(x + offset, y + offset) * scale);
		scale *= 2.0;
		strength *= persistence;
	}

	return sum;
}

static float noise3d_abs(float x, float y, float z, int seed, int octaves, float persistence)
{
	float sum = 0;
	float strength = 1.0;
	float scale = 1.0;

	for(int i = 0; i < octaves; i++) {
		sum += strength * fabs(glm::perlin(glm::vec3(x, y, z) * scale));
		scale *= 2.0;
		strength *= persistence;
	}

	return sum;
}*/

// TODO: fix this peice of crap... we need a Generator class
// NOTE: Generator class is being worked on so I've commented this out
void Chunk::noise( int seed )
{
/*	if ( this->noised )
		return;
	else
		this->noised = true;

	static int r = 0;
	if ( r == 0 )
	{
		srand( seed );
		r = seed / (rand() + rand());
		fprintf( stdout, "r = %d\n", r );
	}

	// -- Biome
	float biome = std::fabs(glm::perlin( glm::vec2( (float)ax / 16.0f, (float)az / 16.0f ) )) * 255;
	fprintf( stdout, "biome[%d,%d] = %f\n", ax,ay, biome );

	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int z = 0; z < CHUNK_LENGTH; z++)
		{
			// Land height
			int rx = x + (ax * CHUNK_WIDTH) + r;
			int rz = z + (az * CHUNK_LENGTH) + r;
			float n = glm::perlin( glm::vec3(rx / 16.0f, rz / 16.0f, seed / 100000000.0f ) ) * 16;
			//float n = noise2d((x + rx * CHUNK_WIDTH ) / 64.0, (z + rz * CHUNK_LENGTH) / 64.0, seed, 3, 0.8) * 16;
			int h = 64 + (n * 1);
			int y = 0;

			// Land blocks
			for(y = 0; y < CHUNK_HEIGHT; y++)
			{
				// Are we above "ground" level?
				if(y + ay * CHUNK_HEIGHT >= h)
				{
					// If we are not yet up to sea level, fill with water blocks
					if(y + ay * CHUNK_HEIGHT < SEALEVEL)
					{
						blk[x][y][z] = new BlockSaltWater( GaiaCraft::resource_manager.getBlockInfo( "salt_water" ) );
						continue;
						// Otherwise, we are in the air
					}
					else
					{
						Block *b = this->get(x, y-1, z);

						// Place a tree if the block below is a dirt/grass block
						if( b != nullptr &&
							b->info == GaiaCraft::resource_manager.getBlockInfo( "dirt" ) &&
							(rand() & 0xff) == 0 )
						{
							// Trunk
							h = (rand() & 0x3) + 3;

							for(int i = 0; i < h; i++)
							{
								this->set(x, y + i, z, new Block( GaiaCraft::resource_manager.getBlockInfo( "wood" ) ) );
							}

							// Leaves
							for(int ix = -3; ix <= 3; ix++) for(int iy = -3; iy <= 3; iy++) for(int iz = -3; iz <= 3; iz++)
							{
								if ( ix * ix + iy * iy + iz * iz < 8 + (rand() & 1) && !get(x + ix, y + h + iy, z + iz) )
								{
									this->set(x + ix, y + h + iy, z + iz, new Block( GaiaCraft::resource_manager.getBlockInfo( "leaves" ) ) );
								}
							}
						}
						break;
					}
				}

				// Random value used to determine land type
				float r = noise3d_abs((x + ax * CHUNK_WIDTH) / 16.0, (y + ay * CHUNK_HEIGHT) / 16.0, (z + az * CHUNK_LENGTH) / 16.0, -seed, 2, 1);

				if(n + r * 5 < 2)		// Sand layer
					blk[x][y][z] = new Block( GaiaCraft::resource_manager.getBlockInfo( "sand" ) );
				else if(n + r * 5 < 8)	// Dirt layer
					blk[x][y][z] = new BlockDirt( GaiaCraft::resource_manager.getBlockInfo( "dirt" ) );
				else if(r < 1.25)
					blk[x][y][z] = new Block( GaiaCraft::resource_manager.getBlockInfo( "stone" ) );
				// Sometimes, ores!
				else
					blk[x][y][z] = new Block( GaiaCraft::resource_manager.getBlockInfo( "ore" ) );
			}
		}
	}
	needs_update = true;*/
}

void Chunk::update()
{
	std::vector<Vertex> vertices;

	chunk_update_count++;

	// View from negative x

	for(int x = CHUNK_WIDTH - 1; x >= 0; x--)
	{
		for(int y = 0; y < CHUNK_HEIGHT; y++)
		{
			for(int z = 0; z < CHUNK_LENGTH; z++)
			{
				Block *b = this->blk[x][y][z];

				if ( b == nullptr )
					continue;

				// Line of sight blocked?
				if ( this->isBlocked(x, y, z, x - 1, y, z) )
				{
					continue;
				}

				b->render( this, x, y, z, -1, vertices );
			}
		}
	}

	// View from positive x

	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int y = 0; y < CHUNK_HEIGHT; y++)
		{
			for(int z = 0; z < CHUNK_LENGTH; z++)
			{
				Block *b		= blk[x][y][z];

				if ( b == nullptr )
					continue;

				if ( this->isBlocked(x, y, z, x + 1, y, z) )
				{
					continue;
				}

				b->render( this, x, y, z, 1, vertices );
			}
		}
	}

	// View from negative y

	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int y = CHUNK_HEIGHT - 1; y >= 0; y--)
		{
			for(int z = 0; z < CHUNK_LENGTH; z++)
			{
				Block *b		= this->blk[x][y][z];

				if(isBlocked(x, y, z, x, y - 1, z))
				{
					continue;
				}

				b->render( this, x, y, z, -2, vertices );
			}
		}
	}

	// View from positive y

	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int y = 0; y < CHUNK_HEIGHT; y++)
		{
			for(int z = 0; z < CHUNK_LENGTH; z++)
			{
				Block *b		= blk[x][y][z];

				if (isBlocked(x, y, z, x, y + 1, z))
				{
					continue;
				}

				b->render( this, x, y, z, 2, vertices );
			}
		}
	}

	// View from negative z

	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int z = CHUNK_LENGTH - 1; z >= 0; z--)
		{
			for(int y = 0; y < CHUNK_HEIGHT; y++)
			{
				Block *b		= blk[x][y][z];

				if(isBlocked(x, y, z, x, y, z - 1))
				{
					continue;
				}

				b->render( this, x, y, z, -3, vertices );
			}
		}
	}

	// View from positive z

	for(int x = 0; x < CHUNK_WIDTH; x++)
	{
		for(int z = 0; z < CHUNK_LENGTH; z++)
		{
			for(int y = 0; y < CHUNK_HEIGHT; y++)
			{
				Block *b		= blk[x][y][z];

				if(isBlocked(x, y, z, x, y, z + 1))
				{
					continue;
				}

				b->render( this, x, y, z, 3, vertices );
			}
		}
	}

	this->needs_update = false;
	this->elements = vertices.size();
	// -- Update the time_last_used member
	this->time_last_updated = std::time( nullptr );

	// -- If this Chunk is empty, no need to allocate a Chunk slot.
	if( !this->elements )
		return;

	// -- Upload vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);
}

void Chunk::render()
{
	if ( this->needs_update )
	{
		// TODO: Add updates to a thread that processes queued update requests
		this->update();
	}

	// -- If there are no elements then we do not render
	if( !this->elements )
	{
		return;
	}

	// -- Push the batch to the GPU (unoptimized)
	// TODO: Handle VBOs at a higher level so more cells can be batched into fewer VBOs
	// NOTE: Possibly have a series of VBOs and the chunk only references the vbo its data belongs to.
	Shader *shader = Renderer::default_shader;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer( shader->getAttrib( "v_position" ), 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0 );
	glVertexAttribPointer( shader->getAttrib( "v_normal" ), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 4) );
	glVertexAttribPointer( shader->getAttrib( "v_texcoord" ), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 7) );
	glVertexAttribPointer( shader->getAttrib( "v_diffuse" ), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 9) );
	glDrawArrays(GL_TRIANGLES, 0, elements);
}

void Chunk::generate( int seed )
{
	ResourceManager::iResourceManager->getWorldGen( "default" ).generate( this );

	// -- Set flags for the chunk having been generated & initialized
	this->initialized = true;
	this->needs_update = true;

	GaiaCraft::addChunkToUpdateQueue( this );
	GaiaCraft::addChunkToSaveQueue( this );
}

void Chunk::save()
{
	// TODO: Implement RLE-8 compression
	char name[256];
	sprintf( name, "world/c[%d][%d][%d].chunk", this->ax, this->ay, this->az );

	// -- Open the file stream for binary output
	std::fstream f;
	f.open( name, std::ios::out | std::ios::binary );

	// -- If the file stream opened properly then we write to it
	if ( f.is_open() )
	{
		// -- Write out header data
		uint16_t version = 1;
		f.write( (char*)&version, sizeof(uint16_t) );
		f.write( (char*)&this->biome, sizeof(uint16_t) );

		// -- Loop through all the blocks in the chunk
		Buffer buffer;

		for( int x=0; x<CHUNK_WIDTH; x++)
		for( int z=0; z<CHUNK_LENGTH; z++)
		for( int y=0; y<CHUNK_HEIGHT; y++)
		{
			Block *b = this->blk[x][y][z];

			if ( b == nullptr )
			{
				uint8_t temp_ID = 0;
				uint8_t temp_data = 0;
				buffer.add( (void*)&temp_ID, sizeof( uint8_t ) );
				buffer.add( (void*)&temp_data, sizeof( uint8_t ) );
				continue;
			}

			buffer.add( (void*)&b->info.ID, sizeof( b->info.ID) );
			buffer.add( (void*)&b->data_value, sizeof( b->data_value) );
		}

		buffer.rle8_compress();

		f.write( (const char*)buffer.ptr(), buffer.size() );

		f.close();
	}
	else
	{
		std::cerr << "[ ERROR ] Failed to open the file: " << name << std::endl;
	}

	this->time_last_saved = time( nullptr );
}
