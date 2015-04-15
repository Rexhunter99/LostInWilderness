
#pragma once

#include <ctime>
#include <map>
#include <string>
#include <thread>

#include "Config.h"
#include "ResourceManager.h"
#include "Renderer.h"


class Chunk;


class GaiaCraft
{
public:
	static GaiaCraft					*iGaiaCraft;
	class Config						*config;
	std::map<std::string,std::thread>	threads;

	GaiaCraft();
	~GaiaCraft();

	static void addChunkToGenerateQueue( int x, int y, int z, int seed, Chunk *chunk );
	static void addChunkToUpdateQueue( Chunk *chunk );
	static void chunkUpdateThread( void );

	int run();

	static GaiaCraft * getInstance() { return iGaiaCraft; }
};
