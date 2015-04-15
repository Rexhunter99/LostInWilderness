
#include "Renderer.h"
#include "Exceptions.h"
#include "LostInWilderness.h"

#include <iostream>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>


#define DELETE_IF_NOT_NULL( A ) if ( A != nullptr ) \
								delete A;

GLFWwindow *g_window, *g_updatecontext;

std::map<std::string, GLFWwindow*>	g_windows;

Renderer *Renderer::iRenderer;
Texture *Renderer::blocks_texture;
Texture *Renderer::items_texture;
TextFont *Renderer::font_texture;
Shader *Renderer::default_shader;


void APIENTRY cbOpenGLError( GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length, const char* _message, void* _userParam)
{
	if ( strstr(_message, "Clear color unsupported") ) return;

	if ( _type != GL_DEBUG_TYPE_ERROR_ARB ) return;

#ifdef _DEBUG
	if ( _severity == GL_DEBUG_SEVERITY_LOW_ARB ) return;
#endif // NDEBUG

	std::cerr << "[OPENGL ERROR] " << " | " << _message << std::endl;

	return;
}


Renderer::Renderer()
{
	Renderer::blocks_texture = nullptr;
	Renderer::items_texture = nullptr;
	Renderer::default_shader = nullptr;
	Renderer::font_texture = nullptr;

	// -- Initialise and status check GLFW 3
	if ( glfwInit() == GL_FALSE )
	{
		throw custom_exception( "GLFW 3 failed to initialize." );
		return;
	}

	// -- Initialise the monitor pointer to no monitor
	GLFWmonitor *monitor = nullptr;

	#if defined(_FULLSCREEN)
	monitor = glfwGetPrimaryMonitor();
	#endif // defined

	// -- MSAA Samples (default to 0 for now...)
	glfwWindowHint( GLFW_SAMPLES, 0 );

	#if defined( _PORTABLE )
	// -- We're using OpenGL on portables
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
	#else
	// -- We're using OpenGL on desktops/laptops
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );
	#endif

	// -- Default our desired context version to 2.0
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );

	// -- Core profile context
	//glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	#ifdef _DEBUG
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
	#endif // _DEBUG

	g_window = glfwCreateWindow( 1280, 720, "GaiaCraft", monitor, nullptr );
	if ( !g_window )
	{
		glfwTerminate();
		throw custom_exception( "GLFW 3 failed to create the window and context." );
		return;
	}

	glfwMakeContextCurrent( g_window );
	Renderer::addWindow( "g_window", g_window );

	// -- Create the multithreading context
	glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
	g_updatecontext = glfwCreateWindow(1, 1, "Multithread Context", NULL, g_window );
	Renderer::addWindow( "g_updatecontext", g_updatecontext );

	// -- Initialise and check the status of the GL Extension Wrangler library
	glewExperimental = GL_TRUE;
	GLenum glew_status = glewInit();
	if ( glew_status != GLEW_OK )
	{
		std::cerr << "[GLEW ERROR] " << glewGetErrorString(glew_status) << std::endl;
		glfwTerminate();
		throw custom_exception( std::string("GLEW was unable to initialize! Error message:\n") + std::string((const char*)glewGetErrorString(glew_status)) );
		return;
	}

	// -- Check the minimum version of OpenGL is actually 2.0 or greater
	if (!GLEW_VERSION_2_0)
	{
		glfwTerminate();
		throw custom_exception( "Video drivers do not support OpenGL 2.0+" );
		return;
	}

	#ifdef _DEBUG
	if ( glfwExtensionSupported( "GL_ARB_debug_output" ) )
	{
		glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)glfwGetProcAddress( "glDebugMessageCallbackARB" );
		glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)glfwGetProcAddress( "glDebugMessageControlARB" );

		if ( glDebugMessageCallbackARB ) glDebugMessageCallbackARB( cbOpenGLError, nullptr );
//		if ( glDebugMessageControlARB ) glDebugMessageControlARB( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE );
//		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
	}
	#endif //_DEBUG

	GLint max_array_texture_layers = 0;
	glGetIntegerv( GL_MAX_ARRAY_TEXTURE_LAYERS, &max_array_texture_layers );
	std::cout << "MAX ARRAY TEXTURE LAYERS: " << max_array_texture_layers << std::endl;

	//glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)glfwGetProcAddress( "glGenerateMipmap" );
	//glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glfwGetProcAddress( "glGenVertexArrays" );

	// -- Handle state changes here that are 'static' or that don't change every frame
	glEnable( GL_MULTISAMPLE );
	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND );

	glActiveTexture( GL_TEXTURE0 );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// -- Enable Swap Control Tear over Swap Interval (control tear does not vsync if fps is behind)
	if ( GaiaCraft::iGaiaCraft->config->getInteger( "renderer.vertical_sync" ) > 0 )
	{
		if ( glfwExtensionSupported( "GLX_EXT_swap_control" ) || glfwExtensionSupported( "WGL_EXT_swap_control" ) )
		{
			if ( glfwExtensionSupported( "GLX_EXT_swap_control_tear" ) || glfwExtensionSupported( "WGL_EXT_swap_control_tear" ) )
				glfwSwapInterval( -1 );
			else
				glfwSwapInterval( 1 );
		}
	}
}

Renderer::~Renderer()
{
	DELETE_IF_NOT_NULL(Renderer::blocks_texture);
	DELETE_IF_NOT_NULL(Renderer::items_texture);
	DELETE_IF_NOT_NULL(Renderer::default_shader);

	glfwTerminate();
}


void Renderer::addWindow( std::string name, void* window )
{
	g_windows.insert( std::map<std::string,GLFWwindow*>::value_type( name, (GLFWwindow*)window ) );
}

void * Renderer::getWindow( std::string name )
{
	std::map<std::string,GLFWwindow*>::iterator it = g_windows.find( name );

	if ( it == g_windows.end() )
		return nullptr;
	else
		return it->second;
}
