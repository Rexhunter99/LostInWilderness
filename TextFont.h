
#ifndef __TEXTFONT_H__
#define __TEXTFONT_H__
#pragma once

#include "Texture.h"

#include "vectors.h"
#include <stdint.h>
#include <string>
#include <map>



class TextFont : public Texture
{
public:

	// -- Define these in VideoGL.cpp
    TextFont() : Texture() {};
    TextFont( std::string p_filename );
    ~TextFont();

    // -- Get dimensions of text
    uint32_t	getTextWidth( std::wstring text );
    uint32_t	getTextHeight( std::wstring text );

    bool		bind();
	bool		loadFont( std::string p_filename );

    void		drawString( int x, int y, uint32_t color, float scale, std::wstring text );

    static TextFont* active;

protected:

	std::string	m_name;			// Name of the font (take it off the descriptor)
	uint32_t	m_vbo;
	uint8_t		m_char_width,
				m_char_height,
				m_spacing;

    struct glyph_s // 10-bytes
    {
		uint16_t	left, right, top, bottom;	// Coordinates in pixels, [0-65536] (2^16)
		uint8_t		width, height;				// Width/Height of the glyph in pixels (px)
    } glyphs[256];

	//std::map<v2u8,struct glyph_s>	glyphs;

private:

};

#endif // __TEXTFONT_H__

