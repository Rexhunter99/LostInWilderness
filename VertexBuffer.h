#pragma once

#ifndef __VERTEXBUFFER_H__
#define __VERTEXBUFFER_H__

#include <vector>

class Vertex;
class Renderer;


/** @class VertexBuffer
 **	@note VertexBuffer is a wrapper for Vertex Arrays and Vertex Buffer Objects to simplify feature support of the two.
 */
class VertexBuffer
{
public:

	VertexBuffer();
	~VertexBuffer();

	/** @fn add( vector<Vertex*> & vertices )
	 ** @param vertices An STL vector of Vertex pointers
	 ** Adds the contents of the Vertex vector to the internal Vertex Buffer
	 */
	void add( std::vector<Vertex*> &vertices );

	/** @fn add( first, last )
	 ** @param first An iterator for the first element to add to the vertex buffer
	 ** @param last An iterator that is the very last element to add to the vertex buffer
	 ** Adds the contents of the Vertex vector to the internal Vertex Buffer
	 */
	//void add( std::vector<Vertex*>::iterator first, std::vector<Vertex*>::iterator last );

	/** @fn remove( first, last )
	 ** @param first An iterator for the first element to add to the vertex buffer
	 ** @param last An iterator that is the very last element to add to the vertex buffer
	 ** Adds the contents of the Vertex vector to the internal Vertex Buffer
	 */
	//void remove( std::vector<Vertex*>::iterator first, std::vector<Vertex*>::iterator last );

	/** @fn clear()
	 ** Clear the buffer to empty status, freeing all used memory.
	 */
	void clear();

private:

	unsigned int			m_vbo;
	std::vector<Vertex*>	m_vertices;
};

#endif // __VERTEXBUFFER_H__
