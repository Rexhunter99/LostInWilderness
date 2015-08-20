
#pragma once

#if !defined( __LOSTINWILDERNESS_H__ )
#define __LOSTINWILDERNESS_H__

#pragma warning( disable : 4244 )

#include <ctime>
#include <map>
#include <string>
#include <thread>

#include <glm/glm.hpp>

#include "Camera.h"
#include "ResourceManager.h"


class Chunk;
class Config;
class World;


/** @class LostInWilderness
 ** @version 2
 ** @author Rexhunter99
 ** @brief
 **
 **/
class LostInWilderness
{
public:
	static LostInWilderness				*iGaiaCraft;
	Config								*config;
	Camera								*camera;
	std::map<std::string,std::thread>	threads;

	LostInWilderness();
	~LostInWilderness();

	static void addChunkToGenerateQueue( int x, int y, int z, int seed, Chunk *chunk );
	static void addChunkToUpdateQueue( Chunk *chunk );
	static void addChunkToSaveQueue( Chunk *chunk );
	static void chunkThread( void );

	/** @fn run()
	 ** @brief Process the game state
	 **
	 **
	 **/
	int run();

	static LostInWilderness * getInstance() { return iGaiaCraft; }
};

#endif
