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

	enum APIVersionEnum {
		AV_OPENGL_20,
		AV_OPENGL_21,
		AV_OPENGL_30,
		AV_OPENGL_31,
		AV_OPENGL_32,
		AV_OPENGL_33
		// TODO: Perhaps OpenGL 4.x?
	} api_version;

	Renderer( APIVersionEnum version );
	Renderer( std::string version );
	~Renderer();

	static void *	getWindow( std::string name );
	static void		addWindow( std::string name, void *window );

	void initLights();

};
