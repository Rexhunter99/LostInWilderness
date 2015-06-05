#pragma once

#ifndef __VERTEX_H__
#define __VERTEX_H__


class v4t2
{
public:

	float	x,y,z,w;
	float	tx,ty;

	v4t2( float x, float y, float z, float w, float tx, float ty )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->tx = tx;
		this->ty = ty;
	}
};

class v4t2c4
{
public:

	float	x,y,z,w;
	float	tx,ty;
	unsigned char r,g,b,a;

	v4t2c4( float x, float y, float z, float w, float tx, float ty )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->tx = tx;
		this->ty = ty;
	}
};

class v4n3t2c4
{
public:

	float	x,y,z,w;
	float	nx,ny,nz;
	float	tx,ty;
	unsigned char r,g,b,a;

	v4n3t2c4( float x, float y, float z, float w, float tx, float ty )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->nx = 0;
		this->ny = -1;
		this->nz = 0;
		this->tx = tx;
		this->ty = ty;
		this->r = 255;
		this->g = 255;
		this->b = 255;
		this->a = 255;
	}

	v4n3t2c4( float x, float y, float z, float w, float nx, float ny, float nz, float tx, float ty )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->nx = nx;
		this->ny = ny;
		this->nz = nz;
		this->tx = tx;
		this->ty = ty;
		this->r = 255;
		this->g = 255;
		this->b = 255;
		this->a = 255;
	}
};

typedef v4n3t2c4	Vertex;

#endif // __VERTEX_H__
