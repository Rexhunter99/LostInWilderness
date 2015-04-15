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

typedef v4t2	Vertex;

#endif // __VERTEX_H__
