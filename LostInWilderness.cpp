
#include "LostInWilderness.h"
#include "Chunk.h"
#include "Config.h"
#include "Exceptions.h"
#include "Network.h"
#include "Shader.h"
#include "Texture.h"
#include "World.h"
#include "WorldGenerator.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <queue>
#include <string>
#include <sstream>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

// TODO: Remove using namespace
using namespace std;


typedef struct ChunkUpdateStruct {
	enum	{
		CHUNK_GENERATE,
		CHUNK_UPDATE,
		CHUNK_UPDATE_LIGHTING,
		CHUNK_UPDATE_GRAPHIC
	}		type;
	Chunk	*chunk;
	int		x,
			y,
			z;
	int		seed;
} ChunkUpdateType;

static Camera		g_camera;
static World		*world;
static GLuint		cursor_vbo;
static GLuint		default_vao;
static int			ww, wh;
static int			mx, my, mz;
static int			face;
static uint8_t		buildtype = 1;
static unsigned int keys;
static bool			select_using_depthbuffer = false;

uint32_t chunk_update_count = 0;
uint32_t chunk_gen_count = 0;


// Shorthand: Size of one chunk in blocks
#define CX CHUNK_WIDTH
#define CY CHUNK_HEIGHT
#define CZ CHUNK_LENGTH


std::thread					g_chunk_updater_thread,
							g_chunk_fileio_thread;
std::atomic<bool>			g_chunk_updater_loop,
							g_chunk_fileio_loop;
std::queue<Chunk*>			thread_chunk_update_queue;
std::queue<Chunk*>			thread_chunk_fileio_queue;
std::queue<ChunkUpdateType>	thread_chunk_gen_queue;

void GaiaCraft::chunkUpdateThread( void )
{
	glfwMakeContextCurrent( (GLFWwindow*)Renderer::getWindow( "g_updatecontext" ) );

	// -- While the queue is not empty
	while ( g_chunk_updater_loop )
	{
		if ( thread_chunk_gen_queue.empty() )
		{
			if ( !thread_chunk_update_queue.empty() )
			{
				Chunk *chunk = thread_chunk_update_queue.front();
				thread_chunk_update_queue.pop();
				// NOTE: Uncommenting this line below fixes the weird VBO glitches but causes the game to process a lot more
				//chunk->update();
			}
			else
			{
				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
				continue;
			}
		}
		else
		{
			ChunkUpdateType chunk_data = thread_chunk_gen_queue.front();
			thread_chunk_gen_queue.pop();
			chunk_data.chunk->generate( chunk_data.seed );
		}
	}

	glfwMakeContextCurrent( nullptr );

	cout << "[THREAD:Updates] Chunk updates finished." << endl;
}

void GaiaCraft::chunkFileIOThread( void )
{
	// -- While the queue is not empty
	while ( g_chunk_fileio_loop )
	{
		if ( !thread_chunk_fileio_queue.empty() )
		{
			Chunk *chunk = thread_chunk_fileio_queue.front();
			thread_chunk_fileio_queue.pop();
			chunk->save();
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}
	}

	cout << "[THREAD:FileIO] Chunk file operations finished." << endl;
}

static void free_resources();

// Calculate the forward, right and lookat vectors from the angle vector
static void update_vectors()
{
	g_camera.forward.x = std::sin(g_camera.angle.x);
	g_camera.forward.y = 0;
	g_camera.forward.z = std::cos(g_camera.angle.x);

	g_camera.right.x = -std::cos(g_camera.angle.x);
	g_camera.right.y = 0;
	g_camera.right.z = std::sin(g_camera.angle.x);

	g_camera.lookat.x = std::sin(g_camera.angle.x) * std::cos(g_camera.angle.y);
	g_camera.lookat.y = std::sin(g_camera.angle.y);
	g_camera.lookat.z = std::cos(g_camera.angle.x) * std::cos(g_camera.angle.y);

	g_camera.up = glm::cross(g_camera.right, g_camera.lookat);
}

int init_resources()
{
	// -- Load Textures
	Renderer::blocks_texture = new Texture();
	Renderer::blocks_texture->loadFile( "data/textures/gc-textures.png", 16, 16 );
	Renderer::blocks_texture->bind();

	Renderer::font_texture = new TextFont( "data/fonts/minecraft" );

	// -- Network
	Network::HttpRequest http;
	http.open( Network::HttpRequest::HTTP_GET, "www.epiczen.net/" );
	http.send( std::string("test") );

	// -- Create shaders
	Shader *shader_world = new Shader();
	Renderer::default_shader = shader_world;

	//shader_world->addVertexShader( "data/shaders/world.120.vs" );
	//shader_world->addFragmentShader( "data/shaders/world.120.fs" );
	shader_world->addVertexShader( "data/shaders/world.150.vs" );
	shader_world->addFragmentShader( "data/shaders/world.150.fs" );
	shader_world->compileShaders();
	shader_world->bind();

	// -- Make the shader object aware of attributes and uniforms
	shader_world->addAttrib( "v_position" );
	shader_world->addAttrib( "v_normal" );
	shader_world->addAttrib( "v_texcoord" );
	shader_world->addAttrib( "v_diffuse" );
	shader_world->addUniform( "mvp" );
	shader_world->addUniform( "texture" );
	shader_world->addUniform( "b_lighting" );
	shader_world->addUniform( "g_SunLightSource.position" );
	shader_world->addUniform( "g_SunLightSource.diffuse" );
	shader_world->addUniform( "g_SunLightSource.ambient" );
	shader_world->addUniform( "g_SunLightSource.specular" );

	// TODO: Put this into the Shader class
	shader_world->bindFragData( 0, "frag_color" );

	// -- Default the uniforms/attribs
	shader_world->setUniform1i( "texture", 0 );
	shader_world->setUniform4f( "g_SunLightSource.position", (float*)glm::value_ptr( glm::vec4( 0, -1, 0, 0 ) ) );
	shader_world->setUniform4f( "g_SunLightSource.diffuse", (float*)glm::value_ptr( glm::vec4( 1, 1, 1, 1 ) ) );
	shader_world->setUniform4f( "g_SunLightSource.ambient", (float*)glm::value_ptr( glm::vec4( 0.4, 0.4, 0.4, 1 ) ) );
	shader_world->setUniform1f( "g_SunLightSource.specular", 1.0f );

	// -- Create a VBO for the cursor
	glGenVertexArrays(1, &default_vao);
	glGenBuffers(1, &cursor_vbo);
	glBindVertexArray( default_vao );

	// -- OpenGL settings that do not change while running this program
	glClearColor(0.6, 0.8, 1.0, 0.0);
	glPolygonOffset(1, 1);
	glEnableVertexAttribArray( shader_world->getAttrib( "v_position" ) );
	glEnableVertexAttribArray( shader_world->getAttrib( "v_normal" ) );
	glEnableVertexAttribArray( shader_world->getAttrib( "v_texcoord" ) );
	glEnableVertexAttribArray( shader_world->getAttrib( "v_diffuse" ) );

	// -- Create the world
	world = new World( "world" );

	// -- Pre-generate the terrain here


  // TODO: place the camera on the terrain
	g_camera.position = glm::vec3(0, CY / 2, 0);
	g_camera.angle = glm::vec3(0, -0.5, 0);
	update_vectors();

	return 1;
}

void resize( GLFWwindow* wnd, int w, int h)
{
	ww = w;
	wh = h;
	glViewport(0, 0, w, h);
}

// -- Not really GLSL fract(), but the absolute distance to the nearest integer value
static float fract(float value)
{
	float f = value - floorf(value);
	if(f > 0.5)
		return 1 - f;
	else
		return f;
}


static double prevFrameTime = 0.0;
static unsigned int framesPerSecond = 0;
static unsigned int frameCount = 0;

static void display()
{
	float fov = Config::getGlobal()->getFloat( "renderer.field_of_view" ) / 180.0f * 3.14f;
	float aspect = (float)ww / (float)wh;
	float znear = 0.25f;
	float zfar = 16.0f * (Config::getGlobal()->getInteger( "renderer.view_distance" ) + 1);

	glm::mat4 view = glm::lookAt(g_camera.position, g_camera.position + g_camera.lookat, g_camera.up);
	glm::mat4 projection = glm::perspective( fov, aspect, znear, zfar );
	glm::mat4 mvp = projection * view;

	Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(mvp) );
	Renderer::default_shader->setUniform1i( "b_lighting", true );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_POLYGON_OFFSET_FILL );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	// -- Then draw chunks

	world->render(mvp);

	// -- At which voxel are we looking?

	if ( select_using_depthbuffer )
	{
		// -- Find out coordinates of the center pixel

		float depth;
		glReadPixels(ww / 2, wh / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

		glm::vec4 viewport = glm::vec4(0, 0, ww, wh);
		glm::vec3 wincoord = glm::vec3(ww / 2, wh / 2, depth);
		glm::vec3 objcoord = glm::unProject(wincoord, view, projection, viewport);

		// -- Find out which block it belongs to

		mx = objcoord.x;
		my = objcoord.y;
		mz = objcoord.z;
		if(objcoord.x < 0)
			mx--;
		if(objcoord.y < 0)
			my--;
		if(objcoord.z < 0)
			mz--;

		// -- Find out which face of the block we are looking at

		if(fract(objcoord.x) < fract(objcoord.y))
			if(fract(objcoord.x) < fract(objcoord.z))
				face = 0; // X
			else
				face = 2; // Z
		else
			if(fract(objcoord.y) < fract(objcoord.z))
				face = 1; // Y
			else
				face = 2; // Z

		if(face == 0 && g_camera.lookat.x > 0)
			face += 3;
		if(face == 1 && g_camera.lookat.y > 0)
			face += 3;
		if(face == 2 && g_camera.lookat.z > 0)
			face += 3;
	}
	else
	{
		// -- Very naive ray casting algorithm to find out which block we are looking at

		glm::vec3 testpos = g_camera.position;
		glm::vec3 prevpos = g_camera.position;

		for ( auto i = 0; i < 16 * 4; i++ )
		{
			// -- Advance from our currect position to the direction we are looking at, in increments of 1 voxel.
			// NOTE: 1 voxel is equal to 1.0f / 16.0f

			prevpos = testpos;
			testpos += g_camera.lookat * (1.0f / 16.0f);

			mx = floorf(testpos.x);
			my = floorf(testpos.y);
			mz = floorf(testpos.z);

			// -- If this block is not air, then break the loop
			if ( world->getBlock(mx, my, mz) )
				break;
		}

		// -- Find out which face of the block we are looking at

		int px = floorf(prevpos.x);
		int py = floorf(prevpos.y);
		int pz = floorf(prevpos.z);

		if(px > mx)
			face = 0;
		else if(px < mx)
			face = 3;
		else if(py > my)
			face = 1;
		else if(py < my)
			face = 4;
		else if(pz > mz)
			face = 2;
		else if(pz < mz)
			face = 5;

		// -- If we are looking at air, move the cursor out of sight
		if ( world->getBlock(mx, my, mz) == nullptr )
		{
			mx = my = mz = 99999;
		}
	}

	/**float bx = mx;
	float by = my;
	float bz = mz;

	// Render a box around the block we are pointing at

	Vertex box[24] = {
		Vertex(bx + 0, by + 0, bz + 0, 0, 0, 0),
		Vertex(bx + 1, by + 0, bz + 0, 0, 0, 0),
		Vertex(bx + 0, by + 1, bz + 0, 0, 0, 0),
		Vertex(bx + 1, by + 1, bz + 0, 0, 0, 0),

		Vertex(bx + 0, by + 0, bz + 1, 0, 0, 0),
		Vertex(bx + 1, by + 0, bz + 1, 0, 0, 0),
		Vertex(bx + 0, by + 1, bz + 1, 0, 0, 0),
		Vertex(bx + 1, by + 1, bz + 1, 0, 0, 0),

		Vertex(bx + 0, by + 0, bz + 0, 0, 0, 0),
		Vertex(bx + 0, by + 1, bz + 0, 0, 0, 0),
		Vertex(bx + 1, by + 0, bz + 0, 0, 0, 0),
		Vertex(bx + 1, by + 1, bz + 0, 0, 0, 0),

		Vertex(bx + 0, by + 0, bz + 1, 0, 0, 0),
		Vertex(bx + 0, by + 1, bz + 1, 0, 0, 0),
		Vertex(bx + 1, by + 0, bz + 1, 0, 0, 0),
		Vertex(bx + 1, by + 1, bz + 1, 0, 0, 0),

		Vertex(bx + 0, by + 0, bz + 0, 0, 0, 0),
		Vertex(bx + 0, by + 0, bz + 1, 0, 0, 0),
		Vertex(bx + 1, by + 0, bz + 0, 0, 0, 0),
		Vertex(bx + 1, by + 0, bz + 1, 0, 0, 0),

		Vertex(bx + 0, by + 1, bz + 0, 0, 0, 0),
		Vertex(bx + 0, by + 1, bz + 1, 0, 0, 0),
		Vertex(bx + 1, by + 1, bz + 0, 0, 0, 0),
		Vertex(bx + 1, by + 1, bz + 1, 0, 0, 0)
	};

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_CULL_FACE);
	Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(mvp) );
	glBindBuffer(GL_ARRAY_BUFFER, cursor_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 24, &box[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer( Renderer::default_shader->getAttrib("v_position"), 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer( Renderer::default_shader->getAttrib("v_texcoord"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 4) );
	glDrawArrays(GL_LINES, 0, 24);

	// -- Draw a cross in the center of the screen

	Vertex cross[4] = {
		Vertex(-0.05*((float)wh/(float)ww), 0, 0, 0, 0, 0 ),
		Vertex(+0.05*((float)wh/(float)ww), 0, 0, 0, 0, 0 ),
		Vertex( 0, -0.05, 0, 0, 0, 0 ),
		Vertex( 0, +0.05, 0, 0, 0, 0 )
	};

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glm::mat4 one(1);
	Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(one) );
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, &cross[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer( Renderer::default_shader->getAttrib("v_position"), 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), 0 );
	glVertexAttribPointer( Renderer::default_shader->getAttrib("v_texcoord"), 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof(float) * 4 ) );

	glEnable( GL_BLEND );
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	glDrawArrays(GL_LINES, 0, 4);

	glDisable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_TEXTURE_2D );*/

	// -- Overlay
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_CULL_FACE);
	glDisable( GL_DEPTH_TEST );

	view		= glm::ortho( 0.0f, (float)ww, (float)wh, 0.0f );
	projection	= glm::mat4(1);
	mvp			= view * projection;

	Renderer::default_shader->setUniformMatrix( "mvp", glm::value_ptr(mvp) );
	//Renderer::default_shader->setUniform1i( "b_lighting", false );

	std::wostringstream overlay_s;
	overlay_s << L"FPS: " << framesPerSecond << L"\nUpdates: " << chunk_update_count << L"\nGens: " << chunk_gen_count << L"\nOpenGL 3.2 Core | GLSL 1.50";
	Renderer::font_texture->bind();
	Renderer::font_texture->drawString( 2, 2, 0xFFFFFFFF, 1.0f, overlay_s.str().c_str() );
	Renderer::blocks_texture->bind();


	// -- Swap the framebuffer back and front, then poll for events in this thread
	glfwSwapBuffers( (GLFWwindow*)Renderer::getWindow( "g_window" ) );
	glfwPollEvents();

	// -- Find the FPS
	double curTime = glfwGetTime();
	if ( curTime >= prevFrameTime+1.0 )
	{
		prevFrameTime = curTime;
		framesPerSecond = frameCount + 1;
		frameCount = 0;
		chunk_update_count = 0;
		chunk_gen_count = 0;
	}
	else frameCount++;
}

void key_cb( GLFWwindow* wnd, int key, int scancode, int action, int mods )
{
	if ( action == GLFW_RELEASE )
	switch(key) {
		case GLFW_KEY_A:
			keys &= ~1;
			break;
		case GLFW_KEY_D:
			keys &= ~2;
			break;
		case GLFW_KEY_W:
			keys &= ~4;
			break;
		case GLFW_KEY_S:
			keys &= ~8;
			break;
		case GLFW_KEY_SPACE:
			keys &= ~16;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			keys &= ~32;
			break;
	}
	if ( action == GLFW_PRESS )
	switch(key)
	{
		case GLFW_KEY_A:
			keys |= 1;
			break;
		case GLFW_KEY_D:
			keys |= 2;
			break;
		case GLFW_KEY_W:
			keys |= 4;
			break;
		case GLFW_KEY_S:
			keys |= 8;
			break;
		case GLFW_KEY_SPACE:
			keys |= 16;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			keys |= 32;
			break;
		case GLFW_KEY_HOME:
			g_camera.position = glm::vec3(0, CY + 1, 0);
			g_camera.angle = glm::vec3(0, -0.5, 0);
			update_vectors();
			break;
		case GLFW_KEY_END:
			g_camera.position = glm::vec3(0, CX * Config::getGlobal()->getInteger( "renderer.view_distance" ), 0);
			g_camera.angle = glm::vec3(0, -3.14 * 0.49, 0);
			update_vectors();
			break;
		case GLFW_KEY_F1:
			select_using_depthbuffer = !select_using_depthbuffer;
			if(select_using_depthbuffer)
				printf("Using depth buffer selection method\n");
			else
				printf("Using ray casting selection method\n");
			break;
		case GLFW_KEY_F4:
			{
				GLint pm;
				glGetIntegerv( GL_POLYGON_MODE, &pm );
				if ( pm == GL_FILL )
					glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				else
					glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			}
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose( (GLFWwindow*)Renderer::getWindow( "g_window" ), GL_TRUE );
			break;
	}
}

static void idle()
{
	static double pt = 0.0;
	static const float movespeed = 10;

	double t = glfwGetTime();
	float dt = (t - pt);
	pt = t;

	if(keys & 1)
		g_camera.position -= g_camera.right * movespeed * dt;
	if(keys & 2)
		g_camera.position += g_camera.right * movespeed * dt;
	if(keys & 4)
		g_camera.position += g_camera.forward * movespeed * dt;
	if(keys & 8)
		g_camera.position -= g_camera.forward * movespeed * dt;
	if(keys & 16)
		g_camera.position.y += movespeed * dt;
	if(keys & 32)
		g_camera.position.y -= movespeed * dt;
}

void motion(GLFWwindow *wnd, double x, double y)
{
	static bool warp = false;
	static const float mousespeed = 0.001f;

	if ( glfwGetWindowAttrib( (GLFWwindow*)Renderer::getWindow( "g_window" ), GLFW_FOCUSED) == GL_FALSE )
		return;

	int wx,wy;
	glfwGetWindowPos( (GLFWwindow*)Renderer::getWindow( "g_window" ), &wx, &wy );

	if(!warp)
	{
		g_camera.angle.x -= (x - (((float)ww / 2.0f)) ) * mousespeed;
		g_camera.angle.y -= (y - (((float)wh / 2.0f)) ) * mousespeed;

		if(g_camera.angle.x < -3.14)
			g_camera.angle.x += 3.14 * 2;
		if(g_camera.angle.x > 3.14)
			g_camera.angle.x -= 3.14 * 2;
		if(g_camera.angle.y < -3.14 / 2)
			g_camera.angle.y = -3.14 / 2;
		if(g_camera.angle.y > 3.14 / 2)
			g_camera.angle.y = 3.14 / 2;

		update_vectors();

		// Force the mouse pointer back to the center of the screen.
		// This causes another call to motion(), which we need to ignore.
		warp = true;
		glfwSetCursorPos( (GLFWwindow*)Renderer::getWindow( "g_window" ), (float)ww / 2.0, (float)wh / 2.0);
	} else {
		warp = false;
	}
}

static void mouse(int button, int state, int x, int y)
{
	if(state != GLFW_PRESS)
		return;

	// Scrollwheel
	if(button == 3 || button == 4)
	{
		if(button == 3)
			buildtype--;
		else
			buildtype++;

		//buildtype &= 0xf;
		//fprintf(stderr, "Building blocks of type %u (%s)\n", buildtype, blocknames[buildtype]);
		return;
	}

	fprintf(stderr, "Clicked on %d, %d, %d, face %d, button %d\n", mx, my, mz, face, button);

	/*if(button == 0)
	{
		if(face == 0)
			mx++;
		if(face == 3)
			mx--;
		if(face == 1)
			my++;
		if(face == 4)
			my--;
		if(face == 2)
			mz++;
		if(face == 5)
			mz--;
		world->set( mx, my, mz, buildtype );
	}
	else
	{
		world->set( mx, my, mz, nullptr );
	}*/
}

static void free_resources()
{
	cout << "[INFO] Resources freed" << endl;

	if ( world != nullptr )
		delete world;

	glDeleteVertexArrays(1, &default_vao);
}

GaiaCraft *GaiaCraft::iGaiaCraft;


GaiaCraft::GaiaCraft()
{
	GaiaCraft::iGaiaCraft = this;

	this->camera						= &g_camera;
	this->config						= Config::getGlobal();
	this->config->load( "client.properties" );

	// -- Prepare the network API
	Network::network_initialise();

	Renderer::iRenderer					= new Renderer( GaiaCraft::iGaiaCraft->config->getString( "renderer.api" ) );
    ResourceManager::iResourceManager	= new ResourceManager();
}

GaiaCraft::~GaiaCraft()
{
	Network::network_cleanup();

	delete this->config;
	delete ResourceManager::iResourceManager;
	delete Renderer::iRenderer;
}

void GaiaCraft::addChunkToGenerateQueue( int x, int y, int z, int seed, Chunk *chunk )
{
	ChunkUpdateType chunk_data;
	chunk_data.chunk	= chunk;
	chunk_data.x		= x;
	chunk_data.y		= y;
	chunk_data.z		= z;
	chunk_data.seed		= seed;
	thread_chunk_gen_queue.push( chunk_data );
}

void GaiaCraft::addChunkToUpdateQueue( Chunk *chunk )
{
	thread_chunk_update_queue.push( chunk );
}

void GaiaCraft::addChunkToSaveQueue( Chunk *chunk )
{
	thread_chunk_fileio_queue.push( chunk );
}

int GaiaCraft::run()
{
	g_chunk_updater_loop = true;
	g_chunk_updater_thread = thread( GaiaCraft::chunkUpdateThread );
	g_chunk_fileio_loop = true;
	g_chunk_fileio_thread = thread( GaiaCraft::chunkFileIOThread );

	if (init_resources())
	{
		glfwGetWindowSize( (GLFWwindow*)Renderer::getWindow( "g_window" ), &ww, &wh );
		glfwSetCursorPos( (GLFWwindow*)Renderer::getWindow( "g_window" ), ww / 2, wh / 2);
		glfwSetWindowSizeCallback( (GLFWwindow*)Renderer::getWindow( "g_window" ), resize );
		glfwSetKeyCallback( (GLFWwindow*)Renderer::getWindow( "g_window" ), key_cb );
		glfwSetCursorPosCallback( (GLFWwindow*)Renderer::getWindow( "g_window" ), motion );

		while ( !glfwWindowShouldClose( (GLFWwindow*)Renderer::getWindow( "g_window" ) ) )
		{
			idle();
			display();
		}
	}

	std::cout << "[INFO] Exited the loop" << std::endl;

	g_chunk_updater_loop = false;
	g_chunk_updater_thread.join();
	g_chunk_fileio_loop = false;
	g_chunk_fileio_thread.join();

	std::cout << "[INFO] Threads have joined" << std::endl;

	free_resources();

	return 0;
}

int main(int argc, char* argv[])
{
	int r = 0;

	try
	{
		GaiaCraft game;
		r = game.run();
	}
	catch ( std::exception& e )
	{
		std::cerr << "Fatal Exception:\n" << e.what() << std::endl;
	}

	return r;
}
