
#include "VertexFormat.h"

#include <GL/glew.h>


VertexFormat::VertexFormat()
{
	glGenVertexArrays( 1, &this->vao );
}

VertexFormat::~VertexFormat()
{
	glDeleteVertexArrays( 1, &this->vao );
}

void VertexFormat::bind()
{
	glBindVertexArray( this->vao );
}

void VertexFormat::unbind()
{
	glBindVertexArray( 0 );
}

void VertexFormat::enableVertexAttrib( uint32_t attrib_location )
{
	glEnableVertexAttribArray( attrib_location );
}
