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

#define COARSEMAP_FILENAME	("images/hmap02.png")
#define COARSEMAP_TEXTURE	("images/hmap03_texture.png")
#define CLIPMAP_DIM			(255)
#define CLIPMAP_RES			(.1f)
#define CLIPMAP_LEVELS		(5)
#define CACHING_LEVEL		(2)
#define CACHING_DIM			((CLIPMAP_DIM + 1) * CACHING_LEVEL)
#define HIGH_DIM			(1024 * CACHING_LEVEL)
#define HIGH_RES			(CLIPMAP_RES / 3.0f)
#define HD_AURA				(CACHING_DIM * CLIPMAP_RES / 2.0f)


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
	conf.sleepTime	= 0.0f;
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

	return 0;
}


//--------------------------------------------------------
DefTer::DefTer(AppConfig& conf) : reGL3App(conf)
{
	m_shMain  = NULL;
	m_pDeform = NULL;
	m_pClipmap= NULL;
	m_pCaching= NULL;
}

//--------------------------------------------------------
DefTer::~DefTer()
{
	FreeImage_DeInitialise();
	glUseProgram(0);
	RE_DELETE(m_shMain);
	RE_DELETE(m_pDeform);
	RE_DELETE(m_pSkybox);
	RE_DELETE(m_pClipmap);
	RE_DELETE(m_pCaching);
	glDeleteBuffers(2, m_pbo);
	glDeleteTextures(1, &m_coarsemap.heightmap);
	glDeleteTextures(1, &m_coarsemap.normalmap);
	glDeleteTextures(1, &m_coarsemap.tangentmap);
	glDeleteTextures(1, &m_colormap_tex);
}

//--------------------------------------------------------
bool
DefTer::InitGL()
{
	int res;
	char* shVersion;
	int nVertTexUnits, nGeomTexUnits, nTexUnits, nTexUnitsCombined, nColorAttachments, nTexSize,
		nTexLayers, nVertAttribs, nGeomVerts, nGeomComponents;
	float aspect;

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

	printf("Setting up GL...");
	glEnable(GL_CULL_FACE);
	glClearColor(.4f, .4f,1.0f, 1.0f);
	if (!CheckError(""))
		return false;
	printf("\t\tDone\n");

	// Init projection matrix
	aspect			= float(m_config.winWidth) / float(m_config.winHeight);
	m_proj_mat		= perspective_proj(PI*.5f, aspect, NEAR_PLANE, FAR_PLANE);

	// Set the initial stamp mode and clicked state
	m_is_hd_stamp	= false;
	m_clicked		= false;

	// Init Shaders
	// Get the Shaders to Compile
	m_shMain		= new ShaderProg("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_TexCoord");

	// NB. must be done after binding attributes
	printf("Compiling shaders...");
	res = m_shMain->CompileAndLink();
	if (!res)
	{
		printf("\t\tError\n\tWill not continue without working shaders\n");
		return false;
	}

	// Assign samplers to texture units
	glUseProgram(m_shMain->m_programID);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "heightmap"), 0);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "normalmap"), 1);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "colormap"),  2);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "projection"), 1, GL_FALSE,	m_proj_mat.m);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "hd_aura"), HD_AURA);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "is_hd_stamp"), (m_is_hd_stamp ? 1.0f : 0.0f));

	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("\t\tDone\n");

	if (!Init())
		return false;
	
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
	"R-Mouse\t"	"= Pick Deform location\n"
	"L-Mouse\t" "= Rotate Camera\n"
	"Wheel\t"	"= Deform\n"
	"F12\t"		"= Screenshot\n"
	"Esc\t"		"= Quit\n"
	);

	//Initialise FreeImage
	FreeImage_Initialise();

	return true;
}

//--------------------------------------------------------
bool
DefTer::Init()
{
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

	// Create the clipmap
	printf("Creating clipmap...\t\t");
	m_pClipmap = new Clipmap(CLIPMAP_DIM, CLIPMAP_RES, CLIPMAP_LEVELS, m_coarsemap_dim);
	printf("Done\n");
	printf("Initialising clipmap...\t\t");
	m_pClipmap->init();
	printf("Done\n");

	// Create the deformer object
	printf("Creating deformer...\t\t");
	m_pDeform = new Deform(m_coarsemap_dim, HIGH_DIM, m_pClipmap->m_metre_to_tex, 1.0f/(HIGH_DIM * HIGH_RES));
	if (!m_pDeform->m_no_error)
	{
		fprintf(stderr, "Error\n\tCould not create deformer\n");
		return false;
	}
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

	// Create Caching System
	printf("Creating caching system...\t");
	m_pCaching = new Caching(m_pDeform, CACHING_DIM, m_coarsemap_dim, CLIPMAP_RES, HIGH_DIM, HIGH_RES);
	printf("Done\n");

	// Shader uniforms
	glUseProgram(m_shMain->m_programID);
	glUniform2f(glGetUniformLocation(m_shMain->m_programID, "scales"), m_pClipmap->m_tex_to_metre, m_pClipmap->m_metre_to_tex);
	
	// Generate the normal map and run a zero deform to init shaders
	printf("Creating initial deform...\t");
	m_pDeform->create_normalmap(m_coarsemap, true);
	m_pDeform->displace_heightmap(m_coarsemap, vector2(0.5f), 1.0f, .0f, true);
	if (!CheckError("Creating initial deform"))
		return false;
	printf("Done\n");

// Ouput normalmap
	/*/
	GLubyte *data  = new GLubyte[m_coarsemap_dim * m_coarsemap_dim * 3];
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.normalmap);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	SavePNG("coarsenormal.png", data, 8, 3, m_coarsemap_dim, m_coarsemap_dim);
	delete [] data;
// Output tangentmap
	data  = new GLubyte[m_coarsemap_dim * m_coarsemap_dim * 3];
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.tangentmap);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	SavePNG("coarsetangent.png", data, 8, 3, m_coarsemap_dim, m_coarsemap_dim);
	delete [] data;
	/*/
	return true;
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

	if (bitdepth!=8)
	{
		fprintf(stderr, "Error\n\tCannot load files with more than 1 component: %s\n",
				filename.c_str());
		return false;
	}

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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RED,	GL_UNSIGNED_BYTE, bits);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Allocate GPU Texture space for normalmap
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_coarsemap.normalmap);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.normalmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Allocate GPU Texture space for tangentmap
	glGenTextures(1, &m_coarsemap.tangentmap);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.tangentmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);

	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
// This method will change the active stamp
void
DefTer::UpdateStamp(int stampID)
{
	if (stampID == 0)
		m_is_hd_stamp = false;
	else
		m_is_hd_stamp = true;

	// Update the click position
	UpdateClickPos();

	glUseProgram(m_shMain->m_programID);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "is_hd_stamp"), (m_is_hd_stamp ? 1.0f : 0.0f));
}

//--------------------------------------------------------
// Updates the click position such that it follows the camera in HD mode,
// Also will hide in shaders if no point is currently 'clicked'
void
DefTer::UpdateClickPos(void)
{
	vector2 temp(2.0f);
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

		// Create a temporary vector and convert clickPos into tex space
		temp  = m_clickPos * m_pClipmap->m_metre_to_tex;
		temp += vector2(0.5f);
	}

	// Pass to shader
	glUseProgram(m_shMain->m_programID);
	glUniform2f(glGetUniformLocation(m_shMain->m_programID, "click_pos"), temp.x, temp.y);
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
	}

	// Change the selected deformation location
	if (m_input.IsButtonPressed(3))
	{
		MousePos pos = m_input.GetMousePos();
		float val;
		float w = (float)m_config.winWidth;
		float h = (float)m_config.winHeight;
		float aspect = w / h;
		// Get value in z-buffer
		glReadPixels((GLint)pos.x, (GLint)(m_config.winHeight- pos.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &val);

		vector3 frag(pos.x, pos.y, val);
		// Derive inverse of view transform (could just use transpose of view matrix
		matrix4 inverse = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);

		// Request unprojected coordinate
		vector3 p = perspective_unproj_world(frag, w, h, NEAR_PLANE, FAR_PLANE, 1.0f, inverse);
		// Factor in the camera translation
		p += m_cam_translate;

		m_clickPos	= vector2(p.x, p.z);
		m_clicked	= true;

		//Update the clicked position in shaders, etc...
		UpdateClickPos();
	}

	// Change the selected deformation location
	if (m_clicked && wheel_ticks != 0)
	{
		if (m_is_hd_stamp)
			m_pCaching->DeformHighDetail(m_coarsemap, m_clickPos, .1f * wheel_ticks);
		else
			m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, 1.0f, .1f * wheel_ticks, true);
	}

	static bool wireframe = false;
	// Toggle wireframe
	if (m_input.WasKeyPressed(SDLK_l))
	{
		wireframe ^= true;
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Take screenshot
	static bool transferring  = false;
	static int index     = 0;
	static int nextIndex = 1;
	if (m_input.WasKeyPressed(SDLK_F12))
	{
		glReadBuffer(GL_FRONT);

		// Start reading into the PBO
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[index]);
		glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_BGR, GL_UNSIGNED_BYTE, 0);
		transferring = true;
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glReadBuffer(0);	
	}
	else if (transferring)
	{
		static int lastScreenshot = 1;
		char filename[256];
		GLubyte* framebuffer;

		// Read from the other PBO into the array
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[index]);
		framebuffer = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

		if (framebuffer)
		{
			mkdir("ScreenShots");
			sprintf(filename, "ScreenShots/screenshot%05d.png", lastScreenshot++);
			SavePNG(filename, framebuffer, 8, 3, SCREEN_W, SCREEN_H, false);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}
		else
		{
			printf("Screenshot Failed\n");
		}
		transferring = false;
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glReadBuffer(0);	
	}

	// Switch stamps
	if (m_input.WasKeyPressed(SDLK_h))
	{
		m_clicked = false;

		if (m_is_hd_stamp)
			UpdateStamp(0);
		else
			UpdateStamp(1);
	}

	// Controls to handle movement of the camera
	// Speed in m/s (average walking speed)
	float speed = 1.33f;
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		speed*=30.0f;
	if (m_input.IsKeyPressed(SDLK_w))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(.0f, .0f, -speed) * dt;
		UpdateClickPos();
	}
	if (m_input.IsKeyPressed(SDLK_s))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(.0f, .0f, speed) * dt;
		UpdateClickPos();
	}
	if (m_input.IsKeyPressed(SDLK_a))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(-speed, .0f, .0f) * dt;
		UpdateClickPos();
	}
	if (m_input.IsKeyPressed(SDLK_d))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(speed, .0f, .0f) * dt;
		UpdateClickPos();
	}
	// Boundary check for wrapping position
	static float boundary = m_coarsemap_dim* m_pClipmap->m_quad_size;
	m_cam_translate.x = WRAP_POS(m_cam_translate.x, boundary);
	m_cam_translate.z = WRAP_POS(m_cam_translate.z, boundary);
	
	// Toggle Frustum Culling
	if (m_input.WasKeyPressed(SDLK_c))
	{
		m_pClipmap->m_enabled ^= true;
		printf("Frustum Culling Enabled: %d\n", int(m_pClipmap->m_enabled));
	}

	reGL3App::ProcessInput(dt);
}

//--------------------------------------------------------
void
DefTer::Logic(float dt)
{
	//Update the caching system
	m_pCaching->Update(vector2(m_cam_translate.x, m_cam_translate.z));

	// Update position
	vector3 pos = m_cam_translate * m_pClipmap->m_metre_to_tex;
	glUseProgram(m_shMain->m_programID);

	// pass the camera's texture coordinates and the shift amount necessary
	// cam = x and y   ;  shift = z and w
	m_clipmap_shift.x = -fmodf(m_cam_translate.x, 32*m_pClipmap->m_quad_size);
	m_clipmap_shift.y = -fmodf(m_cam_translate.z, 32*m_pClipmap->m_quad_size);
	
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
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.normalmap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_colormap_tex);
	
	// Use the shader program and set the view matrix.
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "view"), 1, GL_FALSE, rotate.m);

	// Cull invisible blocks and render clipmap
	m_pClipmap->cull(viewproj, m_clipmap_shift);
	m_pClipmap->render();
	
	m_pSkybox->render(viewproj);

	SDL_GL_SwapWindow(m_pWindow);
}
