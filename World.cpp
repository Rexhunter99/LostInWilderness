
#include "World.h"
#include "Chunk.h"
#include "LostInWilderness.h"
#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include <iostream>


World::World( std::string name )
{
	this->name = name;
	this->seed = std::time( nullptr );

	srand( (unsigned int)this->seed );
	ResourceManager::iResourceManager->getWorldGen( "default" ).reseed( this->seed );

	this->preGenerate( 0, 0, 0, Config::getGlobal()->getInteger( "renderer.view_distance" ) );
}

World::~World()
{
	for ( auto i = this->chunk_map.begin(); i != this->chunk_map.end(); i++ )
	{
		if ( i->second != nullptr )
			delete i->second;
	}
}


Chunk * World::getChunk(int64_t x, int64_t y, int64_t z) const
{
	chunk_map_t::const_iterator i = this->chunk_map.find( vector3i( x, y, z ) );

	if ( i == this->chunk_map.end() )
	{
		return nullptr;
	}

	return i->second;
}

void World::setChunk(int64_t x, int64_t y, int64_t z, Chunk *chunk )
{
	#ifdef _DEBUG
	if ( this->chunk_map[vector3i(x,y,z)] != nullptr )
	{
		std::cout << "[ WARNING ] Attempting to replace chunk at " << x << "," << y << "," << z << std::endl;
	}
	#endif // _DEBUG

	this->chunk_map[vector3i(x,y,z)] = chunk;
}

Block * World::getBlock(int64_t x, int64_t y, int64_t z) const
{
	Chunk	*chunk = nullptr;
	int		vd = Config::getGlobal()->getInteger( "renderer.view_distance" );
	int64_t cx = (x + CHUNK_WIDTH * (vd / 2)) / CHUNK_WIDTH;
	int64_t cy = (y + CHUNK_HEIGHT * (1 / 2)) / CHUNK_HEIGHT;
	int64_t cz = (z + CHUNK_LENGTH * (vd / 2)) / CHUNK_LENGTH;

	// -- If the Chunk doesn't exist then we return no Block
	chunk = this->getChunk( cx, cy, cz );

	if ( chunk == nullptr )
	{
		return nullptr;
	}

	return chunk->get(x & (CHUNK_WIDTH - 1), y & (CHUNK_HEIGHT - 1), z & (CHUNK_LENGTH - 1));
}

void World::setBlock(int64_t x, int64_t y, int64_t z, Block *block )
{
	int vd = Config::getGlobal()->getInteger( "renderer.view_distance" );
	int64_t cx = (x + CHUNK_WIDTH * (vd / 2)) / CHUNK_WIDTH;
	int64_t cy = (y + CHUNK_HEIGHT * (1 / 2)) / CHUNK_HEIGHT;
	int64_t cz = (z + CHUNK_LENGTH * (vd / 2)) / CHUNK_LENGTH;

	if(cx < 0 || cx >= SCX || cy < 0 || cy >= SCY || cz <= 0 || cz >= SCZ)
		return;

	this->chunk_map[vector3i(cx,cy,cz)]->set( x & (CHUNK_WIDTH - 1), y & (CHUNK_HEIGHT - 1), z & (CHUNK_LENGTH - 1), block );
}

void World::preGenerate( int sx, int sy, int sz, int distance )
{
	// -- Allocate the chunks
	for (int x = 0; x < SCX; x++)
	for (int y = 0; y < SCY; y++)
	for (int z = 0; z < SCZ; z++)
	{
		this->chunk_map[ vector3i(x,y,z) ] = new Chunk(x - SCX / 2, y - SCY / 2, z - SCZ / 2);
	}

	// -- Assign sibling pointers
	for(int x = 0; x < SCX; x++)
	for(int y = 0; y < SCY; y++)
	for(int z = 0; z < SCZ; z++)
	{
		Chunk *c = this->chunk_map[ vector3i(x,y,z) ];

		c->left = this->getChunk( x - 1, y, z );
		c->right = this->getChunk( x + 1, y, z );
		c->below = this->getChunk( x, y - 1, z );
		c->above = this->getChunk( x, y + 1, z );
		c->front = this->getChunk( x, y, z - 1 );
		c->back = this->getChunk( x, y, z + 1 );
	}

	// -- Pre-generate the chunks
	for (int x = 0; x < SCX; x++)
	for (int y = 0; y < SCY; y++)
	for (int z = 0; z < SCZ; z++)
	{
		this->chunk_map[ vector3i(x,y,z) ]->generate( this->seed );
	}
}

void World::render(const glm::mat4 &pv)
{
	float ud = 1.0 / 0.0;
	int ux = -1;
	int uy = -1;
	int uz = -1;

	for(int x = 0; x < SCX; x++)
	{
		for(int y = 0; y < SCY; y++)
		{
			for(int z = 0; z < SCZ; z++)
			{
				if ( c[x][y][z] == nullptr )
				{
					continue;
				}

				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(c[x][y][z]->ax * CHUNK_WIDTH, c[x][y][z]->ay * CHUNK_HEIGHT, c[x][y][z]->az * CHUNK_LENGTH));
				glm::mat4 mvp = pv * model;

				// Is this chunk on the screen?
				glm::vec4 center = mvp * glm::vec4(CHUNK_WIDTH / 2, CHUNK_HEIGHT / 2, CHUNK_LENGTH / 2, 1);

				float d = glm::length(center);
				center.x /= center.w;
				center.y /= center.w;

				// If it is behind the camera, don't bother drawing it
				if(center.z < -CHUNK_HEIGHT / 2)
					continue;

				// If it is outside the screen, don't bother drawing it
				if(std::fabs(center.x) > 1 + std::fabs(CHUNK_HEIGHT * 2 / center.w) || std::fabs(center.y) > 1 + std::fabs(CHUNK_HEIGHT * 2 / center.w))
					continue;

				// If this chunk is not initialized, skip it
				if( !c[x][y][z]->initialized )
				{
					// But if it is the closest to the camera, mark it for initialization
					if(ux < 0 || d < ud)
					//if ( d < ud )
					{
						ud = d;
						ux = x;
						uy = y;
						uz = z;
						//GaiaCraft::addChunkToGenerateQueue( ux, uy, uz, (int)this->seed, this->c[ux][uy][uz] );
					}
					continue;
				}

				// If this chunk has not yet been generated, add it to the queue
				/*if ( c[x][y][z]->generated == false && c[x][y][z]->generating == false )
				{
					GaiaCraft::addChunkToGenerateQueue( x, y, z, (int)this->seed, this->c[x][y][z] );
					c[x][y][z]->generating = true;
				}*/

				Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(mvp) );

				c[x][y][z]->render();
			}
		}
	}

	// TODO: Add to a queue of chunks to update in a second thread
	if ( ux >= 0 )
	{
		GaiaCraft::addChunkToGenerateQueue( ux, uy, uz, (int)this->seed, this->c[ux][uy][uz] );
	}
}
