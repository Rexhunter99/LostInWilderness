#pragma once

#ifndef __NOISE_H__
#define __NOISE_H__

#include <vector>
#include <glm/fwd.hpp>

namespace Noise {

/** @class Perlin
 ** @note Implements Ken Perlin's Simplex noise as well as the original Perlin noise.
 ** @version 14
 **/
class Perlin
{
public:

	Perlin();
	Perlin( unsigned int seed );

	void reseed( unsigned int seed );

	float perlin2D( float x, float y, int octaves, float persistence, float offset = 0.0f, float scale = 1.0f );
	float perlin3D( float x, float y, float z, int octaves, float persistence, float offset = 0.0f, float scale = 1.0f );
	float simplex2D( float x, float y, int octaves, float persistence, float offset = 0.0f, float scale = 1.0f );
	float simplex3D( float x, float y, float z, int octaves, float persistence, float offset = 0.0f, float scale = 1.0f );

private:

	float getPerlin2D( float x, float y );
	float getPerlin3D( float x, float y, float z );

	float fade( float t );
	float lerp( float t, float a, float b );
	float grad( int hash, float x, float y );
	float grad( int hash, float x, float y, float z );

	std::vector<int> p;

};

class Cell
{
public:

	Cell();
	Cell( int cells_x, int cells_y, int num_points, unsigned int seed );

	float noise( float x, float y );

private:

	unsigned int						seed;
	std::vector<std::vector<glm::vec3>>	grid;

};

};

#endif // __NOISE_H__
