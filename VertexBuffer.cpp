
#include "VertexBuffer.h"
#include "Renderer.h"

#include <GL/glew.h>


template <class T>
VertexBuffer<T>::VertexBuffer()
{
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
}

template <class T>
VertexBuffer<T>::~VertexBuffer()
{
	if ( !GLEW_VERSION_3_0 && !GLEW_ARB_vertex_buffer_object )
	{
	}
	else
	{
		glDeleteBuffers( 1, &this->m_vbo );
	}
}

template <class T>
void VertexBuffer<T>::render()
{
	Shader *shader = Renderer::default_shader;

	if ( !GLEW_VERSION_3_0 && !GLEW_ARB_vertex_buffer_object )
	{
		// -- We need to use the old Vertex Array method
		glVertexPointer( 4, GL_FLOAT, sizeof( T ), &this->m_vertices[0] );
		glNormalPointer( GL_FLOAT, sizeof( T ), (float*)&this->m_vertices[0] + 4 );
		glTexCoordPointer( 2, GL_FLOAT, sizeof( T ), (float*)&this->m_vertices[0] + 7 );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( T ), (float*)&this->m_vertices[0] + 9 );

		glDrawArrays( GL_TRIANGLES, 0, this->m_vertices.size() );
	}
	else
	{
		// -- We can use the modern Vertex Buffer Object method
		glBindBuffer( GL_ARRAY_BUFFER, this->m_vbo );

		// -- Send the buffer data to the GPU VRAM
		glBufferData(GL_ARRAY_BUFFER, this->m_vertices.size() * sizeof(T), &this->m_vertices[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer( shader->getAttrib( "v_position" ), 4, GL_FLOAT, GL_FALSE, sizeof(T), 0 );
		glVertexAttribPointer( shader->getAttrib( "v_normal" ), 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*) (sizeof(float) * 4) );
		glVertexAttribPointer( shader->getAttrib( "v_texcoord" ), 2, GL_FLOAT, GL_FALSE, sizeof(T), (void*) (sizeof(float) * 7) );
		glVertexAttribPointer( shader->getAttrib( "v_diffuse" ), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(T), (void*) (sizeof(float) * 9) );

		glDrawArrays( GL_TRIANGLES, 0, this->m_vertices.size() );

		// -- Unnecessary and likely causes a slowdown if there are a lot of VertexBuffer instances running
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}
}
