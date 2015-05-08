#pragma once

#ifndef __VERTEX_H__
#define __VERTEX_H__

#include <stdint.h>


class Vertex
{
public:

	float     x, y, z, w;
	float     nx, ny, nz;
	float     tx, ty;
	uint32_t  rgba;

  Vertex( float x, float y, float z, float w, float tx, float ty, uint32_t rgba )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->nx = 0.0f;
		this->ny = 0.0f;
		this->nz = 0.0f;
		this->tx = tx;
		this->ty = ty;
		this->rgba = rgba;
	}

  Vertex( float x, float y, float z, float w, float nx, float ny, float nz, float tx, float ty )
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
		this->rgba = 0xFFFFFF;
	}

	Vertex( float x, float y, float z, float w, float nx, float ny, float nz, float tx, float ty, uint32_t rgba )
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
		this->rgba = rgba;
	}

};

#endif // __VERTEX_H__
