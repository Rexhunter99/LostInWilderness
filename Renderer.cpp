
#include "Renderer.h"
#include "Config.h"
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

	//if ( _type != GL_DEBUG_TYPE_ERROR_ARB ) return;

#ifdef _DEBUG
	if ( _severity == GL_DEBUG_SEVERITY_LOW_ARB ) {} //return;
	else
	{
		std::cerr << "[OPENGL ERROR] " << " | " << _message << std::endl;
	}
#else

	std::cerr << "[OPENGL DEBUG ERROR] " << " | " << _message << std::endl;

#endif // NDEBUG

	return;
}


Renderer::Renderer( std::string version )
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

	int glfw_hints[5][4] = {
		#if defined( _PORTABLE )
		{
			GLFW_OPENGL_ES_API,
			3, 1,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_ES_API,
			3, 0,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_ES_API,
			2, 0,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_ES_API,
			1, 1,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_ES_API,
			1, 0,
			GLFW_OPENGL_ANY_PROFILE,
		}
		#else
		{
			GLFW_OPENGL_API,
			3, 2,
			GLFW_OPENGL_CORE_PROFILE,
		},
		{
			GLFW_OPENGL_API,
			3, 1,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_API,
			3, 0,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_API,
			2, 1,
			GLFW_OPENGL_ANY_PROFILE,
		},
		{
			GLFW_OPENGL_API,
			2, 0,
			GLFW_OPENGL_ANY_PROFILE,
		}
		#endif
	};
	const int glfw_hints_num = 5;

	// -- Initialise the monitor pointer to no monitor
	GLFWmonitor *monitor = nullptr;

	#if defined(_FULLSCREEN)
	monitor = glfwGetPrimaryMonitor();
	#endif // defined

	bool msaa = false;

	for ( int h=0; h<glfw_hints_num; h++ )
	{
		glfwDefaultWindowHints();

		glfwWindowHint( GLFW_CLIENT_API, glfw_hints[h][0] );

		// -- Hint at our desired context version
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, glfw_hints[h][1] );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, glfw_hints[h][2] );

		// -- Core profile context
		glfwWindowHint( GLFW_OPENGL_PROFILE, glfw_hints[h][3] );

		#ifdef _DEBUG
		glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
		#endif // _DEBUG

		// -- MSAA Samples (default to 0 for now...)
		msaa = false;

		if ( Config::getGlobal()->getString( "renderer.antialiasing" ).find("msaa") != std::string::npos )
		{
			std::string aa = Config::getGlobal()->getString( "renderer.antialiasing" );
			msaa = true;
			if ( aa.find("2x") != std::string::npos )
				glfwWindowHint( GLFW_SAMPLES, 2 );
			else if ( aa.find("4x") != std::string::npos )
				glfwWindowHint( GLFW_SAMPLES, 4 );
			else if ( aa.find("8x") != std::string::npos )
				glfwWindowHint( GLFW_SAMPLES, 8 );
		}
		else
		{
			glfwWindowHint( GLFW_SAMPLES, 0 );
		}

		g_window = glfwCreateWindow( 1280, 720, "Lost In Wilderness - Alpha", monitor, nullptr );

		if ( g_window )
		{
			// -- We found a working set of hints!
			break;
		}
	}

	if ( !g_window )
	{
		glfwTerminate();
		throw custom_exception( "GLFW 3 failed to create a window/context that matched any of the desired hints." );
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

	// -- Version output
	#ifdef _DEBUG
	int gl_version[2];
	glGetIntegerv( GL_MAJOR_VERSION, &gl_version[0] );
	glGetIntegerv( GL_MINOR_VERSION, &gl_version[1] );
	std::cout << "GL_VERSION: " << glGetString( GL_VERSION ) << std::endl;
	std::cout << "GL_VERSION: " << gl_version[0] << "." << gl_version[1] << std::endl;
	int glsl_version_num = 0;
	glGetIntegerv( GL_NUM_SHADING_LANGUAGE_VERSIONS, &glsl_version_num );

	for ( int i=0; i<glsl_version_num; i++ )
	{
		std::cout << "GLSL VERSION: " << glGetStringi( GL_SHADING_LANGUAGE_VERSION, i ) << std::endl;
	}
	#endif // _DEBUG

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
	if ( msaa == true )
	{
		glEnable( GL_MULTISAMPLE );
	}

	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND );

	glActiveTexture( GL_TEXTURE0 );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// -- Enable Swap Control Tear over Swap Interval (control tear does not vsync if fps is behind)
	if ( Config::getGlobal()->getInteger( "renderer.vertical_sync" ) > 0 )
	{
		if ( glfwExtensionSupported( "GLX_EXT_swap_control" ) || glfwExtensionSupported( "WGL_EXT_swap_control" ) )
		{
			if ( glfwExtensionSupported( "GLX_EXT_swap_control_tear" ) || glfwExtensionSupported( "WGL_EXT_swap_control_tear" ) )
			{
				glfwSwapInterval( -1 );
				std::cout << "Adaptive Vertical Sync: Enabled" << std::endl;
			}
			else
			{
				glfwSwapInterval( 1 );
				std::cout << "1:1 Vertical Sync: Enabled" << std::endl;
			}
		}
	}

	glClearColor(0.6, 0.8, 1.0, 0.0);
	glPolygonOffset(1, 1);
}

Renderer::~Renderer()
{
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
