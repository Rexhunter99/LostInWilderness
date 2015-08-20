
#include "Texture.h"

#include <GL/glew.h>

#include <fstream>
#include <iostream>

Texture::Texture()
{
	this->width		= 0;
	this->height	= 0;
	this->iformat	= GL_RGBA8;
	this->format	= GL_RGBA;
	glGenTextures( 1, &texid );
}


Texture::Texture( std::string texture_file, uint32_t width, uint32_t height )
{
	this->width		= width;
	this->height	= height;
	this->iformat	= GL_RGBA8;
	this->format	= GL_RGBA;
	glGenTextures( 1, &texid );
	this->loadFile( texture_file, width, height );
}

Texture::~Texture()
{
	glDeleteTextures( 1, &texid );
}

bool Texture::loadFile( std::string p_texturefile, uint32_t index_width, uint32_t index_height )
{
	// -- Header for testing if it is a png
	png_byte header[8];

	// -- Create a C++ string of the filename/directory
	std::string p_filename = p_texturefile;

	// Create a filestream object for loading PNG files
	std::fstream file;

	//open file as binary
	file.open( p_filename, std::fstream::in | std::fstream::binary);
	if (file.fail())
	{
        std::cerr << "Error opening picture file\n%s\n" << p_filename << std::endl;
		return false;
	}

	//read the header
	file.read( (char*)header, 8 );

	//test if png
	if ( !(!png_sig_cmp(header, 0, 8)) )
	{
		file.close();
		std::cerr << "LoadPicturePNG()\nThe specified file is not a valid PNG.\n" << std::endl;
		return false;
	}

	//create png struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
	{
		file.close();
		return false;
	}

	// Set PNG read function to our custom read function
	png_set_read_fn(png_ptr,(png_voidp)&file, customReadPNG);

	//create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		file.close();
		return false;
	}

	//create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		file.close();
		return false;
	}

	//png error stuff, not sure libpng man suggests this.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		file.close();
		return false;
	}

	//init png reading
	//png_init_io(png_ptr, file);

	//let libpng know you already read the first 8 bytes
	png_set_sig_bytes(png_ptr, 8);

	// read all the info up to the image data
	png_read_info(png_ptr, info_ptr);

	//variables to pass to get info
	int			bit_depth,
				color_type;
	png_uint_32	twidth,
				theight;

	//int channels = png_get_channels( png_ptr, info_ptr );

	// -- Get info about png
	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, nullptr, nullptr, nullptr);

	// -- Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);

	// -- Row size in bytes.
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// -- Allocate the image_data as a big block, to be given to opengl
	png_byte *image_data = (png_byte*)malloc( rowbytes * theight );
	if (!image_data)
	{
		// -- Clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		file.close();
		std::cerr << "loadTexture() :: Failed to allocate the image data for the PNG.\n" << std::endl;
		return false;
	}

	// -- Row_pointers is for pointing to image_data for reading the png with libpng
	png_bytep *row_pointers = new png_bytep[ theight ];
	if (!row_pointers)
	{
		// -- Clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free( image_data );
		file.close();
		std::cerr << "loadTexture() :: Failed to allocate the row pointers for the PNG.\n" << std::endl;
		return false;
	}

	// -- Set the individual row_pointers to point at the correct offsets of image_data
	for (unsigned int i = 0; i < theight; ++i)
	{
		// -- Row_pointers[pic.m_height - 1 - i] = image_data + i * rowbytes;
		row_pointers[i] = image_data + i * rowbytes;
	}

	// -- Read the png into image_data through row_pointers
	png_read_image(png_ptr, row_pointers);

	// -- Apply the texture as current
	this->width = twidth;
	this->height = theight;
	glBindTexture( GL_TEXTURE_2D, this->texid );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// -- Mipmaps
	if ( !GLEW_VERSION_3_0 )
			glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

	// -- Upload the raster data to the graphics hardware
	if ( bit_depth == 8 )
	{
		if (color_type == PNG_COLOR_TYPE_RGB )//&& channels == 3 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data );
		}
		else if (color_type == PNG_COLOR_TYPE_RGBA )// && channels == 4)
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data );
		}
		else std::cerr << "loadTexture() :: glTexImage2D 8-bit format is Unknown\n" << std::endl;
	}
	else if ( bit_depth == 16 )
	{
		if (color_type == PNG_COLOR_TYPE_RGB )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16, twidth, theight, 0, GL_RGB, GL_UNSIGNED_SHORT, image_data );
		}
		else if (color_type == PNG_COLOR_TYPE_RGBA )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_SHORT, image_data );
		}
		else std::cerr << "loadTexture() :: glTexImage2D 16-bit format is Unknown\n" << std::endl;
	}

	if ( GLEW_VERSION_3_0 )
		glGenerateMipmap( GL_TEXTURE_2D );


	//-- Clean up memory and close stuff
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	delete[] row_pointers;
	free( image_data );
	file.close();

	this->index_width = index_width;
	this->index_height = index_height;

    return false;
}

void Texture::bind() const
{
	glBindTexture( GL_TEXTURE_2D, this->texid );
}

void Texture::customReadPNG(png_structp png_ptr, png_bytep data, png_size_t length)
{
    // Get IO pointer from the PNG struct so we can use with fstream
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    // Cast io ptr to fstream and read it with std::fstream::read
    ( (std::fstream*) io_ptr )->read( (char*) data, length );
}

float Texture::getAtlasU( int index_x, int index_y )
{
	if ( index_x == 0 )
		return 0.0f;
	return float( index_x ) / float( this->width / this->index_width );
}

float Texture::getAtlasV( int index_x, int index_y )
{
	if ( index_y == 0 )
		return 1.0f;
	return 1.0f - (float( index_y ) / float( this->height / this->index_height ));
}
