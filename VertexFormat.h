#pragma once

#if !defined( __VERTEXFORMAT_H__ )
#define __VERTEXFORMAT_H__

#include <cstdint>


/** @class VertexFormat
 ** @version 1
 ** @author Rexhunter99
 **
 ** A class that handles the vertex format for the end-user
 **/
class VertexFormat
{
private:
	uint32_t vao;

public:

	VertexFormat();
	~VertexFormat();

	void bind();
	static void unbind();

	void enableVertexAttrib( uint32_t attrib_location );
};

#endif
