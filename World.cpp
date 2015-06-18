
#include "World.h"
#include "Chunk.h"
#include "Config.h"
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
	//int64_t cx = (x + CHUNK_WIDTH * (vd / 2)) / CHUNK_WIDTH;
	//int64_t cy = (y + CHUNK_HEIGHT * (1 / 2)) / CHUNK_HEIGHT;
	//int64_t cz = (z + CHUNK_LENGTH * (vd / 2)) / CHUNK_LENGTH;
	int64_t cx = x / CHUNK_WIDTH;
	int64_t cy = y / CHUNK_HEIGHT;
	int64_t cz = z / CHUNK_LENGTH;

	Chunk *chunk = this->getChunk( cx, cy, cz );
	if ( chunk == nullptr )
		return;

	//this->chunk_map[vector3i(cx,cy,cz)]->set( x & (CHUNK_WIDTH - 1), y & (CHUNK_HEIGHT - 1), z & (CHUNK_LENGTH - 1), block );
	chunk->set( x & (CHUNK_WIDTH - 1), y & (CHUNK_HEIGHT - 1), z & (CHUNK_LENGTH - 1), block );
}

void World::preGenerate( int sx, int sy, int sz, int distance )
{
	int vd = Config::getGlobal()->getInteger( "renderer.view_distance" );

	// -- Allocate the chunks
	for (int x = 0; x < vd; x++)
	for (int y = 0; y < 1; y++)
	for (int z = 0; z < vd; z++)
	{
		this->chunk_map[ vector3i(x,y,z) ] = new Chunk(x - vd / 2, y - 1 / 2, z - vd / 2);
	}

	// -- Assign sibling pointers
	for(int x = 0; x < vd; x++)
	for(int y = 0; y < 1; y++)
	for(int z = 0; z < vd; z++)
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
	for (int x = 0; x < vd; x++)
	for (int y = 0; y < 1; y++)
	for (int z = 0; z < vd; z++)
	{
		this->chunk_map[ vector3i(x,y,z) ]->generate( this->seed );
	}
}

void World::render(const glm::mat4 &pv)
{
	int vd = Config::getGlobal()->getInteger( "renderer.view_distance" );
	Camera *camera = GaiaCraft::iGaiaCraft->camera;

	int64_t cx = roundf( camera->position.x / CHUNK_WIDTH );
	int64_t cy = roundf( camera->position.y / CHUNK_HEIGHT );
	int64_t cz = roundf( camera->position.z / CHUNK_LENGTH );

	int ud = 0;
	int ux = -1;
	int uy = -1;
	int uz = -1;

	for(int x = cx - (vd); x < cx + (vd); x++)
	{
		for(int y = 0; y < 1; y++)
		{
			for(int z = cz - (vd); z < cz + (vd); z++)
			{
				Chunk *chunk = this->getChunk( x, y, z );

				if ( chunk == nullptr )
				{
					continue;
				}

				//if ( x  > vd/2 || z > vd/2 )sion
				//	continue;

				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunk->ax * CHUNK_WIDTH, chunk->ay * CHUNK_HEIGHT, chunk->az * CHUNK_LENGTH));
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
				if( !chunk->initialized )
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
				/*if ( chunk->generated == false && chunk->generating == false )
				{
					GaiaCraft::addChunkToGenerateQueue( x, y, z, (int)this->seed, this->chunk );
					chunk->generating = true;
				}*/

				Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(mvp) );

				chunk->render();
			}
		}
	}

	// TODO: Add to a queue of chunks to update in a second thread
	if ( ux >= 0 )
	{
		GaiaCraft::addChunkToGenerateQueue( ux, uy, uz, (int)this->seed, this->getChunk( ux, uy, uz ) );
	}
}
