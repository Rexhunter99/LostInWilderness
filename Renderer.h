#pragma once

#include "Shader.h"
#include "Texture.h"
#include "TextFont.h"
#include "Vertex.h"

#include <string>
#include <unordered_map>

class Renderer
{
public:

	static Renderer *iRenderer;

	static Shader *default_shader;
	static Texture *blocks_texture;
	static Texture *items_texture;
	static TextFont *font_texture;

	Renderer();
	~Renderer();

	static void *	getWindow( std::string name );
	static void		addWindow( std::string name, void *window );

	void initLights();

};
