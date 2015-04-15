
#include "Noise.h"

#include <cmath>
#include <algorithm>
#include <random>

#include <glm/glm.hpp>

using namespace Noise;


Perlin::Perlin()
{
	// -- Initialise the permutation to the reference permutation
	this->p = {
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
		8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
		35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
		134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
		55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
		250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
		189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
		43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
		97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
		107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

	// -- Duplicate the permutation
	p.insert( p.end(), p.begin(), p.end() );
}

Perlin::Perlin( unsigned int seed )
{
	this->reseed( seed );
}

void Perlin::reseed( unsigned int seed )
{
	p.resize(256);

	// Fill p with values from 0 to 255
	std::iota(p.begin(), p.end(), 0);

	// Initialize a random engine with seed
	std::default_random_engine engine(seed);

	// Suffle  using the above random engine
	std::shuffle(p.begin(), p.end(), engine);

	// Duplicate the permutation vector
	p.insert(p.end(), p.begin(), p.end());
}

float Perlin::getPerlin2D( float x, float y )
{
	// Find the unit cube that contains the point
	int X = (int) floor(x) & 255;
	int Y = (int) floor(y) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);

	// Compute fade curves for each of x, y, z
	float u = fade(x);
	float v = fade(y);

	// Hash coordinates of the 4 square corners
	int A = p[X] + Y;
	int AA = p[A];
	int AB = p[A + 1];
	int B = p[X + 1] + Y;
	int BA = p[B];
	int BB = p[B + 1];

	// Add blended results from 4 corners of square
	float res = lerp(v, lerp(u, grad(p[AA], x, y), grad(p[BA], x-1, y)), lerp(u, grad(p[AB], x, y-1), grad(p[BB], x-1, y-1)));
	return ( ( (res + 1.0)/2.0 ) - 0.5f );
//	return ( res + 1.0f ) / 2.0f;
}

float Perlin::getPerlin3D( float x, float y, float z )
{
	// Find the unit cube that contains the point
	int X = (int) floor(x) & 255;
	int Y = (int) floor(y) & 255;
	int Z = (int) floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	float u = fade(x);
	float v = fade(y);
	float w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	float res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x-1, y, z)), lerp(u, grad(p[AB], x, y-1, z), grad(p[BB], x-1, y-1, z))),	lerp(v, lerp(u, grad(p[AA+1], x, y, z-1), grad(p[BA+1], x-1, y, z-1)), lerp(u, grad(p[AB+1], x, y-1, z-1),	grad(p[BB+1], x-1, y-1, z-1))));
	return ( (res + 1.0) / 2.0 ) - 0.5f;
}

float Perlin::perlin2D( float x, float y, int octaves, float persistence, float offset, float scale )
{
	float n = 0;
	float strength = 1.0f;
	float frequency = 1.0f;
	float lacunarity = 2.0f;

	for ( int i = 0; i < octaves; i++ )
	{
		n += strength * this->getPerlin2D( x * frequency, y * frequency );
		frequency *= lacunarity;
		strength *= persistence;
	}

	return n * scale + offset;
}

float Perlin::perlin3D( float x, float y, float z, int octaves, float persistence, float offset, float scale )
{
	float n = 0;
	float strength = 1.0f;
	float frequency = 1.0f;

	for(int i = 0; i < octaves; i++)
	{
		n += strength * this->getPerlin3D( x * frequency, y * frequency, z * frequency );
		frequency *= 2.0;
		strength *= persistence;
	}

	return n * scale + offset;
}

float Perlin::fade( float t )
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float Perlin::lerp(float t, float a, float b)
{
	return a + t * (b - a);
}

float Perlin::grad( int hash, float x, float y )
{
	int h = hash & 15;
	// Convert lower 4 bits of hash inot 12 gradient directions
	float  u = h < 8 ? x : y,
		   v = h < 4 ? y : x;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float Perlin::grad( int hash, float x, float y, float z )
{
	int h = hash & 15;
	// Convert lower 4 bits of hash inot 12 gradient directions
	float  u = h < 8 ? x : y,
		   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}


Cell::Cell( int cells_x, int cells_y, int num_points, unsigned int seed )
{
	this->seed = seed;
	this->grid.resize( cells_x * cells_y );

	srand( this->seed );

	// -- Generate points in the square
	for ( int p=0; p<num_points; p++ )
	{
		int x = rand() % cells_x;
		int y = rand() % cells_y;

		std::vector<glm::vec3> &points = this->grid[ x + ( y * cells_x ) ];

		float px = float(rand() & 1023) / 1023.0f;
		float py = float(rand() & 1023) / 1023.0f;

		points.push_back( glm::vec3( float(x) + px, float(y) + py, 0.0f ) );
	}

	// TODO: Implement a wrap pass here that makes the cells on the far right and bottom of the grid be the same as the cells opposite themself

}

float Cell::noise( float x, float y )
{
	return 0.0f;
}
