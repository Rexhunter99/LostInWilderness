

#include "Shader.h"
#include "Exceptions.h"

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

#include <GL/glew.h>

using namespace std;


/*********************************************************************/
/*** Shader Implementation */

Shader::Shader()
{
	this->program = glCreateProgram();
}

Shader::~Shader()
{
	glDeleteProgram( this->program );
}

void Shader::bind()
{
	glUseProgram( this->program );
}

void Shader::addProgram( string &text, uint32_t type )
{
	string src_str	= "";
	const char* src = nullptr;
	int len			= 0;
	int status		= 0;
	FILE* fp		= nullptr;
	uint32_t shader = glCreateShader( type );

	// -- Validate that the shader was created
	if ( shader == 0 )
	{
		cerr << "[class:Shader] Failed to create the shader!" << endl;
		return;
	}

	if ( (fp = fopen( text.c_str(), "rb" )) != nullptr )
	{
		fseek( fp, 0, SEEK_END );
		len = ftell( fp );
		fseek( fp, 0, SEEK_SET );

		char* fragShader = new char [len];

		fread( fragShader, len, 1, fp );
		src_str = fragShader;

		fclose( fp );
		delete fragShader;
	}
	else
	{
		cerr << "[class:Shader] Failed to open the shader source \"" << text << "\" for reading!" << endl;
		glDeleteShader( shader );
		return;
	}
	src = src_str.c_str();

	glShaderSource( shader, 1, &src, &len );
	glCompileShader( shader );

	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if ( status == 0 )
	{
		int info_log_len = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &info_log_len );

		char *info_log = new char [ info_log_len ];
		glGetShaderInfoLog( shader, info_log_len, nullptr, info_log );

		cerr << "[class:Shader] Failed to compile shader source:\n" << text << "\n===========================================\n" << info_log << "\n===========================================" << endl;

		delete [] info_log;
		glDeleteShader( shader );

		throw custom_exception( "Failed to compile the shader!" );
		return;
	}

    glAttachShader( this->program, shader );

	// -- Mark this shader for deletion, it will happen when glDeleteProgram occurs, which detaches all shaders
    glDeleteShader( shader );
}

void Shader::compileShaders()
{
	int status = 0;

	glLinkProgram( this->program );
	glGetProgramiv( this->program, GL_LINK_STATUS, &status );

    if ( status == 0 )
	{
		char infolog[1024];
		glGetProgramInfoLog( this->program, 1024, nullptr, infolog );
        cerr << "[OPENGL ERROR] Failed to link shader program\n===========================================\n" << infolog << "\n===========================================" << endl;
        return;
    }
}

void Shader::addVertexShader( string filename )
{
	this->addProgram( filename, GL_VERTEX_SHADER );
}

void Shader::addGeometryShader( string filename )
{
	this->addProgram( filename, GL_GEOMETRY_SHADER );
}

void Shader::addFragmentShader( string filename )
{
	this->addProgram( filename, GL_FRAGMENT_SHADER );
}

void Shader::bindFragData( uint32_t color, std::string name )
{
	glBindFragDataLocation( this->program, color, name.c_str() );
}

void Shader::addUniform( string name )
{
	int uniformLoc = glGetUniformLocation( this->program, name.c_str() );

	if ( uniformLoc == -1 )
	{
		fprintf( stderr, "OpenGL :: Failed to get the Uniform \"%s\" location in the shader program!\n", name.c_str() );
		return;
	}

	this->uniforms.insert( unordered_map<string,int>::value_type( name, uniformLoc ) );
}

void Shader::setUniform1f( string name, float value )
{
	glUniform1f( this->uniforms.find( name )->second, value );
}

void Shader::setUniform4f( string name, float value[4] )
{
	glUniform4f( this->uniforms.find( name )->second, value[0], value[1], value[2], value[3] );
}

void Shader::setUniform1i( string name, int value )
{
	glUniform1i( this->uniforms.find( name )->second, value );
}

void Shader::setUniformMatrix( std::string name, float value[16] )
{
	glUniformMatrix4fv( this->uniforms.find( name)->second, 1, GL_FALSE, value );
}


void Shader::addAttrib( string name )
{
	glGetError();
	int attribLoc = glGetAttribLocation( this->program, name.c_str() );

	if ( attribLoc == -1 )
	{
		fprintf( stderr, "OpenGL :: Failed to get the Attribute \"%s\" location in the shader program!\n", name.c_str() );
		return;
	}

	this->attribs.insert( unordered_map<string,int>::value_type( name, attribLoc ) );
}

uint32_t Shader::getAttrib( std::string name )
{
	return glGetAttribLocation( this->program, name.c_str() );
}

void Shader::setAttrib1f( std::string name, float value )
{
	glVertexAttrib1f( this->attribs.find( name )->second, value );
}

void Shader::setAttrib2f( std::string name, float value[2] )
{
	glVertexAttrib2fv( this->attribs.find( name )->second, value );
}

void Shader::setAttrib3f( std::string name, float value[3] )
{
	glVertexAttrib3fv( this->attribs.find( name )->second, value );
}

void Shader::setAttrib4f( std::string name, float value[4] )
{
	glVertexAttrib4fv( this->attribs.find( name )->second, value );
}

#if defined( _SHADER_UTILS_H__ )
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL
 */
char* file_read(const char* filename)
{
  FILE* in = fopen(filename, "rb");
  if (in == NULL) return NULL;

  int res_size = BUFSIZ;
  char* res = (char*)malloc(res_size);
  int nb_read_total = 0;

  while (!feof(in) && !ferror(in)) {
    if (nb_read_total + BUFSIZ > res_size) {
      if (res_size > 10*1024*1024) break;
      res_size = res_size * 2;
      res = (char*)realloc(res, res_size);
    }
    char* p_res = res + nb_read_total;
    nb_read_total += fread(p_res, 1, BUFSIZ, in);
  }

  fclose(in);
  res = (char*)realloc(res, nb_read_total + 1);
  res[nb_read_total] = '\0';
  return res;
}

/**
 * Display compilation errors from the OpenGL shader compiler
 */
void print_log(GLuint object)
{
  GLint log_length = 0;
  if (glIsShader(object))
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    fprintf(stderr, "printlog: Not a shader or a program\n");
    return;
  }

  char* log = (char*)malloc(log_length);

  if (glIsShader(object))
    glGetShaderInfoLog(object, log_length, NULL, log);
  else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);

  fprintf(stderr, "%s", log);
  free(log);
}

/**
 * Compile the shader from file 'filename', with error handling
 */
GLuint create_shader(const char* filename, GLenum type)
{
  const GLchar* source = file_read(filename);
  if (source == NULL) {
    fprintf(stderr, "Error opening %s: ", filename); perror("");
    return 0;
  }
  GLuint res = glCreateShader(type);
  const GLchar* sources[] = {
    // Define GLSL version
/*#ifdef GL_ES_VERSION_2_0
    "#version 100\n"
#else
    "#version 120\n"
#endif
    ,
    // GLES2 precision specifiers
#ifdef GL_ES_VERSION_2_0
    // Define default float precision for fragment shaders:
    (type == GL_FRAGMENT_SHADER) ?
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "precision highp float;           \n"
    "#else                            \n"
    "precision mediump float;         \n"
    "#endif                           \n"
    : ""
    // Note: OpenGL ES automatically defines this:
    // #define GL_ES
#else
    // Ignore GLES 2 precision specifiers:
    "#define lowp   \n"
    "#define mediump\n"
    "#define highp  \n"
#endif
	""
    ,*/
    source };
  glShaderSource(res, 1, sources, NULL);
  free((void*)source);

  glCompileShader(res);
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "%s:", filename);
    print_log(res);
    glDeleteShader(res);
    return 0;
  }

  return res;
}

GLuint create_program(const char *vertexfile, const char *fragmentfile) {
	GLuint program = glCreateProgram();
	GLuint shader;

	if(vertexfile) {
		shader = create_shader(vertexfile, GL_VERTEX_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);
	}

	if(fragmentfile) {
		shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		print_log(program);
		glDeleteProgram(program);
		return 0;
	}

	return program;
}

#ifdef GL_GEOMETRY_SHADER
GLuint create_gs_program(const char *vertexfile, const char *geometryfile, const char *fragmentfile, GLint input, GLint output, GLint vertices) {
	GLuint program = glCreateProgram();
	GLuint shader;

	if(vertexfile) {
		shader = create_shader(vertexfile, GL_VERTEX_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);
	}

	if(geometryfile) {
		shader = create_shader(geometryfile, GL_GEOMETRY_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);

		glProgramParameteriEXT(program, GL_GEOMETRY_INPUT_TYPE_EXT, input);
		glProgramParameteriEXT(program, GL_GEOMETRY_OUTPUT_TYPE_EXT, output);
		glProgramParameteriEXT(program, GL_GEOMETRY_VERTICES_OUT_EXT, vertices);
	}

	if(fragmentfile) {
		shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		print_log(program);
		glDeleteProgram(program);
		return 0;
	}

	return program;
}
#else
GLuint create_gs_program(const char *vertexfile, const char *geometryfile, const char *fragmentfile, GLint input, GLint output, GLint vertices) {
	fprintf(stderr, "Missing support for geometry shaders.\n");
	return 0;
}
#endif

GLint get_attrib(GLuint program, const char *name) {
	GLint attribute = glGetAttribLocation(program, name);
	if(attribute == -1)
		fprintf(stderr, "Could not bind attribute %s\n", name);
	return attribute;
}

GLint get_uniform(GLuint program, const char *name) {
	GLint uniform = glGetUniformLocation(program, name);
	if(uniform == -1)
		fprintf(stderr, "Could not bind uniform %s\n", name);
	return uniform;
}
#endif
