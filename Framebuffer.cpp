
#include "Framebuffer.h"

#include <GL/glew.h>


Framebuffer::Framebuffer()
{
	glGenFramebuffers( 1, &this->m_fbo );
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers( 1, &this->m_fbo );
}

uint32_t createRenderBuffer()
{
	uint32_t rbo;
	glGenRenderbuffers( 1, &rbo );
	//glRenderbufferStorage();
	return rbo;
}
