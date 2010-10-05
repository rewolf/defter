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
#endif

class Deform;

#include <vector>
#include <limits.h>
#include "regl3.h"
#include <SDL/SDL_image.h>
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

#define HIGH_DIM			(2048)
#define CLIPMAP_DIM			(255)
#define CLIPMAP_RES			(.1f)
#define CLIPMAP_LEVELS		(5)
#define HIGH_RES			(CLIPMAP_RES/3.0f)


/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[]){
	AppConfig conf;
	conf.VSync = false;
	conf.gl_major = 3;
	conf.gl_minor = 2;
	conf.fsaa=0;
	conf.sleepTime = .0f;
	conf.winWidth = 1024;
	conf.winHeight= 768;
	DefTer test(conf);
	
	int sleepTime = 1000;
	if (!test.Start())
	{
		printf("Application failed to start\n");
		sleepTime *= 10;
	}

#ifdef _WIN32
	Sleep(sleepTime);
#endif
	return 0;
}


//--------------------------------------------------------
DefTer::DefTer(AppConfig& conf) : reGL3App(conf){
	m_shMain  = NULL;
	m_pDeform = NULL;
	m_pClipmap= NULL;
	m_pCaching= NULL;
}

//--------------------------------------------------------
DefTer::~DefTer(){
	glUseProgram(0);
	RE_DELETE(m_shMain);
	RE_DELETE(m_pDeform);
	RE_DELETE(m_pSkybox);
	RE_DELETE(m_pClipmap);
	RE_DELETE(m_pCaching);
	glDeleteTextures(1, &m_coarsemap.heightmap);
	glDeleteTextures(1, &m_coarsemap.normalmap);
	glDeleteTextures(1, &m_coarsemap.tangentmap);
	glDeleteTextures(1, &m_colormap_tex);
}

//--------------------------------------------------------
bool
DefTer::InitGL(){
	int res;
	char* shVersion;
	int nVertTexUnits, nGeomTexUnits, nTexUnits, nTexUnitsCombined, nColorAttachments, nTexSize,
		nTexLayers, nVertAttribs, nGeomVerts, nGeomComponents;
	float aspect;

	if (!reGL3App::InitGL())
		return false;
#ifdef _WIN32
	if (glewInit()!=GLEW_OK){
		printf("Could not init GLEW\n");
		return false;
	}
#endif

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

	printf("-----------------------------------------\n");


	glEnable(GL_CULL_FACE);
	glClearColor(.4f, .4f,1.0f, 1.0f);

	if (!CheckError("Enabling GL settings"))
		return false;

	// init projection matrix
	aspect = float(m_config.winWidth)/m_config.winHeight;
	m_proj_mat = perspective_proj(PI*.5f, aspect, NEAR_PLANE, FAR_PLANE);

	// Init Camera
	//m_cam_rotate.y = PI*.5f;

	// Init Shaders
	// Get the Shaders to Compile
	m_shMain = new ShaderProg("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_TexCoord");

	// NB. must be done after binding attributes
	printf("compiling shaders...\n");
	res = m_shMain->CompileAndLink();
	if (!res){
		printf("Will not continue without working shaders\n");
		return false;
	}

	// Assign samplers to texture units
	glUseProgram(m_shMain->m_programID);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "heightmap"),0);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "normalmap"),1);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "colormap")	,2);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "projection"), 1, GL_FALSE,	m_proj_mat.m);

	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("done\n");

	printf("creating geometry...\n");
	if (!Init())
		return false;
	printf("done\n");
	
	// Print key commands
	printf(
	"==================\n"
	"  Key Controls:\n"
	"==================\n"
	"w,a,s,d\t"	"= Camera Translation\n"
	"l\t"		"= Lines/Wireframe Toggle\n"
	"c\t"		"= En/Disable Frustum Culling\n"
	"R-Mouse\t"	"= Pick Deform location\n"
	"L-Mouse\t" "= Rotate Camera\n"
	"Wheel\t"	"= Deform\n"
	"==================\n"
	);
	return true;
}

//--------------------------------------------------------
bool
DefTer::Init(){
	int w,h;


	// Load heightmap
	printf("\tloading heightmap...\n");
	if (!LoadHeightmap("images/hmap02.png"))
		return false;
	if (!LoadTexturePNG(&m_colormap_tex, &w, &h, "images/hmap01_texture.png"))
		return false;
	printf("\tdone\n");

	// Create the deformer object and skybox
	m_pDeform = new Deform(m_coarsemap_dim, HIGH_DIM);
	m_pSkybox = new Skybox();

	if (!m_pDeform->m_no_error){
		fprintf(stderr, "Could not create deformer\n");
	}
	if (!m_pSkybox->m_no_error){
		fprintf(stderr, "Could not create skybox\n");
	}

	// Create Caching System
	m_pCaching  = new Caching(m_pDeform, CLIPMAP_DIM, m_coarsemap_dim, CLIPMAP_RES, HIGH_DIM, HIGH_RES);

	// Create the clipmap
	m_pClipmap	= new Clipmap(CLIPMAP_DIM, CLIPMAP_RES, CLIPMAP_LEVELS, m_coarsemap_dim);
	m_pClipmap->init();
	// Shader uniforms
	glUseProgram(m_shMain->m_programID);
	glUniform2f(glGetUniformLocation(m_shMain->m_programID, "scales"), 
			m_pClipmap->m_tex_to_metre, m_pClipmap->m_metre_to_tex);
	
	// Generate the normal map
	float2 centre = { .5,  .5};
	vector2 scale(1.0);

	m_pDeform->calculate_normals(m_coarsemap, centre, scale, true);
	m_pDeform->displace_heightmap(m_coarsemap, centre, .0f, true);

	return true;
}

//--------------------------------------------------------
// LOADTEXTURE loads a heightmap from a pgm file into the given texture name.  It assumes there are
// no comment lines in the file.  The texture has a byte per texel.
bool
DefTer::LoadHeightmap(string filename){
	SDL_Surface*  surface;
	int bpp;

	surface = IMG_Load(filename.c_str());
	if (surface == NULL){
		fprintf(stderr, "Could not load heightmap PNG %s: %s\n", filename.c_str(), IMG_GetError());
		return false;
	}

	// Get image details
	m_coarsemap_dim 	= surface->w;
	bpp 	= surface->format->BytesPerPixel;

	if (bpp!=1){
		fprintf(stderr, "Cannot load heightmap files with more than 1 component: %s\n",
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

	if (!CheckError("Loading PNG heightmap, setting parameters")){
		fprintf(stderr, "file: %s\n", filename.c_str());
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RED,
			GL_UNSIGNED_BYTE, surface->pixels);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);

	// Allocate GPU Texture space for normalmap
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_coarsemap.normalmap);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.normalmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Allocate GPU Texture space for tangentmap
	glGenTextures(1, &m_coarsemap.tangentmap);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.tangentmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	return true;
}

//--------------------------------------------------------
void
DefTer::ProcessInput(float dt){
	int wheel_ticks = m_input.GetWheelTicks();
	MouseDelta move = m_input.GetMouseDelta();
	static float2 clickPos;
	static bool clicked = false;

	// Rotate Camera
	if (m_input.IsButtonPressed(1)){
		//pitch
		m_cam_rotate.x += dt*move.y*PI*.1f;
		//yaw
		m_cam_rotate.y += dt*move.x*PI*.1f;
	}
	// Change the selected deformation location
	if (m_input.IsButtonPressed(3)){
		MousePos pos = m_input.GetMousePos();
		float val;
		float w = m_config.winWidth;
		float h = m_config.winHeight;
		float aspect = float(m_config.winWidth)/m_config.winHeight;
		// get value in z-buffer
		glReadPixels(pos.x, m_config.winHeight- pos.y, 1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &val);

		vector3 frag(pos.x,pos.y,val);
		// derive inverse of view transform (could just use transpose of view matrix
		matrix4 inverse = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);

		// request unprojected coordinate
		vector3 p = perspective_unproj_world(
				frag, w, h, NEAR_PLANE, FAR_PLANE, 1.0f, inverse);
		p += m_cam_translate;
		p *= m_pClipmap->m_metre_to_tex;
		float2 tc = {p.x+.5f, p.z+.5f};
	//	tc.u = tc.u > 1.0f ? tc.u - int(tc.u) : (tc.u < .0f ? tc.u + 1 - int(tc.u) : tc.u);
	//	tc.v = tc.v > 1.0f ? tc.v - int(tc.v) : (tc.v < .0f ? tc.v + 1 - int(tc.v) : tc.v);

		clicked = true;
		clickPos = tc;

		// pass to shader
		glUseProgram(m_shMain->m_programID);
		glUniform2f(glGetUniformLocation(m_shMain->m_programID, "click_pos"), tc.u, tc.v);
	}

	// Change the selected deformation location
	if (clicked && wheel_ticks!=0)
		m_pDeform->displace_heightmap(m_coarsemap, clickPos, .1f * wheel_ticks, true);

	static bool wireframe = false;
	// Toggle wireframe
	if (m_input.WasKeyPressed(SDLK_l)){
		wireframe^=true;
		if (wireframe){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}


	float speed = 1.33f;		// in m/s (average walking speed)
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		speed*=30.0f;
	if (m_input.IsKeyPressed(SDLK_w)){
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(.0f, .0f, -speed) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_s)){
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(.0f, .0f, speed) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_a)){
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(-speed, .0f, .0f) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_d)){
		matrix4 rot = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
		m_cam_translate += rot * vector3(speed, .0f, .0f) * dt;
	}
	static float boundary = m_coarsemap_dim* m_pClipmap->m_quad_size;
	m_cam_translate.x = WRAP_POS(m_cam_translate.x, boundary);
	m_cam_translate.z = WRAP_POS(m_cam_translate.z, boundary);
	
	// Toggle Frustum Culling
	if (m_input.WasKeyPressed(SDLK_c)){
		m_pClipmap->m_enabled ^= true;
		printf("Frustum Culling Enabled: %d\n", int(m_pClipmap->m_enabled));
	}

	m_pCaching->Update(vector2(m_cam_translate.x, m_cam_translate.z));
	reGL3App::ProcessInput(dt);
}

//--------------------------------------------------------
void 
DefTer::Logic(float dt){
	// Update position
	vector3 pos = m_cam_translate * m_pClipmap->m_metre_to_tex;
	glUseProgram(m_shMain->m_programID);

	// pass the camera's texture coordinates and the shift amount necessary
	// cam = x and y   ;  shift = z and w

	m_clipmap_shift.x = -fmodf(m_cam_translate.x, 32*m_pClipmap->m_quad_size);
	m_clipmap_shift.y = -fmodf(m_cam_translate.z, 32*m_pClipmap->m_quad_size);
	
	glUniform4f(glGetUniformLocation(m_shMain->m_programID, "cam_and_shift"), 
			pos.x,	pos.z, 	m_clipmap_shift.x, 	m_clipmap_shift.y);
}

//--------------------------------------------------------
void
DefTer::Render(float dt){
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

