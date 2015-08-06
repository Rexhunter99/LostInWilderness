
#include "VertexBuffer.h"
#include "Renderer.h"

#include <GL/glew.h>


template <typename P, typename N, typename T, typename C>
VertexBuffer<P,N,T,C>::VertexBuffer()
{
	lock.lock();
	if ( !GLEW_VERSION_3_0 && !GLEW_ARB_vertex_buffer_object )
	{
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
	}
	else
	{
		glGenBuffers( 1, &this->m_vbo );
	}
	lock.unlock();
}

template <typename P, typename N, typename T, typename C>
VertexBuffer<P,N,T,C>::~VertexBuffer()
{
	lock.lock();
	if ( !GLEW_VERSION_3_0 && !GLEW_ARB_vertex_buffer_object )
	{
	}
	else
	{
		glDeleteBuffers( 1, &this->m_vbo );
	}
	lock.unlock();
}

template <typename P, typename N, typename T, typename C>
void VertexBuffer<P,N,T,C>::bind()
{
	lock.lock();
	glBindBuffer( GL_ARRAY_BUFFER, this->m_vbo );
	lock.unlock();
}

template <typename P, typename N, typename T, typename C>
void VertexBuffer<P,N,T,C>::upload()
{
	lock.lock();
	glNamedBufferData( this->m_vbo, this->m_vertices.size() * sizeof(T), &this->m_vertices[0], GL_DYNAMIC_DRAW );
	lock.unlock();
}

template <typename P, typename N, typename T, typename C>
void VertexBuffer<P,N,T,C>::render()
{
	std::lock_guard<std::mutex> lock(lock);

	Shader *shader = Renderer::default_shader;

	if ( !GLEW_VERSION_3_0 && !GLEW_ARB_vertex_buffer_object )
	{
		// -- We need to use the old Vertex Array method
		glVertexPointer( 4, GL_FLOAT, sizeof( P ) * 4, &this->positions[0] );
		glNormalPointer( GL_FLOAT, sizeof( N ) * 3, (float*)&this->normals[0] );
		glTexCoordPointer( 2, GL_FLOAT, sizeof( T ) * 2, (float*)&this->texcoords[0] );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( C ) * 4, (float*)&this->colors[0] );

		glDrawArrays( GL_TRIANGLES, 0, this->positions.size() / 4 );
	}
	else
	{
		// -- We can use the modern Vertex Buffer Object method
		glBindBuffer( GL_ARRAY_BUFFER, this->m_vbo );

		glVertexAttribPointer( shader->getAttrib( "v_position" ), 4, GL_FLOAT, GL_FALSE, sizeof(T), 0 );
		glVertexAttribPointer( shader->getAttrib( "v_normal" ), 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*) (sizeof(float) * 4) );
		glVertexAttribPointer( shader->getAttrib( "v_texcoord" ), 2, GL_FLOAT, GL_FALSE, sizeof(T), (void*) (sizeof(float) * 7) );
		glVertexAttribPointer( shader->getAttrib( "v_diffuse" ), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(T), (void*) (sizeof(float) * 9) );

		glDrawArrays( GL_TRIANGLES, 0, this->positions.size() / 4 );
	}
}
