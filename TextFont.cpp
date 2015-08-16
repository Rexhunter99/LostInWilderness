
#include "TextFont.h"
#include "Exceptions.h"
#include "Renderer.h"

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace std;

TextFont* TextFont::active;


TextFont::TextFont( string p_filename ) : Texture( )
{
	this->loadFont( p_filename );
	glGenBuffers( 1, &this->m_vbo );
}

TextFont::~TextFont()
{
	glDeleteBuffers( 1, &this->m_vbo );
}

bool TextFont::loadFont( string p_filename )
{
	size_t		glyphs_loaded = 0;
	FILE		*fp = nullptr;

	// -- Create a C++ string of the filename/directory
	string p_fontfilename = string( p_filename + string( ".fnt" ) );

	// -- Read the properties file
	if ( (fp = fopen( p_fontfilename.c_str(), "r" )) != nullptr )
	{

		while ( !feof(fp) )
		{
			char line[128];

			fgets( line, 128, fp );

			char* tok = strtok( line, " \t\n" );

			if ( tok == nullptr ) continue;

			if ( !strcmp( tok, "width" ) )
			{
				this->m_char_width = atoi( strtok( nullptr, " \t\n" ) );
			}
			else if ( !strcmp( tok, "height" ) )
			{
				this->m_char_height = atoi( strtok( nullptr, " \t\n" ) );
			}
			else if ( !strcmp( tok, "spacing" ) )
			{
				this->m_spacing = atoi( strtok( nullptr, " \t\n" ) );
			}
			else if ( !strcmp( tok, "char" ) )
			{
                tok = strtok( nullptr, " \t\n" );

                if ( tok == nullptr ) continue;

				uint8_t g = (uint8_t)tok[0];

                this->glyphs[ g ].left		= atoi( strtok( nullptr, " \t\n" ) );
                this->glyphs[ g ].top		= atoi( strtok( nullptr, " \t\n" ) );
                this->glyphs[ g ].right		= atoi( strtok( nullptr, " \t\n" ) );
                this->glyphs[ g ].bottom	= atoi( strtok( nullptr, " \t\n" ) );

                this->glyphs[ g ].width		= atoi( strtok( nullptr, " \t\n" ) );
                this->glyphs[ g ].height	= atoi( strtok( nullptr, " \t\n" ) );

                glyphs_loaded++;
			}
		}

		fclose( fp );

		this->loadFile( p_filename + string(".png"), this->m_char_width, this->m_char_height );
	}
	else
	{
		string err_str = string("OpenGL | ERROR | Failed to open font definition file \"") + p_filename + string("\"");
		throw custom_exception( err_str );
		return false;
	}
	return true;
}

bool TextFont::bind()
{
	Texture::bind();
	return true;
}

uint32_t TextFont::getTextWidth( std::wstring text )
{
	uint32_t accum_w = 0;

	for ( size_t i=0; i<text.length(); i++ )
	{
		accum_w += this->glyphs[ text.at(i) ].width;
	}

	return accum_w;
}

uint32_t TextFont::getTextHeight( std::wstring text )
{
	uint32_t accum_h = 0;

	for ( size_t i=0; i<text.length(); i++ )
	{
		uint32_t glyph_h = this->glyphs[ text.at(i) ].height;
		if ( accum_h < glyph_h )
		accum_h = glyph_h;
	}

	return accum_h;
}

void TextFont::drawString( int x, int y, uint32_t color, float scale, std::wstring text )
{
	// -- Use a single std::vector here to avoid overhead.
	std::vector< Vertex >		vertices;
	float		gui_scale = scale;
	uint32_t	cx = x,
				cy = y;
	uint32_t	count = 0;

    for ( size_t i=0; i<text.length(); i++ )
    {
		uint8_t c = text.at( i );
		struct TextFont::glyph_s* g = &this->glyphs[c];

		if ( c == '\n' )
		{
			cx = x;
			cy += this->m_char_height;
			continue;
		}

		if ( c == '\t' )
		{
			cx += this->m_char_width * 4;
			continue;
		}

		if ( c == ' ' )
		{
			cx += this->m_char_width;
			continue;
		}

		float fl = float(g->left) / this->width;
		float ft = float(g->top) / this->height;
		float fr = float(g->right) / this->width;
		float fb = float(g->bottom) / this->height;

		vertices.push_back( Vertex( cx * gui_scale, cy * gui_scale, 0, 100, fl, ft ) );
		vertices.push_back( Vertex( (cx + g->width) * gui_scale, cy * gui_scale, 0, 100, fr, ft ) );
		vertices.push_back( Vertex( (cx + g->width) * gui_scale, (cy + g->height) * gui_scale, 0, 100, fr, fb ) );

		vertices.push_back( Vertex( cx * gui_scale, cy * gui_scale, 0, 100, fl, ft ) );
		vertices.push_back( Vertex( (cx + g->width) * gui_scale, (cy + g->height) * gui_scale, 0, 100, fr, fb ) );
		vertices.push_back( Vertex( cx * gui_scale, (cy + g->height) * gui_scale, 0, 100, fl, fb ) );

		cx += g->width + 1;
		count += 6;
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo );
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer( Renderer::default_shader->getAttrib( "v_position" ), 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0 );
	glVertexAttribPointer( Renderer::default_shader->getAttrib( "v_normal" ), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 4) );
	glVertexAttribPointer( Renderer::default_shader->getAttrib( "v_texcoord" ), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 7) );
	glVertexAttribPointer( Renderer::default_shader->getAttrib( "v_diffuse" ), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 9) );
	glDrawArrays( GL_TRIANGLES, 0, count );

	//glTexCoordPointer( 2, GL_FLOAT, sizeof(float)*4, &(vertices[1]) );
	//glVertexPointer( 2, GL_FLOAT, sizeof(float)*4, &(vertices[0]) );
	//glDrawArrays( GL_QUADS, 0, count );
}
