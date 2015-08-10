#pragma once

#include <string>
#include <glm/fwd.hpp>
#include <png.h>

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
	Texture( std::string filename, uint32_t width, uint32_t height );
	~Texture();

	bool		loadFile( std::string p_texturefile, uint32_t index_width = 0, uint32_t index_height = 0 );
	void		bind() const;

    /** @brief Custom read function to allow datatypes other than FILE*
     **
     ** @param png_ptr Pointer to the PNG struct for the file
     ** @param data Data block the source file data is allocated to
     ** @param length Size of the data block
     ** @author TambourineReindeer
     **
     ** This function allows user-defined data types to be read in from fstream or istream, eliminating the need for C types.
     ** Adapted from tutorial by Wracky: http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/
     **/
	static void customReadPNG(png_structp png_ptr, png_bytep data, png_size_t length);

	float		getAtlasU( int index_x, int index_y );
	float		getAtlasV( int index_x, int index_y );

	uint32_t	getWidth() const	{ return this->width; }
	uint32_t	getHeight() const	{ return this->height; }
	uint32_t	getID() const		{ return this->texid; }
	uint32_t	getFormat() const	{ return this->format; }
	uint32_t	getInternalFormat() const { return this->iformat; }

};
