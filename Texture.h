#pragma once

#include <string>
#include <glm/fwd.hpp>

class Texture
{
protected:

	unsigned int 	width,
					height,
					index_width,
					index_height;
	unsigned int 	iformat,
					format;
	unsigned int	texid;

public:

	Texture();
	Texture( std::string filename );
	~Texture();

	bool		loadFile( std::string p_texturefile, uint32_t index_width = 0, uint32_t index_height = 0 );
	void		bind() const;

	float		getAtlasU( int index_x, int index_y );
	float		getAtlasV( int index_x, int index_y );

	uint32_t	getWidth() const	{ return this->width; }
	uint32_t	getHeight() const	{ return this->height; }
	uint32_t	getID() const		{ return this->texid; }
	uint32_t	getFormat() const	{ return this->format; }
	uint32_t	getInternalFormat() const { return this->iformat; }

};
