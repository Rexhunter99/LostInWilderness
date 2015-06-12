#pragma once

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <stdint.h>
#include <string>
#include <vector>

class Framebuffer
{
private:
	uint32_t				m_fbo;
	std::vector<uint32_t>	m_buffers;

public:

	Framebuffer();
	~Framebuffer();

	void bindRenderToTarget() const;
	void bindColorTexture( size_t index ) const;
};

#endif // __FRAMEBUFFER_H__
