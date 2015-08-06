#pragma once

#include "Shader.h"
#include "Texture.h"
#include "TextFont.h"
#include "Vertex.h"
#include "VertexFormat.h"

#include <glm/fwd.hpp>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Renderer
{
private:
	std::mutex		m_mtx;


public:

	static Renderer *iRenderer;

	static Shader *default_shader;
	static Texture *blocks_texture;
	static Texture *items_texture;
	static TextFont *font_texture;

	Renderer( std::string version );
	~Renderer();

	static void *	getWindow( std::string name );
	static void		addWindow( std::string name, void *window );

	// -- Information
	static const char * getDriverVendor();
	static const char * getShaderVersion();


	// -- Lighting
	void initLights();
	uint32_t  addPointLight( );
	uint32_t  addDirectionalLight( );
	uint32_t  addSpotLight( );
	void setDirectionalLight( uint32_t light_index, glm::vec3 position, glm::vec4 diffuse, glm::vec4 ambient, float specular_strength );
	void setPointLight( uint32_t light_index, glm::vec3 position, glm::vec4 diffuse, glm::vec4 ambient, float specular_strength );
	void setSpotLight( uint32_t light_index, glm::vec3 position, glm::vec3 at_angle, float cone_angle, glm::vec4 diffuse, glm::vec4 ambient, float specular_strength );

	/** @fn updateLights();
	 ** @brief Update all the lights in the scene to the GPU.
	 ** This method will update all of the scene lights, processing any renderer controlled aspects before
	 ** uploading them to the current shader. For each shader used in a scene, the lights should be updated.
	 **/
	void updateLights();

	std::mutex & getMutex() { return this->m_mtx; }
};
