#pragma once

#ifndef __WORLD_H__
#define __WORLD_H__

#include <cstdlib>
#include <string>
#include <tuple>
#include <unordered_map>

#include <glm/fwd.hpp>
#include <vector.h>

#define SCX 12
#define SCY 1
#define SCZ 12

class Chunk;
class Block;


/** @class World
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
	Chunk		*c[SCX][SCY][SCZ];
	std::unordered_map< vector3<uint64_t>, Chunk*> c;

	// The world seed
	time_t		seed;

	// The internal name of the world
	std::string	name;

	// The locale name of the world
	std::wstring locale_name;


	World( std::string name = "world" );
	~World();

	/** @fn get( int x, int y, int z ) const
	 ** @return Block* A pointer to the block at the position in the world described by <x,y,z>
	 ** @param x The x co-ordinate on the horizontal plane (East<->West)
	 ** @param y The y co-ordinate on the vertical plane (Up<->Down)
	 ** @param z The z co-ordinate on the horizontal plane (North<->South)
	 **/
	Block * get(int x, int y, int z) const;

	/** @fn set( int x, int y, int z, Block *block )
	 ** @param x The x co-ordinate on the horizontal plane (East<->West)
	 ** @param y The y co-ordinate on the vertical plane (Up<->Down)
	 ** @param z The z co-ordinate on the horizontal plane (North<->South)
	 ** @param block An instance of a Block class to place in the world
	 **/
	void set(int x, int y, int z, Block *block );

	void render(const glm::mat4 &pv);
};

#endif // __WORLD_H__
