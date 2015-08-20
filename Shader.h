#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>

/** @class Shader
 ** @brief Container for managing shader programs
 ** @todo Finish implementing class Shader
 **
 ** Container for managing OpenGL shader programs, simplifies the usage and management of the internal
 ** handles and data as well as error handling, abstracting away some of the more complex methods. */
class Shader
{
protected:

	uint32_t								program;
	std::unordered_map<std::string, int>	uniforms;
	std::unordered_map<std::string, int>	attribs;

	void addProgram( std::string &text, uint32_t type );

public:

	Shader();
	~Shader();

	/** @fn bind
	 ** @brief Set this shader as the current program for rendering with */
	void bind();

	/** @fn addVertexShader
	 ** @param filename A string that points to the file */
	void addVertexShader( std::string filename );

	/** @fn addGeometryShader
	 ** @param filename A string that points to the file */
	void addGeometryShader( std::string filename );

	/** @fn addFragmentShader
	 ** @param filename A string that points to the file */
	void addFragmentShader( std::string filename );

	/** @fn compileShaders
	 **/
	void compileShaders();

	/** @fn bindFragData
	 **/
	void bindFragData( uint32_t color, std::string name );

	/** @fn addUniform
	 ** @param name The name of the uniform to collect in the shader for input */
	void addUniform( std::string name );

	/** @fn setUniform1i
	 ** @param name The name of the uniform to set the value of
	 ** @param value The value to set the uniform to */
	void setUniform1i( std::string name, int value );

	/** @fn setUniform1f
	 ** @param name The name of the uniform to set the value of
	 ** @param value The value to set the uniform to */
	void setUniform1f( std::string name, float value );

	/** @fn setUniform2f
	 ** @param name The name of the uniform to set the value of
	 ** @param value The value to set the uniform to */
	void setUniform2f( std::string name, float value[2] );

	/** @fn setUniform3f
	 ** @param name The name of the uniform to set the value of
	 ** @param value The value to set the uniform to */
	void setUniform3f( std::string name, float value[3] );

	/** @fn setUniform4f
	 ** @param name The name of the uniform to set the value of
	 ** @param value The value to set the uniform to */
	void setUniform4f( std::string name, float value[4] );

	/** @fn setUniformMatrix
	 ** @param name The name of the uniform to set the value of
	 ** @param value The value to set the uniform to */
	void setUniformMatrix( std::string name, float value[16] );

	/** @fn addAttrib
	 ** @param name The name of the vertex attribute to collect in the shader for input */
	void addAttrib( std::string name );

	/** @fn getAttrib
	 ** @param name The name of the vertex attribute to collect in the shader for input
	 ** @return The location of the attribute */
	uint32_t getAttrib( std::string name );

	/** @fn setAttrib1i
	 ** @param name The name of the attribute to set the value of
	 ** @param value The value to set the attribute to */
	void setAttrib1i( std::string name, int value );

	/** @fn setAttrib1f
	 ** @param name The name of the attribute to set the value of
	 ** @param value The value to set the attribute to */
	void setAttrib1f( std::string name, float value );

	/** @fn setAttrib2f
	 ** @param name The name of the attribute to set the value of
	 ** @param value The value to set the attribute to */
	void setAttrib2f( std::string name, float value[2] );

	/** @fn setAttrib3f
	 ** @param name The name of the attribute to set the value of
	 ** @param value The value to set the attribute to */
	void setAttrib3f( std::string name, float value[3] );

	/** @fn setAttrib4f
	 ** @param name The name of the attribute to set the value of
	 ** @param value The value to set the attribute to */
	void setAttrib4f( std::string name, float value[4] );

	void setDefaults();
};

