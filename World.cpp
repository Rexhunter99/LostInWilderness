
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
#include <sstream>


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
	this->chunk_map.clear();
}


Chunk * World::getChunk(int64_t x, int64_t y, int64_t z) const
{
	//std::lock_guard<std::mutex> lock((std::mutex&)this->chunk_mutex);

	chunk_map_t::const_iterator i = this->chunk_map.find( vector3i( x, y, z ) );

	if ( i == this->chunk_map.end() )
	{
		return nullptr;
	}

	return i->second.get();
}

void World::setChunk(int64_t x, int64_t y, int64_t z, Chunk *chunk )
{
	//std::lock_guard<std::mutex> lock(this->chunk_mutex);

	#ifdef _DEBUG
	if ( this->chunk_map.find(vector3i(x,y,z)) != this->chunk_map.end() )
	{
		std::cout << "[ WARNING ] Attempting to replace chunk at " << x << "," << y << "," << z << std::endl;
	}
	#endif // _DEBUG

	this->chunk_map[vector3i(x,y,z)].reset(chunk);
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
	int64_t cx = x / CHUNK_WIDTH;
	int64_t cy = y / CHUNK_HEIGHT;
	int64_t cz = z / CHUNK_LENGTH;

	Chunk *chunk = this->getChunk( cx, cy, cz );

	if ( chunk != nullptr )
	{
		//this->chunk_map[vector3i(cx,cy,cz)]->set( x & (CHUNK_WIDTH - 1), y & (CHUNK_HEIGHT - 1), z & (CHUNK_LENGTH - 1), block );
		chunk->set( x & (CHUNK_WIDTH - 1), y & (CHUNK_HEIGHT - 1), z & (CHUNK_LENGTH - 1), block );
	}
}

void World::preGenerate( int sx, int sy, int sz, int distance )
{
	int vd = Config::getGlobal()->getInteger( "renderer.view_distance" );

	// -- Allocate the chunks
	//this->chunk_mutex.lock();
	for (int x = sx - vd; x <= sx + vd; x++)
	for (int y = 0; y < 1; y++)
	for (int z = sz - vd; z <= sz + vd; z++)
	{
		Chunk *c = new Chunk(x, y, z);
		this->chunk_map.insert( std::make_pair(vector3i(x,y,z), std::unique_ptr<Chunk>(c) ) );
	}
	//this->chunk_mutex.unlock();

	// -- Assign sibling pointers
	for (int x = sx - vd; x <= sx + vd; x++)
	for (int y = 0; y < 1; y++)
	for (int z = sz - vd; z <= sz + vd; z++)
	{
		Chunk *c = this->chunk_map[ vector3i(x,y,z) ].get();

		c->left = this->getChunk( x - 1, y, z );
		c->right = this->getChunk( x + 1, y, z );
		c->below = this->getChunk( x, y - 1, z );
		c->above = this->getChunk( x, y + 1, z );
		c->front = this->getChunk( x, y, z - 1 );
		c->back = this->getChunk( x, y, z + 1 );
	}

	// -- Pre-generate the chunks
	for (int x = sx - vd; x <= sx + vd; x++)
	for (int y = 0; y < 1; y++)
	for (int z = sz - vd; z <= sz + vd; z++)
	{
		this->chunk_map[ vector3i(x,y,z) ]->generate( this->seed );
	}
}

void World::render(const glm::mat4 &pv)
{
	int64_t vd = Config::getGlobal()->getInteger( "renderer.view_distance" );
	Camera *camera = LostInWilderness::iGaiaCraft->camera;

	int64_t cx = floorf( (float)camera->position.x / (float)CHUNK_WIDTH );
	int64_t cy = floorf( (float)camera->position.y / (float)CHUNK_HEIGHT );
	int64_t cz = floorf( (float)camera->position.z / (float)CHUNK_LENGTH );

	for ( int64_t x = cx - vd; x <= cx + vd; x++)
	{
		for ( int64_t y = cy; y < 1; y++)
		{
			for ( int64_t z = cz - vd; z <= cz + (vd); z++)
			{
				Chunk *chunk = this->getChunk( x, y, z );

				// -- If there is no chunk then create one, assign its neighbours, and add it to the generation queue
				if ( chunk == nullptr )
				{
					//this->chunk_mutex.lock();
					this->chunk_map.insert( std::make_pair(vector3i(x,y,z), std::unique_ptr<Chunk>(new Chunk(x, y, z)) ) );
					chunk = this->chunk_map[ vector3i(x,y,z) ].get();
					//this->chunk_mutex.unlock();

					chunk->left = this->getChunk( x - 1, y, z );
					chunk->right = this->getChunk( x + 1, y, z );
					chunk->below = this->getChunk( x, y - 1, z );
					chunk->above = this->getChunk( x, y + 1, z );
					chunk->front = this->getChunk( x, y, z - 1 );
					chunk->back = this->getChunk( x, y, z + 1 );

					// -- If chunk exists on disk, load it instead.
					std::stringstream name;
					name << "../world/" << x << "," << y << "," << z << ".chunk";

					// -- Open the file stream for binary output
					std::fstream f;
					f.open( name.str().c_str(), std::ios::in | std::ios::binary );

					// -- If the file stream is not open, there is no file so add it to the generate queue (new chunk)
					if ( !f.is_open() )
					{
						LostInWilderness::addChunkToGenerateQueue( x, y, z, (int)this->seed, chunk );
					}
					// -- Otherwise just load it into memory
					// TODO: move to chunk load to threading?
					else
					{
						chunk->generated = true;

						f.close();
						chunk->load();
					}

					continue;
				}

				// If this chunk is not initialized, skip it
				if( !chunk->initialized )
				{
					LostInWilderness::addChunkToGenerateQueue( x, y, z, (int)this->seed, chunk );
					continue;
				}

				// -- Is the chunk inside the view frustum?
				//if ( camera->boxInFrustum(...) != false )
				//{
				//	return;
				//}

				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunk->ax * CHUNK_WIDTH, chunk->ay * CHUNK_HEIGHT, chunk->az * CHUNK_LENGTH));
				glm::mat4 mvp = pv * model;

				Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(mvp) );

				chunk->render();
			}
		}
	}
}

void World::update()
{
	int64_t vd = Config::getGlobal()->getInteger( "renderer.view_distance" );
	Camera *camera = LostInWilderness::iGaiaCraft->camera;

	int64_t cx = std::floor( (float)camera->position.x / (float)CHUNK_WIDTH );
	//int64_t cy = std::floor( (float)camera->position.y / (float)CHUNK_HEIGHT );
	int64_t cz = std::floor( (float)camera->position.z / (float)CHUNK_LENGTH );

	int64_t edge_w = cx - vd;
	int64_t edge_n = cz - vd;
	int64_t edge_e = cx + vd;
	int64_t edge_s = cz + vd;

    /*auto it = this->chunk_map.begin();
	while ( it != this->chunk_map.end() )
	{
	    auto new_it = it;
		auto k = new_it->first;
		if ( k.x > edge_e || k.z > edge_s || k.x < edge_w || k.z < edge_n )
		{
			this->chunk_mutex.lock();
			this->chunk_map.erase( k );
			this->chunk_mutex.unlock();
		}
	}*/
}
