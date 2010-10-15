/*
    Copyright (C) 2010 Andrew Flower <andrew.flower@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifdef _WIN32
//#	pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#	pragma comment(linker, "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
#	define GLEW_STATIC 1
#	pragma comment(lib, "opengl32.lib")
#	pragma comment(lib, "sdl.lib")
#	pragma comment(lib, "sdlmain.lib")
#	pragma comment(lib, "sdl_image.lib")
#	pragma comment(lib, "freeimage.lib")
#endif

class Deform;

#include <vector>
#include <list>
#include <queue>
#include <limits.h>

#ifdef _WIN32
#	include <direct.h>
#else
#	include <sys/stat.h>
#	define mkdir(x) mkdir(x, S_IRWXU)
#endif

#include "regl3.h"
#include "FreeImage.h"
#include "re_math.h"
using namespace reMath;
#include <map>
using namespace std;
#include "re_shader.h"
#include "util.h"
#include "deform.h"
#include "clipmap.h"
#include "skybox.h"
#include "caching.h"
#include "main.h"

#define PI					(3.14159265358f)
#define FAR_PLANE			(1000.0f)
#define NEAR_PLANE			(.5f)

#define WRAP_POS(p,b)		( p < -b * .5f ? p + b : \
							( p >  b * .5f ? p - b : p))

extern const int SCREEN_W	= 1024;
extern const int SCREEN_H	=  768;
extern const float ASPRAT	= float(SCREEN_W) / SCREEN_H;

#define COARSEMAP_FILENAME	("images/hmap02.png")
#define COARSEMAP_TEXTURE	("images/hmap03_texture.png")
#define	SPLASHMAP_TEXTURE	("images/splash.png")
#define CLIPMAP_DIM			(255)
#define CLIPMAP_RES			(.1f)
#define CLIPMAP_LEVELS		(5)
#define CACHING_LEVEL		(2)
#define CACHING_DIM			((CLIPMAP_DIM + 1) * CACHING_LEVEL)
#define HIGH_DIM			(1024 * CACHING_LEVEL)
#define HIGH_RES			(CLIPMAP_RES / 3.0f)
#define HD_AURA				(CACHING_DIM * CLIPMAP_RES / 2.0f)
#define HD_AURA_SQ			(HD_AURA * HD_AURA)
#define COARSE_AURA			((CLIPMAP_DIM + 1) * 8 * CLIPMAP_RES)
#define VERT_SCALE			(40.0f)
#define GRAVITY				(-9.81f)
#define EYE_HEIGHT			(2.0f)

//float timeCount = 0;
//long frameCount = 0;


/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[])
{
	AppConfig conf;
	conf.VSync		= false;
	conf.gl_major	= 3;
	conf.gl_minor	= 2;
	conf.fsaa		= 0;
	conf.sleepTime	= 0.01f;
	conf.winWidth	= SCREEN_W;
	conf.winHeight	= SCREEN_H;
	DefTer test(conf);

	int sleepTime = 1000;
	if (!test.Start())
	{
		printf("\nApplication failed to start\n");
		sleepTime *= 10;
	}

#ifdef _WIN32
	Sleep(sleepTime);
#endif

	//printf("Average render: %.3fms\n", timeCount*1000.0f / frameCount);

	return 0;
}


//--------------------------------------------------------
DefTer::DefTer(AppConfig& conf) : reGL3App(conf)
{
	m_shMain  		= NULL;
	m_pDeform 		= NULL;
	m_pClipmap		= NULL;
	m_pCaching		= NULL;
	m_pSkybox 		= NULL;
	m_elevationData	= NULL;
}

//--------------------------------------------------------
DefTer::~DefTer()
{
	glDeleteBuffers(3, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	FreeImage_DeInitialise();
	glUseProgram(0);
	RE_DELETE(m_shSplash);
	RE_DELETE(m_shMain);
	RE_DELETE(m_pDeform);
	RE_DELETE(m_pSkybox);
	RE_DELETE(m_pClipmap);
	RE_DELETE(m_pCaching);
	if (m_elevationData)
		delete [] m_elevationData;
	glDeleteBuffers(2, m_pbo);
	glDeleteTextures(1, &m_coarsemap.heightmap);
	glDeleteTextures(1, &m_coarsemap.pdmap);
	glDeleteTextures(1, &m_colormap_tex);
	glDeleteTextures(1, &m_splashmap);
}

//--------------------------------------------------------
bool
DefTer::InitGL()
{
	int res;
	char* shVersion;
	int nVertTexUnits, nGeomTexUnits, nTexUnits, nTexUnitsCombined, nColorAttachments, nTexSize,
		nTexLayers, nVertAttribs, nGeomVerts, nGeomComponents;

	if (!reGL3App::InitGL())
		return false;

#ifdef _WIN32
	if (glewInit()!=GLEW_OK)
	{
		printf("Could not init GLEW\n");
		return false;
	}
#endif

	printf("\n\n");
	printf("-----------------------------------------\n");
	printf("------------Hardware Features------------\n");
	printf("-----------------------------------------\n");

	// Query some Hardware specs
	shVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nTexUnits);
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &nVertTexUnits);
	glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &nGeomTexUnits);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nTexUnitsCombined);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nTexSize);
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &nGeomVerts);
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &nGeomComponents);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &nTexLayers);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &nColorAttachments);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nVertAttribs);
	printf("Supports GLSL version:\t\t%s\n", shVersion);
	printf("# of texture units:\t\t%d (VS), %d (GS), %d (FS), %d (comb.)\n",nVertTexUnits,
			nGeomTexUnits, nTexUnits, nTexUnitsCombined);
	printf("Max texture size:\t\t%d\n", nTexSize);
	printf("Max output vertices (GS):\t%d\n", nGeomVerts);
	printf("Max output components:\t\t%d\n", nGeomVerts);
	printf("Max FBO color attachments:\t%d\n", nColorAttachments);
	printf("Max array texture layers:\t%d\n", nTexLayers);
	printf("Max vertex attributes:\t\t%d\n", nVertAttribs);

	printf("\n");
	printf("-----------------------------------------\n");
	printf("------------Program Launching------------\n");
	printf("-----------------------------------------\n");

	printf("Setting up GL...\t\t");
	glEnable(GL_CULL_FACE);
	glClearColor(0.4f, 0.4f, 1.0f, 1.0f);
	if (!CheckError(""))
		return false;
	printf("Done\n");
	
	//Initialise FreeImage
	FreeImage_Initialise();

	// Init the splash screen info
	printf("Initialising Splash screen...\t");
	if (!InitSplash())
	{
		printf("Error\n\tSplash screen initialisation error\n");
		return false;
	}
	printf("Done\n");

	// Render the splash screen
	printf("Rendering splash screen...\t");
	RenderSplash();
	if (!CheckError("Splash rendering"))
		return false;
	printf("Done\n");


	// Init projection matrix
	m_proj_mat		= perspective_proj(PI*.5f, ASPRAT, NEAR_PLANE, FAR_PLANE);

	// Init the cameras position such that it is in the middle of a tile
	float halfTile = HIGH_DIM * HIGH_RES * 0.5f;
	m_cam_translate.set(-halfTile, 0.0f, -halfTile);

	// Set the initial stamp mode and clicked state
	m_stampName		= "Gaussian";
	m_stampIntensity= 0.2f;
	m_stampScale	= 50.0f;
	m_is_hd_stamp	= false;
	m_clicked		= false;
	m_clickPos		= vector2(0.0f);
	m_clickPosPrev	= vector2(0.0f);

	// Init the world settings
	m_gravity_on	= true;
	m_is_crouching	= false;
	m_fall_speed	= .0f;

	// Init Shaders
	// Get the Shaders to Compile
	m_shMain		= new ShaderProg("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "vert_TexCoord");

	// NB. must be done after binding attributes
	printf("Compiling shaders...\t\t");
	res = m_shMain->CompileAndLink();
	if (!res)
	{
		printf("Error\n\tWill not continue without working shaders\n");
		return false;
	}

	// Assign samplers to texture units
	glUseProgram(m_shMain->m_programID);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "heightmap"), 0);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "pdmap"), 1);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "colormap"),  2);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "detail0"),  3);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "detail1"),  4);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "detail2"),  5);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "detail3"),  6);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "projection"), 1, GL_FALSE,	m_proj_mat.m);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "is_hd_stamp"), (m_is_hd_stamp ? 1.0f : 0.0f));

	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("Done\n");

	// Initialise the program
	if (!Init())
		return false;
	printf("\nProgram Loaded Successfully\t=D\n");
	
	//Print out some cool stats about the various components
	printf("\n");
	printf("-----------------------------------------\n");
	printf("-------------Some Cool Stats-------------\n");
	printf("-----------------------------------------\n");
	printf(
	"Clipmap Stats:\n"
	"--------------\n"
	"%s\n"
	, m_pClipmap->m_clipmap_stats.c_str());
	
	printf("\n");
	printf(
	"Caching Stats:\n"
	"--------------\n"
	"%s\n"
	, m_pCaching->m_caching_stats.c_str());

	// Print key commands
	printf("\n");
	printf("-----------------------------------------\n");
	printf("-------------System Controls-------------\n");
	printf("-----------------------------------------\n");
	printf(
	"w,a,s,d\t"	"= Camera Translation\n"
	"l\t"		"= Lines/Wireframe Toggle\n"
	"c\t"		"= En/Disable Frustum Culling\n"
	"g\t"		"= Toggle Gravity\n"
	"Space\t"	"= Jump/Float\n"
	"L-Ctrl\t"	"= Crouch/Sink\n"
	"h\t"		"= High Detail Toggle\n"
	"L-Shift\t"	"= En/Disable Super Speed\n"
	"R-Mouse\t"	"= Pick Deform location\n"
	"L-Mouse\t" "= Rotate Camera\n"
	"Wheel\t"	"= Deform\n"
	"F12\t"		"= Screenshot\n"
	"Esc\t"		"= Quit\n"
	);
	printf("-----------------------------------------\n");
	printf("-------------Stamp  Controls-------------\n");
	printf("-----------------------------------------\n");
	printf(
	"Pg Up/Dn"	"= Stamp Scale\n"
	"+/-\t"		"= Stamp Intensity \n"
	"0\t"		"= %%\n"
	"1\t"		"= Gaussian\n"
	);
	printf("-----------------------------------------\n");
	printf("-----------------------------------------\n");
	printf("\n");

	return true;
}

//--------------------------------------------------------
bool
DefTer::Init()
{
	// Init the PBO buffers
	glGenBuffers(2, m_pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(GLubyte) * 3 * SCREEN_W * SCREEN_H, NULL,	GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(GLubyte) * 3 * SCREEN_W * SCREEN_H, NULL,	GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Load heightmap
	printf("Loading coarsemap...\t\t");
	if (!LoadCoarseMap(COARSEMAP_FILENAME))
		return false;
	printf("Done\n");

	// Load colormap
	printf("Loading colormap...\t\t");
	if (!LoadPNG(&m_colormap_tex, COARSEMAP_TEXTURE))
		return false;
	printf("Done\n");

	// Create & initialise the clipmap
	printf("Creating clipmap...\t\t");
	m_pClipmap = new Clipmap(CLIPMAP_DIM, CLIPMAP_RES, CLIPMAP_LEVELS, m_coarsemap_dim);
	printf("Done\n");
	printf("Initialising clipmap...\t\t");
	m_pClipmap->init();
	printf("Done\n");

	// Shader uniforms (Clipmap data)
	glUseProgram(m_shMain->m_programID);
	glUniform2f(glGetUniformLocation(m_shMain->m_programID, "scales"), m_pClipmap->m_tex_to_metre, m_pClipmap->m_metre_to_tex);

	// Create the deformer object
	printf("Creating deformer...\t\t");
	m_pDeform = new Deform(m_coarsemap_dim, HIGH_DIM, m_pClipmap->m_metre_to_tex, 1.0f/(HIGH_DIM * HIGH_RES));
	if (!m_pDeform->m_no_error)
	{
		fprintf(stderr, "Error\n\tCould not create deformer\n");
		return false;
	}
	printf("Done\n");

	// Generate the normal map and run a zero deform to init shaders
	printf("Creating initial deform...\t");
	m_pDeform->displace_heightmap(m_coarsemap, vector2(0.5f), vector2(0.0f), m_stampName, vector3(0.0f, 0.0f, 0.0f), true);
	m_pDeform->create_pdmap(m_coarsemap, true);
	if (!CheckError("Creating initial deform"))
		return false;
	printf("Done\n");

	// Create & initialise the caching system
	printf("Creating caching system...\t");
	m_pCaching = new Caching(m_pDeform, CACHING_DIM, m_coarsemap_dim, CLIPMAP_RES, HIGH_DIM, HIGH_RES);
	printf("Done\n");
	printf("Initialising caching system...\t");
	m_pCaching->Init(m_coarsemap.heightmap, m_colormap_tex, vector2(m_cam_translate.x, m_cam_translate.z));
	printf("Done\n");

	// Create the skybox object
	printf("Creating skybox...\t\t");
	m_pSkybox = new Skybox();
	if (!m_pSkybox->m_no_error)
	{
		fprintf(stderr, "\t\tError\n\tCould not create skybox\n");
		return false;
	}
	printf("Done\n");

	// Assign some more to shader uniforms
	glUseProgram(m_shMain->m_programID);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "hd_aura_sq"), 
			float(HD_AURA_SQ) / (CLIPMAP_RES*m_coarsemap_dim * CLIPMAP_RES*m_coarsemap_dim));
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "inv_tile_size"), 
				1.0f/ (HIGH_RES * HIGH_DIM * m_pClipmap->m_metre_to_tex));

	return true;
}

//--------------------------------------------------------
// Setup the splash screen
//--------------------------------------------------------
bool
DefTer::InitSplash(void)
{
	// Setup shader
	m_shSplash = new ShaderProg("shaders/splash.vert", "", "shaders/splash.frag");
	glBindAttribLocation(m_shSplash->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shSplash->m_programID, 1, "vert_texCoord");
	if (!m_shSplash->CompileAndLink())
		return false;

	// Load the splash map
	if (!LoadPNG(&m_splashmap, SPLASHMAP_TEXTURE, false, true))
		return false;

	// Set uniforms
	glUseProgram(m_shSplash->m_programID);
	glUniform1i(glGetUniformLocation(m_shSplash->m_programID, "splashmap"), 0);

	// Vertex positions
	GLfloat square[]	= { -1.0f, -1.0f,
							 1.0f, -1.0f,
							 1.0f,  1.0f,
							-1.0f,  1.0f };
	// Texcoords are upside-down to mimic the systems coordinates
	GLfloat texcoords[]	= { 0.0f, 0.0f,
							1.0f, 0.0f,
							1.0f, 1.0f,
							0.0f, 1.0f };
	GLuint indices[]	= { 3, 0, 2, 1 };

	// Create the vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(3, m_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, square, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);
	
	return true;
}

//--------------------------------------------------------
// Render the splash screen
//--------------------------------------------------------
void
DefTer::RenderSplash(void)
{
	// Clear the screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Bind the vertex array
	glBindVertexArray(m_vao);

	// Set the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_splashmap);

	// Draw the screen
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);

	// Swap windows to show screen
	SDL_GL_SwapWindow(m_pWindow);
}

//--------------------------------------------------------
// LOADCOARSEMAP loads a heightmap from a png file for the coarse map.
// It also allocates memory for the normal and tangent maps.
bool
DefTer::LoadCoarseMap(string filename)
{
	FIBITMAP*		image;
	BYTE*			bits;
	int				bitdepth;

	image = FreeImage_Load(FIF_PNG, filename.c_str(), 0);
	if (image == NULL)
	{
		fprintf(stderr, "Error\n\tCould not load PNG: %s\n", filename.c_str());
		return false;
	}

	// Get image details
	m_coarsemap_dim = FreeImage_GetWidth(image);
	bitdepth		= FreeImage_GetBPP(image);
	bits 			= (BYTE*) FreeImage_GetBits(image);

	FreeImage_FlipVertical(image);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_coarsemap.heightmap);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.heightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Loading PNG heightmap, setting parameters"))
	{
		fprintf(stderr, "\tFile: %s\n", filename.c_str());
		return false;
	}

	if (bitdepth==8)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RED,	GL_UNSIGNED_BYTE, bits);
	}
	else if (bitdepth==16)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RED,	GL_UNSIGNED_SHORT, bits);
	}
	else
	{
		fprintf(stderr, "Error\n\tCannot load files with bitdepths other than 8- or 16-bit: %s\n",
				filename.c_str());
		return false;
	}

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create elevationData for camera collisions
	m_elevationData = new float[m_coarsemap_dim * m_coarsemap_dim];
	float scale = VERT_SCALE * (bitdepth == 8 ? 1.0f/255 : 1.0f/USHRT_MAX);
	for (int i = 0; i < m_coarsemap_dim * m_coarsemap_dim; i++)
	{
		m_elevationData[i] = bits[i] * scale;
	}

	// Allocate GPU Texture space for partial derivative map
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_coarsemap.pdmap);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.pdmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);

	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
// Updates the click position such that it follows the camera in HD mode,
// Also will hide in shaders if no point is currently 'clicked'
void
DefTer::UpdateClickPos(void)
{
	vector2 temp(2.0f);
	// If a there is a click check this here
	if (m_clicked)
	{
		// Check that the dot is not further than the max 'allowable distance' if in HD stamp mode
		if (m_is_hd_stamp)
		{
			vector2 camPos(m_cam_translate.x, m_cam_translate.z);
			m_clickPos -= camPos;

			// Calculate the magnitude and scale to keep in range
			float pmag  = m_clickPos.Mag();
			if (pmag > HD_AURA)
			{
				float scale = HD_AURA / pmag;
				m_clickPos *= scale;
			}
			m_clickPos += camPos;
		}

		temp  = m_clickPos * m_pClipmap->m_metre_to_tex;
		temp += vector2(0.5f);
	}
	else
	{
		m_clickPos = temp;
	}

	// Pass to shader if value has changed
	if (m_clickPosPrev != m_clickPos)
	{
		m_clickPosPrev = m_clickPos;

		glUseProgram(m_shMain->m_programID);
		glUniform2f(glGetUniformLocation(m_shMain->m_programID, "click_pos"), temp.x, temp.y);
	}
}

//--------------------------------------------------------
// Interpolates the height of the coarsemap at the given location in world space
float
DefTer::InterpHeight(vector2 worldPos)
{
	worldPos = (worldPos*m_pClipmap->m_metre_to_tex + vector2(.5f)) * m_coarsemap_dim;
	int x 	= int(worldPos.x);
	int y 	= int(worldPos.y);
	float fx= worldPos.x - x;
	float fy= worldPos.y - y;
	
	float x0 = (1-fx) * m_elevationData[x   + m_coarsemap_dim * (y)		] 
		     + (  fx) * m_elevationData[x+1 + m_coarsemap_dim * (y)		];
	float x1 = (1-fx) * m_elevationData[x   + m_coarsemap_dim * (y+1)	] 
		     + (  fx) * m_elevationData[x+1 + m_coarsemap_dim * (y+1)	];

	return (1-fy) * x0 + fy * x1 + EYE_HEIGHT * (m_is_crouching ? .5f : 1.0f);
}

//--------------------------------------------------------
// Process user input
void
DefTer::ProcessInput(float dt)
{
	int wheel_ticks = m_input.GetWheelTicks();
	MouseDelta move = m_input.GetMouseDelta();

	// Rotate Camera
	if (m_input.IsButtonPressed(1))
	{
		// Pitch
		m_cam_rotate.x += dt*move.y*PI*.1f;
		// Yaw
		m_cam_rotate.y += dt*move.x*PI*.1f;

		//Clamp the camera to prevent the user flipping
		//upside down messing up everything
		if (m_cam_rotate.x < -M_PI * 0.5f)
			m_cam_rotate.x = -M_PI * 0.5f;
		if (m_cam_rotate.x > M_PI * 0.5f)
			m_cam_rotate.x = M_PI * 0.5f;
	}

	// Change the selected deformation location
	if (m_input.IsButtonPressed(3))
	{
		MousePos pos = m_input.GetMousePos();
		float val;

		// Get value in z-buffer
		glReadPixels((GLint)pos.x, (GLint)(SCREEN_H - pos.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &val);

		vector3 frag(pos.x, pos.y, val);
		// Derive inverse of view transform (could just use transpose of view matrix
		matrix4 inverse = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);

		// Request unprojected coordinate
		vector3 p = perspective_unproj_world(frag, SCREEN_W, SCREEN_H, NEAR_PLANE, FAR_PLANE, 1.0f, inverse);

		// Check that the position is in valid range when using coarse stamping
		if (!m_is_hd_stamp && vector2(p.x, p.z).Mag() > COARSE_AURA)
		{
			m_clicked = false;
		}
		else
		{
			// Factor in the camera translation
			p += m_cam_translate;
			m_clickPos	= vector2(p.x, p.z);
			m_clicked	= true;
		}

		//Update the clicked position in shaders, etc...
		UpdateClickPos();
	}

	// Increase the game speed
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
	{
		dt *= 5.0f;
		m_is_super_speed = true;
	}
	if (m_is_super_speed && !m_input.IsKeyPressed(SDLK_LSHIFT))
	{
		m_is_super_speed = false;
	}

	// Change the selected deformation location
	if (m_clicked && wheel_ticks != 0)
	{
		if (m_is_hd_stamp)
			m_pCaching->DeformHighDetail(m_clickPos, m_stampName, 10.0f, 0.4f * wheel_ticks);
		else
		{
			vector2 areaMin(m_clickPos - vector2(m_stampScale / 2.0f));
			vector2 areaMax(areaMin	+ vector2(m_stampScale));

			areaMin *= m_pClipmap->m_metre_to_tex;
			areaMin += vector2(0.5f);
			areaMax *= m_pClipmap->m_metre_to_tex;
			areaMax += vector2(0.5f);

			// Left-Col
			if (areaMin.x < 0.0 && areaMax.y > 1.0)
			{
				// Left-Top
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(1.0f, -1.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			if (areaMin.x < 0.0)
			{
				// Left-Centre
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(1.0f, 0.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			if (areaMin.x < 0.0 && areaMin.y < 0.0)
			{
				// Left-Bottom
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(1.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			// Centre-Col
			if (areaMax.y > 1.0)
			{
				// Centre-Top
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(0.0f, -1.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			if (areaMin.y < 0.0)
			{
				// Centre-Bottom
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(0.0f, 1.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			// Right-Col
			if (areaMax.x > 1.0 && areaMax.y > 1.0)
			{
				// Right-Top
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(-1.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			if (areaMax.x > 1.0)
			{
				// Right-Centre
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(-1.0f, 0.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			if (areaMax.x > 1.0 && areaMin.y < 0.0)
			{
				// Right-Bottom
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(-1.0f, 1.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);
			}
			
			m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, vector2(0.0f), m_stampName, vector3(m_stampScale, m_stampIntensity * wheel_ticks, 0.0f), true);

			// Left-Col
			if (areaMin.x < 0.0 && areaMax.y > 1.0)
			{
				// Left-Top
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(1.0f, -1.0f), m_stampScale, true);
			}
			if (areaMin.x < 0.0)
			{
				// Left-Centre
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(1.0f, 0.0f), m_stampScale, true);
			}
			if (areaMin.x < 0.0 && areaMin.y < 0.0)
			{
				// Left-Bottom
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(1.0f), m_stampScale, true);
			}
			// Centre-Col
			if (areaMax.y > 1.0)
			{
				// Centre-Top
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(0.0f, -1.0f), m_stampScale, true);
			}
			if (areaMin.y < 0.0)
			{
				// Centre-Bottom
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(0.0f, 1.0f), m_stampScale, true);
			}
			// Right-Col
			if (areaMax.x > 1.0 && areaMax.y > 1.0)
			{
				// Right-Top
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(-1.0f), m_stampScale, true);
			}
			if (areaMax.x > 1.0)
			{
				// Right-Centre
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(-1.0f, 0.0f), m_stampScale, true);
			}
			if (areaMax.x > 1.0 && areaMin.y < 0.0)
			{
				// Right-Bottom
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(-1.0f, 1.0f), m_stampScale, true);
			}
			
			m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, vector2(0.0f), m_stampScale, true);
		}
	}

	// Take screenshot
	static int lastScreenshot = 1;
	if (m_input.WasKeyPressed(SDLK_F12))
	{
		char filename[256];
		GLubyte* framebuffer = new GLubyte[3 * SCREEN_W * SCREEN_H];
		glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)framebuffer);

		mkdir("screenshots");
		sprintf(filename, "screenshots/screenshot%05d.png", lastScreenshot++);
		if (SavePNG(filename, framebuffer, 8, 3, SCREEN_W, SCREEN_H, false))
			printf("Wrote screenshot to %s\n", filename);
		else
			fprintf(stderr, "Failed to write screenshot\n");

		delete[] framebuffer;		
	}

	// Toggle Frustum Culling
	if (m_input.WasKeyPressed(SDLK_c))
	{
		m_pClipmap->m_cullingEnabled ^= true;
		printf("Frustum Culling Enabled: %s\n", m_pClipmap->m_cullingEnabled ? "ON" : "OFF");
	}

	// Toggle wireframe
	static bool wireframe = false;
	if (m_input.WasKeyPressed(SDLK_l))
	{
		wireframe ^= true;
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Toggle between HD and coarse mode
	if (m_input.WasKeyPressed(SDLK_h))
	{
		m_clicked = false;

		m_is_hd_stamp ^= true;

		// Update the click position
		UpdateClickPos();

		glUseProgram(m_shMain->m_programID);
		glUniform1f(glGetUniformLocation(m_shMain->m_programID, "is_hd_stamp"), (m_is_hd_stamp ? 1.0f : 0.0f));

		printf("HD Mode: %s\n", m_is_hd_stamp ? "ON" : "OFF");
	}

	// Change between a set of stamps
	if (m_input.WasKeyPressed(SDLK_1))
	{
		m_stampName = "%";
		printf("Stamp: %%\n");
	}
	else if (m_input.WasKeyPressed(SDLK_2))
	{
		m_stampName = "Gaussian";
		printf("Stamp: Gaussian\n");
	}

	// Change the scale of the stamp
	if (m_input.IsKeyPressed(SDLK_PAGEUP))
	{
		float change = 0.1f * (m_is_super_speed ? 10.0f : 1.0f);
		m_stampScale = min(m_stampScale + change, 200.0f);
		printf("Stamp Scale: %.1f\n", m_stampScale);
	}
	else if (m_input.IsKeyPressed(SDLK_PAGEDOWN))
	{
		float change = 0.1f * (m_is_super_speed ? 10.0f : 1.0f);
		m_stampScale = max(m_stampScale - change, 0.1f);
		printf("Stamp Scale: %.1f\n", m_stampScale);
	}
	// Change the intensity of the stamp
	if (m_input.IsKeyPressed(SDLK_PLUS) || m_input.IsKeyPressed(SDLK_KP_PLUS))
	{
		m_stampIntensity = min(m_stampIntensity + 0.01f, 1.0f);
		printf("Stamp Intensity: %.2f\n", m_stampIntensity);
	}
	else if (m_input.IsKeyPressed(SDLK_MINUS) || m_input.IsKeyPressed(SDLK_KP_MINUS))
	{
		m_stampIntensity = max(m_stampIntensity - 0.01f, 0.01f);
		printf("Stamp Intensity: %.2f\n", m_stampIntensity);
	}

	// Toggle gravity
	if (m_input.WasKeyPressed(SDLK_g))
	{
		m_gravity_on ^= true;
		printf("Gravity: %s\n", m_gravity_on ? "ON" : "OFF");
	}

	// Controls to handle movement of the camera
	// Speed in m/s (average walking speed)
	float speed = 1.33f;
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		speed*=3.0f;
	if (m_input.IsKeyPressed(SDLK_w))
	{
		matrix4 rot =  rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);
		m_cam_translate += rot * vector3(.0f, .0f, -speed) * dt;
		UpdateClickPos();
	}
	if (m_input.IsKeyPressed(SDLK_s))
	{
		matrix4 rot =  rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);
		m_cam_translate += rot * vector3(.0f, .0f, speed) * dt;
		UpdateClickPos();
	}
	if (m_input.IsKeyPressed(SDLK_a))
	{
		matrix4 rot =  rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);
		m_cam_translate += rot * vector3(-speed, .0f, .0f) * dt;
		UpdateClickPos();
	}
	if (m_input.IsKeyPressed(SDLK_d))
	{
		matrix4 rot =  rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);
		m_cam_translate += rot * vector3(speed, .0f, .0f) * dt;
		UpdateClickPos();
	}

	// Boundary check for wrapping position
	static float boundary = m_coarsemap_dim* m_pClipmap->m_quad_size;
	m_cam_translate.x = WRAP_POS(m_cam_translate.x, boundary);
	m_cam_translate.z = WRAP_POS(m_cam_translate.z, boundary);

	// Controls for jumping (Floating)
	if (m_input.WasKeyPressed(SDLK_SPACE))
	{
		// Only jump if on the ground
		if (close_enough(m_fall_speed, .0f))
			m_fall_speed = 5.0f;
	}
	else if (m_input.IsKeyPressed(SDLK_SPACE) && !m_gravity_on)
	{
		// Float if gravity off
		m_cam_translate.y += 5.0f * dt;
	}
	// Controls for crouching (Sinking)
	if (m_input.IsKeyPressed(SDLK_LCTRL))
	{
		// Sink if gravity off
	   if (!m_gravity_on)
	   {
			m_cam_translate.y -= 5.0f * dt;
	   }
	   // Crouch - Drop camera down
	   else
	   {
		   m_is_crouching = true;
		   m_cam_translate.y -= 100.0f;
	   }
	}
	// Disable crouching if it was previously enabled and no longer pressing Ctrl
	if (m_is_crouching && !m_input.IsKeyPressed(SDLK_LCTRL))
	{
		m_is_crouching = false;
	}
	
	reGL3App::ProcessInput(dt);
}

//--------------------------------------------------------
void
DefTer::Logic(float dt)
{
	// Increase game speed
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		dt *= 5.0f;

	// Update the caching system
	m_pCaching->Update(vector2(m_cam_translate.x, m_cam_translate.z), vector2(m_cam_rotate.x, m_cam_rotate.y));

	// Update position
	glUseProgram(m_shMain->m_programID);
	if (m_gravity_on)
	{
		m_fall_speed	  += GRAVITY * dt;
		m_cam_translate.y += m_fall_speed * dt;

		float terrain_height = InterpHeight(vector2(m_cam_translate.x, m_cam_translate.z));
		if (m_cam_translate.y < terrain_height)
		{
			m_cam_translate.y = terrain_height;
			m_fall_speed = 	  .0f;
		}
	}
	else
	{
		float terrain_height = InterpHeight(vector2(m_cam_translate.x, m_cam_translate.z));
		if (m_cam_translate.y < terrain_height)
		{
			m_cam_translate.y = terrain_height;
			m_fall_speed = 	  .0f;
		}
	}
	vector3 pos = m_cam_translate * m_pClipmap->m_metre_to_tex;

	// Pass the camera's texture coordinates and the shift amount necessary
	// cam = x and y   ;  shift = z and w
	m_clipmap_shift.x = -fmodf(m_cam_translate.x, 32*m_pClipmap->m_quad_size);
	m_clipmap_shift.y = -fmodf(m_cam_translate.z, 32*m_pClipmap->m_quad_size);
	

	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "cam_height"), m_cam_translate.y);
	glUniform4f(glGetUniformLocation(m_shMain->m_programID, "cam_and_shift"), pos.x, pos.z, m_clipmap_shift.x, m_clipmap_shift.y);
}

//--------------------------------------------------------
void
DefTer::Render(float dt)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	matrix4 rotate, viewproj;

	rotate   = rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f) * rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);
	viewproj = m_proj_mat * rotate;

	// Bind coarse heightmap and its corresponding normal and colour maps
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.heightmap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.pdmap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_colormap_tex);
	
	// Use the shader program and set the view matrix.
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "view"), 1, GL_FALSE, rotate.m);

	// Cull invisible blocks and render clipmap
	m_pClipmap->cull(viewproj, m_clipmap_shift);

	// Block of four tiles where the 0th tile is the top left active tile
	Tile* activeTiles[4];
	m_pCaching->GetActiveTiles(activeTiles);
	int firstTile[2] = {activeTiles[0]->m_row, activeTiles[0]->m_col};
	glUniform2i(glGetUniformLocation(m_shMain->m_programID, "tileOffset"), firstTile[1],
			firstTile[0]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, activeTiles[0]->m_texdata.heightmap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, activeTiles[1]->m_texdata.heightmap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, activeTiles[2]->m_texdata.heightmap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, activeTiles[3]->m_texdata.heightmap);
	//--
/*
	static reTimer timer;
	glFinish();
	timer.start();
*/
	m_pClipmap->render();
	
	m_pSkybox->render(viewproj);

	m_pCaching->Render();
/*
	glFinish();
	timeCount += timer.getElapsed();
	frameCount ++;
*/
	SDL_GL_SwapWindow(m_pWindow);
}
