#pragma once

#ifndef __VERTEXBUFFER_H__
#define __VERTEXBUFFER_H__

#include "Vertex.h"
#include <vector>


class Renderer;


/** @class VertexBuffer
 **	@note VertexBuffer is a wrapper for Vertex Arrays and Vertex Buffer Objects to simplify feature support of the two.
 */
template <class T = Vertex>
class VertexBuffer
{
public:

	VertexBuffer();
	~VertexBuffer();

	/** @fn add( vector<T*> & vertices )
	 ** @param vertices An STL vector of T pointers
	 ** Adds the contents of the T vector to the internal Vertex Buffer
	 */
	void add( std::vector<T*> &vertices );

	/** @fn add( first, last )
	 ** @param first An iterator for the first element to add to the vertex buffer
	 ** @param last An iterator that is the very last element to add to the vertex buffer
	 ** Adds the contents of the Vertex vector to the internal Vertex Buffer
	 */
	//void add( std::vector<T*>::iterator first, std::vector<T*>::iterator last );

	/** @fn clear()
	 ** Clear the buffer to empty status, freeing all used memory.
	 */
	void clear();

	/** @fn render()
	 ** Upload the vertex buffer data to the Video RAM and render the buffer
	 */
	void render();

	void addBlock( float x1, float y1, float z1, float x2, float y2, float z2, int flags );
	void addCylinder( float x, float y, float z, float w, float l, float h, int segments );

private:

	unsigned int			m_vbo;
	std::vector<T*>			m_vertices;
};

#endif // __VERTEXBUFFER_H__
