
#pragma once

#if !defined( __LOSTINWILDERNESS_H__ )
#define __LOSTINWILDERNESS_H__

#include <ctime>
#include <map>
#include <string>
#include <thread>

#include <glm/glm.hpp>

#include "ResourceManager.h"
#include "Renderer.h"


class Camera
{
public:

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 lookat;
	glm::vec3 angle;
};

class Chunk;
class Config;
class World;

class GaiaCraft
{
public:
	static GaiaCraft					*iGaiaCraft;
	Config								*config;
	Camera								*camera;
	std::map<std::string,std::thread>	threads;

	GaiaCraft();
	~GaiaCraft();

	static void addChunkToGenerateQueue( int x, int y, int z, int seed, Chunk *chunk );
	static void addChunkToUpdateQueue( Chunk *chunk );
	static void addChunkToSaveQueue( Chunk *chunk );
	static void chunkUpdateThread( void );
	static void chunkFileIOThread( void );

	int run();

	static GaiaCraft * getInstance() { return iGaiaCraft; }
};

#endif
