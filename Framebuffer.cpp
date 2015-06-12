
#include "Framebuffer.h"

#include <gl/glew.h>


Framebuffer::Framebuffer()
{
	glGenFramebuffers( 1, &this->m_fbo );
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers( 1, &this->m_fbo );
}
