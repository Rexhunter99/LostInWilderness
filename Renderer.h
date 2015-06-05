#pragma once

#include "Shader.h"
#include "Texture.h"
#include "TextFont.h"
#include "Vertex.h"

#include <glm/fwd.hpp>
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
	static const char * getDriverVendor() const;
	static const char * getShaderVersion()

	void initLights();

	void setDirectionalLight( glm::vec3 position, glm::vec4 diffuse, glm::vec4 ambient, float specular_strength );

	uint32_t  addPointLight( .. );

	/** @fn updateLights();
	 ** @brief Update all the lights in the scene to the GPU.
	 ** This method will update all of the scene lights, processing any renderer controlled aspects before
	 ** uploading them to the current shader. For each shader used in a scene, the lights should be updated.
	 **/
	void updateLights();

};
