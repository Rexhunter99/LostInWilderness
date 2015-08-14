
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
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <memory.h>
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

Chunk::Chunk( const Chunk & chunk )
{
	this->ax = chunk.ax;
	this->ay = chunk.ay;
	this->az = chunk.az;
	this->block_pool = (Block*)std::calloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_LENGTH, sizeof(Block));
	memcpy( this->blk, chunk.blk, sizeof(this->blk) );
	this->left = chunk.left;
	this->right = chunk.right;
	this->below = chunk.below;
	this->above = chunk.above;
	this->front = chunk.front;
	this->back = chunk.back;

	this->elements = chunk.elements;
	this->biome = chunk.biome;
	glGenBuffers(1, &vbo);
	this->time_last_saved = chunk.time_last_saved;
	this->time_last_updated = chunk.time_last_updated;
	this->time_inhabited = chunk.time_inhabited;

	this->needs_update = chunk.needs_update;
	this->initialized = chunk.initialized;
	this->generated = chunk.generated;
	this->generating = chunk.generating;
	this->humidity = chunk.humidity;
	this->temperature = chunk.temperature;
}

Chunk::~Chunk()
{
	free(this->block_pool);

	glDeleteBuffers( 1, &this->vbo );
}

Chunk & Chunk::operator = ( const Chunk & chunk )
{
	this->ax = chunk.ax;
	this->ay = chunk.ay;
	this->az = chunk.az;

	if (this->block_pool != nullptr)
		free(this->block_pool);

	this->block_pool = (Block*)std::calloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_LENGTH, sizeof(Block));
	memcpy( this->blk, chunk.blk, sizeof(this->blk) );

	this->left = chunk.left;
	this->right = chunk.right;
	this->below = chunk.below;
	this->above = chunk.above;
	this->front = chunk.front;
	this->back = chunk.back;

	this->elements = chunk.elements;
	this->biome = chunk.biome;
	glGenBuffers(1, &vbo);
	this->time_last_saved = chunk.time_last_saved;
	this->time_last_updated = chunk.time_last_updated;
	this->time_inhabited = chunk.time_inhabited;

	this->needs_update = chunk.needs_update;
	this->initialized = chunk.initialized;
	this->generated = chunk.generated;
	this->generating = chunk.generating;
	this->humidity = chunk.humidity;
	this->temperature = chunk.temperature;

	return *this;
}

void Chunk::init( bool manual_gen )
{
	// -- Allocate a single pool of memory for the Block instances, initialise it to 0's
	this->block_pool = (Block*)std::calloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_LENGTH, sizeof(Block));

	// -- Fill the 3D array with pointers to the areas of memory for each little blockywocky
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int z = 0; z < CHUNK_LENGTH; z++) {
			for (int y = 0; y < CHUNK_HEIGHT; y++) {
				this->blk[x][y][z] = nullptr;//this->block_pool + (x + CHUNK_WIDTH * (z + CHUNK_LENGTH) * y);
			}
		}
	}

	// -- Initialize the neighbour chunk pointers to nullptr
	left = right = below = above = front = back = nullptr;

	// -- Initialize the time the chunk was last used to 'now'
	this->time_last_saved	= std::time( nullptr );

	// -- Properties
	this->elements			= 0;
	this->biome				= 0;
	this->humidity			= 0.0f;
	this->temperature		= 0.0f;
	this->time_inhabited	= 0;
	this->time_last_updated	= 0;
	// -- Initialize the flags (1 bit each)
	this->needs_update			= true;
	this->initialized			= false;
	this->generated				= manual_gen;
	this->generating			= false;

	glGenBuffers(1, &this->vbo);
}


bool Chunk::isBlocked(int x1, int y1, int z1, int x2, int y2, int z2)
{
	//Block *b = blk.at(vector3i(x1,y1,z1));
	Block *b = blk[x1][y1][z1];
	Block *b2 = this->get(x2, y2, z2);

	// -- if itself is empty/air then it is blocked
	if (b == nullptr)
		return true;

	// -- Air blocks/empty blocks do not block
	if (b2 == nullptr)
		return false;

	// -- If both blocks are water blocks they are blocked
	if (b->info.name == "salt_water" && b2->info.name == "salt_water")
		return true;

	// Blocks that are see through (glass/liw:leaves/etc) do not block
	if (b2 != nullptr && (b2->info.flags & BI_TRANSLUCENT || b2->info.flags & BI_OPACITY))
		return false;
	else // -- Non-transparent blocks always block line of sight
		return true;

	// Otherwise, LOS is only blocked by blocks if the same transparency type
	return (b2->info.flags & BI_TRANSLUCENT || b2->info.flags & BI_OPACITY) == (b->info.flags & BI_TRANSLUCENT || b->info.flags & BI_OPACITY);
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

	return this->blk[x][y][z];
}


void Chunk::set( int x, int y, int z, Block * new_block )
{
	// -- If coordinates are outside this Chunk, make that Chunk handle the set() call
	if(x < 0)
	{
		if(left)
			left->set(x + CHUNK_WIDTH, y, z, new_block);
		return;
	}
	if(x >= CHUNK_WIDTH)
	{
		if(right)
			right->set(x - CHUNK_WIDTH, y, z, new_block);
		return;
	}
	if(y < 0)
	{
		if(below)
			below->set(x, y + CHUNK_HEIGHT, z, new_block);
		return;
	}
	if(y >= CHUNK_HEIGHT)
	{
		if(above)
			above->set(x, y - CHUNK_HEIGHT, z, new_block);
		return;
	}
	if(z < 0)
	{
		if(front)
			front->set(x, y, z + CHUNK_LENGTH, new_block);
		return;
	}
	if(z >= CHUNK_LENGTH)
	{
		if(back)
			back->set(x, y, z - CHUNK_LENGTH, new_block);
		return;
	}

	// -- If the `new_block` is nullptr, then we set the array pointer to null as well
	if ( new_block == nullptr )
	{
		if (this->blk[x][y][z] != nullptr)
		{
			this->blk[x][y][z]->~Block();
			this->blk[x][y][z] = nullptr;
		}
	}
	// -- Otherwise we need to copy the data via dereference
	else
	{
		// -- Point the pointer to the correct part of the pool
		if (this->blk[x][y][z] == nullptr)
		{
			Block *blk_ptr = this->block_pool + (x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_LENGTH));
			this->blk[x][y][z] = new (blk_ptr) Block(new_block->info);
		}
		else
		{
			this->blk[x][y][z]->info = new_block->info;
			this->blk[x][y][z]->data_value = new_block->data_value;
		}
	}

	this->needs_update = true;

	/*
	When updating blocks at the edge of this Chunk,
	visibility of blocks in the neighbouring Chunk might change.
	*/

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

void Chunk::update()
{
	//std::lock_guard<std::mutex> lock(this->lock);

	//VertexBuffer vb;
	std::vector<Vertex> vertices;
	vertices.resize(0);

	chunk_update_count++;

	// View from negative x

	for(int x = CHUNK_WIDTH - 1; x >= 0; x--)
	{
		for(int y = 0; y < CHUNK_HEIGHT; y++)
		{
			for(int z = 0; z < CHUNK_LENGTH; z++)
			{
				Block *b = this->blk[x][y][z];
				//Block *b = this->blk.at(vector3i(x,y,z));

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
				//Block *b      = blk.at(vector3i(x,y,z));

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
				//Block *b      = this->blk.at(vector3i(x,y,z));

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
				//Block *b      = blk.at(vector3i(x,y,z));

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
				//Block *b      = blk.at(vector3i(x,y,z));

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
				//Block *b      = blk.at(vector3i(x,y,z));

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
	if ( this->elements == 0 || vertices.empty() )
	{
		return;
	}

	// -- Upload vertices
	Renderer::iRenderer->getMutex().lock();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);
	// ^ SIGSEGV happens here, vertices is corrupt/missing
	Renderer::iRenderer->getMutex().unlock();
}

void Chunk::render()
{
	std::lock_guard<std::mutex> lock(this->lock);

	if ( this->needs_update )
	{
		// TODO: Add updates to a thread that processes queued update requests
		//this->update();
		this->needs_update = false;
		LostInWilderness::addChunkToUpdateQueue( this );
	}

	// -- If there are no elements then we do not render
	if( this->elements == 0 )
	{
		return;
	}

	// -- Push the batch to the GPU (unoptimized)
	// TODO: Handle VBOs at a higher level so more cells can be batched into fewer VBOs
	// NOTE: Possibly have a series of VBOs and the chunk only references the vbo its data belongs to.
	Shader *shader = Renderer::default_shader;

	Renderer::iRenderer->getMutex().lock();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer( shader->getAttrib( "v_position" ), 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0 );
	glVertexAttribPointer( shader->getAttrib( "v_normal" ), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 4) );
	glVertexAttribPointer( shader->getAttrib( "v_texcoord" ), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 7) );
	glVertexAttribPointer( shader->getAttrib( "v_diffuse" ), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 9) );
	glGetError();
	glDrawArrays(GL_TRIANGLES, 0, this->elements);
	Renderer::iRenderer->getMutex().unlock();
}

void Chunk::generate( int seed )
{
	if ( this->generated == true )
	{
		return;
	}

	// -- Lock the data from being accessed outside this thread
	std::unique_lock<std::mutex> alock(this->lock);

	this->generating = true;

	ResourceManager::iResourceManager->getWorldGen( "default" ).generate( this );

	// -- Set flags for the chunk having been generated & initialized
	this->initialized = true;
	this->needs_update = true;
	this->generated = true;
	this->generating = false;

	// -- Unlock the data so it can be accessed again
	alock.unlock();

	LostInWilderness::addChunkToUpdateQueue( this );
	LostInWilderness::addChunkToSaveQueue( this );
}

void Chunk::save()
{
	std::stringstream name;
	name << "../world/" << this->ax << "," << this->ay << "," << this->az << ".chunk";

	// -- Open the file stream for binary output
	std::fstream f;
	f.open( name.str().c_str(), std::ios::out | std::ios::binary );

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
			//Block *b = this->blk.at(vector3i(x,y,z));

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

		buffer.rle_compress<uint8_t>();

		f.write( (const char*)buffer.ptr(), buffer.size() );

		f.close();
	}
	else
	{
		std::cerr << "[ ERROR ] Failed to open the file: " << name.str() << std::endl;
	}

	this->time_last_saved = time( nullptr );
}

void Chunk::load()
{
	std::stringstream name;
	name << "../world/" << this->ax << "," << this->ay << "," << this->az << ".chunk";

	// -- Open the file stream for binary output
	std::fstream f;
	f.open( name.str().c_str(), std::ios::in | std::ios::binary );

	// -- If the file stream opened properly then we write to it
	if ( !f.is_open() )
	{
		std::cerr << "[ ERROR ] File does not exist, or is corrupt: \"" << name.str() << "\"" << std::endl;
	}
	else
	{
		std::unique_lock<std::mutex> alock(this->lock);

		// -- Write out header data
		uint16_t version = 1;
		f.read( (char*)&version, sizeof(uint16_t) );
		f.read( (char*)&this->biome, sizeof(uint16_t) );

		// -- Loop through all the blocks in the chunk
		Buffer buffer( CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_LENGTH * sizeof(uint16_t) );

		// -- Read the chunk data from the file then close the file
		f.read( (char*)buffer.ptr(), buffer.size() );
		f.close();

		// -- Decompress the RLE8 data
		buffer.rle_decompress<uint8_t>();

		for( int x=0; x<CHUNK_WIDTH; x++)
		for( int z=0; z<CHUNK_LENGTH; z++)
		for( int y=0; y<CHUNK_HEIGHT; y++)
		{
			uint8_t temp_ID = 0;
			uint8_t temp_data = 0;

			f.read( (char*)&temp_ID, sizeof(uint8_t) );
			f.read( (char*)&temp_data, sizeof(uint8_t) );

			this->blk[x][y][z] = new Block( ResourceManager::iResourceManager->getBlockInfo( (size_t)temp_ID ) );
			this->blk[x][y][z]->data_value = temp_data;
			//this->blk.at(vector3i(x,y,z)) = new Block( ResourceManager::iResourceManager->getBlockInfo( (size_t)temp_ID ) );
			//this->blk.at(vector3i(x,y,z))->data_value = temp_data;
		}

		f.close();
	}

	// -- Set the time_last_saved to now.
	this->time_last_saved = time( nullptr );
}
