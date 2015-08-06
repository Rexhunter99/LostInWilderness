#pragma once

#ifndef __VERTEXBUFFER_H__
#define __VERTEXBUFFER_H__

#include "Vertex.h"
#include <cstdint>
#include <mutex>
#include <vector>


class Renderer;


/** @class VertexBuffer
 **	@note VertexBuffer is a wrapper for Vertex Arrays and Vertex Buffer Objects to simplify feature support of the two.
 */
template <typename P = float, typename N = float, typename T = float, typename C = uint8_t>
class VertexBuffer
{

public:

	VertexBuffer();
	~VertexBuffer();

	/** @fn add( P *ptr_p, N *ptr_n, T *ptr_t, C *ptr_c )
	 **
	 ** Adds the contents of the T vector to the internal Vertex Buffer
	 */
	void add( P *ptr_p, N *ptr_n, T *ptr_t, C *ptr_c );

	/** @fn add( vector<P> v_p, vector<N> v_n, vector<T> v_t, vector<C> v_c )
	 **
	 ** Adds the contents of the T vector to the internal Vertex Buffer
	 */
	void add( std::vector<P> v_p, std::vector<N> v_n, std::vector<T> v_t, std::vector<C> v_c );

	/** @fn bind()
	 **/
	void bind();

	/** @fn upload()
	 **/
	void upload();

	/** @fn clear()
	 **
	 ** Clear the buffer to empty status, freeing all used memory.
	 */
	void clear();

	/** @fn render()
	 **
	 ** Upload the vertex buffer data to the Video RAM and render the buffer
	 */
	void render();

	void addBlock( float x1, float y1, float z1, float x2, float y2, float z2, int flags );
	void addCylinder( float x, float y, float z, float w, float l, float h, int segments );

private:

	unsigned int		m_vbo;
	std::mutex			lock;
	std::vector<P>		positions;
	std::vector<N>		normals;
	std::vector<T>		texcoords;
	std::vector<C>		colors;
};

#endif // __VERTEXBUFFER_H__
