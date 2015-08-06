#pragma once

#ifndef __WORLD_H__
#define __WORLD_H__

#include <stdint.h>
#include <cstdlib>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <glm/fwd.hpp>
#include "vectors.h"

class Chunk;
class Block;


/** @class World
 ** @version 2
 ** @author Rexhunter99
 ** @brief The world class contains all the loaded chunks as well as world-specific information

 ** The world class is considered as a superchunk, it contains all the world specific information
 ** as well as handling a lot of top-level access to the worldspace itself.
 ** World is written in a way that should support multiple instances of the World class for servers
 ** that may want to have several worldspaces available to players.
 **/
class World
{
public:
	// Loaded chunks
	typedef std::unordered_map<vector3i, std::unique_ptr<Chunk>, vector3i_hash, vector3i_hash> chunk_map_t;
	//typedef std::map<vector3i, std::unique_ptr<Chunk>> chunk_map_t;
	chunk_map_t chunk_map;
	std::mutex	chunk_mutex;

	/** @var seed
	 ** The seed this world uses for randomly generating its terrain
	 **/
	time_t		seed;

	/** @var name
	 ** The internal name of the world, usually in English.
	 **/
	std::string	name;

	/** @var locale_name
	 ** The name of the world in the 'locale' text
	 **/
	std::wstring locale_name;


	World( std::string name = "world" );
	~World();

	/** @fn getBlock( int64_t x, int64_t y, int64_t z ) const
	 ** @return Block* A pointer to the block at the position in the world described by <x,y,z>
	 ** @param x The x co-ordinate on the horizontal plane (East<->West)
	 ** @param y The y co-ordinate on the vertical plane (Up<->Down)
	 ** @param z The z co-ordinate on the horizontal plane (North<->South)
	 **/
	Block * getBlock( int64_t x, int64_t y, int64_t z ) const;

	/** @fn setBlock( int64_t x, int64_t y, int64_t z, Block *block )
	 ** @param x The x co-ordinate on the horizontal plane (East<->West)
	 ** @param y The y co-ordinate on the vertical plane (Up<->Down)
	 ** @param z The z co-ordinate on the horizontal plane (North<->South)
	 ** @param block An instance of a Block class to place in the world
	 **/
	void setBlock( int64_t x, int64_t y, int64_t z, Block *block );

	/** @fn getChunk( int64_t x, int64_t y, int64_t z ) const
	 ** @return Chunk* A pointer to the Chunk at the position in the world described by <x,y,z>
	 ** @param x The x co-ordinate on the horizontal plane (East<->West)
	 ** @param y The y co-ordinate on the vertical plane (Up<->Down)
	 ** @param z The z co-ordinate on the horizontal plane (North<->South)
	 **/
	Chunk * getChunk( int64_t x, int64_t y, int64_t z ) const;

	/** @fn setChunk( int64_t x, int64_t y, int64_t z, Block *block )
	 ** @param x The x co-ordinate on the horizontal plane (East<->West)
	 ** @param y The y co-ordinate on the vertical plane (Up<->Down)
	 ** @param z The z co-ordinate on the horizontal plane (North<->South)
	 ** @param chunk An instance of a Chunk class to place in the world
	 **/
	void setChunk( int64_t x, int64_t y, int64_t z, Chunk *chunk );

	/** @fn preGenerate( int sx, int sy, int sz, int distance )
	 ** @brief Pre-generate the world from the supplied co-ordinates in all directions with the supplied distance
	 ** @param sx The x-coordinate in the world to generate from
	 ** @param sy The y-coordinate in the world to generate from
	 ** @param sz The z-coordinate in the world to generate from
	 ** @param distance The distance in chunks to generate out from x,y,z
	 ** @todo Allow replacement of Chunk
	 **/
	void preGenerate( int sx, int sy, int sz, int distance );

	/** @fn render( const glm::mat4 &pv )
	 ** @param pv Projection*View Matrix to apply to the rendering
	 **/
	void render(const glm::mat4 &pv);

	/** @fn update()
	 **/
	void update();
};

#endif // __WORLD_H__
