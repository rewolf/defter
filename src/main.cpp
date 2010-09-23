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

#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_shader.h"
#include "deform.h"
#include "skybox.h"
#include "main.h"
#include <vector>
#include <SDL/SDL_image.h>
#include <limits.h>
#include "util.h"

#define PI					(3.14159265358f)
#define FAR_PLANE			(1000.0f)
#define NEAR_PLANE			(.5f)



/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[]){
	AppConfig conf;
	conf.VSync = false;
	conf.gl_major = 3;
	conf.gl_minor = 2;
	conf.fsaa=4;
	conf.sleepTime = .0f;
	conf.winWidth = 1024;
	conf.winHeight= 768;
	DefTer test(conf);
	
	if (!test.Start())
		printf("Application failed to start\n");
#ifdef _WIN32
	Sleep(10000);
#endif
	return 0;
}


//--------------------------------------------------------
DefTer::DefTer(AppConfig& conf) : reGL3App(conf){
	m_shMain = NULL;
	m_deform = NULL;
}

//--------------------------------------------------------
DefTer::~DefTer(){
	glUseProgram(0);
	RE_DELETE(m_shMain);
	RE_DELETE(m_deform);
	RE_DELETE(m_pSkybox);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(3, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteTextures(1, &m_heightmap_tex);
	glDeleteTextures(1, &m_normalmap_tex);
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
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "colormap"),2);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "projection"), 1, GL_FALSE,	m_proj_mat.m);

	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("done\n");

	printf("creating geometry...\n");
	if (!Init())
		return false;
	printf("done\n");
	return true;
}

//--------------------------------------------------------
bool
DefTer::Init(){
	vector3* vertices;
	float2*	 texcoords;
	GLuint* indices;
	int i,j,x,z;
	int nVerts;
	int w,h;


	// Load heightmap
	printf("\tloading heightmap...\n");
	if (!LoadHeightmap("images/hmap01.png"))
		return false;
	if (!LoadTexturePNG(&m_colormap_tex, &w, &h, "images/hmap01_texture.png"))
		return false;
	printf("\tdone\n");

	CreateClipmap(&vertices, &texcoords, &indices, &nVerts, &m_nIndices);

	// Create the vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(3, m_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVerts, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * nVerts, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_nIndices, indices, GL_STATIC_DRAW);

	delete[] vertices;
	delete[] texcoords;
	delete[] indices;

	// Create the deformer object
	m_deform = new Deform(&m_heightmap_tex, &m_normalmap_tex, m_heightmap_width, m_heightmap_height);
	m_pSkybox = new Skybox();

	if (!m_deform->m_no_error){
		fprintf(stderr, "Could not create deformer\n");
	}
	if (!m_pSkybox->m_no_error){
		fprintf(stderr, "Could not create skybox\n");
	}

	// Generate the normal map
	float2 t = {.0, .0};
	m_deform->displace_heightmap(t, .0f);

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
	m_heightmap_width 	= surface->w;
	m_heightmap_height	= surface->h;
	bpp 	= surface->format->BytesPerPixel;

	if (bpp!=1){
		fprintf(stderr, "Cannot load heightmap files with more than 1 component: %s\n",
				filename.c_str());
		return false;
	}


	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_heightmap_tex);
	glBindTexture(GL_TEXTURE_2D, m_heightmap_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Loading PNG heightmap, setting parameters")){
		fprintf(stderr, "file: %s\n", filename.c_str());
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_heightmap_width, m_heightmap_height, 0, GL_RED,
			GL_UNSIGNED_BYTE, surface->pixels);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);

	// Allocate GPU Texture space for normalmap
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_normalmap_tex);
	glBindTexture(GL_TEXTURE_2D, m_normalmap_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, m_heightmap_width, m_heightmap_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	if (m_input.IsButtonPressed(1)){
		//pitch
		m_cam_rotate.x += dt*move.y*PI*.1f;
		//yaw
		m_cam_rotate.y += dt*move.x*PI*.1f;
	}
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
		p *= m_scale_metreToTex;
		float2 tc = {p.x+.5f, p.z+.5f};
		tc.u = tc.u > 1.0f ? tc.u - int(tc.u) : (tc.u < .0f ? tc.u + 1 - int(tc.u) : tc.u);
		tc.v = tc.v > 1.0f ? tc.v - int(tc.v) : (tc.v < .0f ? tc.v + 1 - int(tc.v) : tc.v);

		clicked = true;
		clickPos = tc;

		// pass to shader
		glUseProgram(m_shMain->m_programID);
		glUniform2f(glGetUniformLocation(m_shMain->m_programID, "click_pos"), tc.u, tc.v);
	}


	if (clicked && wheel_ticks!=0){
		m_deform->displace_heightmap(clickPos, .1f * wheel_ticks);
	}	
	
	// Toggle mouse grabbinga
	if (m_input.WasKeyPressed(SDLK_m)){
		printf("Toggle\n");
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(0);
	}

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


	float speed = .1*5.0f;		// in mps
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

	reGL3App::ProcessInput(dt);
}

//--------------------------------------------------------
void 
DefTer::Logic(float dt){
	// Update position
	vector3 pos = m_cam_translate * m_scale_metreToTex;
	glUseProgram(m_shMain->m_programID);

	// pass the camera's texture coordinates and the shift amount necessary
	// cam = x and y   ;  shift = z and w

	glUniform4f(glGetUniformLocation(m_shMain->m_programID, "cam_and_shift"), 
			pos.x,
			pos.z, 
			-fmodf(m_cam_translate.x, 32*m_quadSize), 
			-fmodf(m_cam_translate.z, 32*m_quadSize));
}

//--------------------------------------------------------
void
DefTer::Render(float dt){
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	matrix4 rotate;

	rotate = rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f) * rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);

	glBindVertexArray(m_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_heightmap_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalmap_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_colormap_tex);
	// Use the shader program and setup uniform variables.
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "view"), 1, GL_FALSE,
			(rotate).m);
	// Depending on state, draw triangle or quad patch
	static reTimer timer;
	//glFinish();
	//timer.start();
	
	glDrawElements(GL_TRIANGLES, m_nIndices, GL_UNSIGNED_INT, 0);
	//glFinish();
	//printf("render:%.3f\n", timer.getElapsed());
	
	matrix4 tr = m_proj_mat * rotate;
	m_pSkybox->render(tr);

	SDL_GL_SwapWindow(m_pWindow);
}

//--------------------------------------------------------
void
DefTer::CreateClipmap(vector3** outVerts, float2** outTex, GLuint **outInds, 
		int *nVerts, int *nIndices){
	int i,j,k,m,n,p;
	float quadSize, texelSize, left, ffar, zbase;
	std::vector<vector3> vertices;
	std::vector<float2> texcoords;
	std::vector<GLuint> indices;

	//////////////////////////////////////////////////
	// Just some scaling settings
	// 1 unit = 1 metre
	// NB The finest clip level quads should match texel resolution
	// => texelsize = 1.0/texture_dimension
	// The size of the finest quads will determine the physical area a heightmap will represent

	// Initialize camera start position
	m_cam_translate.x = .0f;
	m_cam_translate.z = .0f;

	// Clipmap settings
	p			= 5; 			// # of levels
	n			= 255; 		// # of vertices per side of ring for a given level
	m			= (n+1)/4;	// block size
	m_quadSize 	=
	quadSize 	= .1f;
	texelSize	= 1.0f/(m_heightmap_width+1);

	m_scale_metreToTex = texelSize/quadSize;

	// Tell shader of the tex coord increments
	glUseProgram(m_shMain->m_programID);
	//glUniform1f(glGetUniformLocation(m_shMain->m_programID, "texToMetre"), 1.0f/m_scale_metreToTex);
	//glUniform1f(glGetUniformLocation(m_shMain->m_programID, "metreToTex"), m_scale_metreToTex);
	glUniform2f(glGetUniformLocation(m_shMain->m_programID, "scales"), 1.0f/m_scale_metreToTex, m_scale_metreToTex);
	////////////////////////////////////////////////////

	printf("\tClipmap levels:\t\t%d\n",p);
	printf("\tFinest quad size:\t%.3fm\n",quadSize);
	printf("\tVertices per ring-side:\t%d\n",n);
	printf("\tSample distance:\t%.3f\n",texelSize);
	printf("\tEffective heightmap size:\t%.2f\n", m_heightmap_width * quadSize);


	int vcount = 0, vrow = n,offset =0;

	// Create the inner grid
	left = - (n-1) * .5f;
	ffar  = - (n-1) * .5f;
	for (i = 0; i < n;i++){
		for (j = 0; j < n; j++){
			vector3 v;
			float2 tc;
			v.x = left*quadSize + j*quadSize;
			v.y = ffar*quadSize + i*quadSize;
			
			// lod mipmap
			if (i==0 || i==n-1 || j==0 || j==n-1)
				v.z = 1.0f;
			else if (i==1 || i==n-2 || j==1 || j==n-2)
				v.z = .5f;
			else
				v.z = .0f;

			tc.u= 0.5 + left*texelSize + j*texelSize;
			tc.v= 0.5 + ffar*texelSize + i*texelSize;;
			vertices.push_back(v);
			texcoords.push_back(tc);
			if (i>0 && j>0){
				indices.push_back( vcount-1 );
				indices.push_back( vcount-vrow );
				indices.push_back( vcount-vrow-1 );

				indices.push_back( vcount-vrow );
				indices.push_back( vcount-1 );
				indices.push_back( vcount );
			}
			vcount++;
		}
	}

	// Create the LOD regions
	for (i=0; i < p; i++){
		vrow = n;
		offset =0;
		quadSize*=2;
		texelSize*=2;
		left = left*.5f - (m-1 + i%2);
		ffar  = ffar*.5f - (m-1 + (i+1)%2);
		zbase = ffar;
		// [1.]
		// two mxm blocks + mx3 fixup + two mxm blocks
		//   add the L-strip on alternating corners depending on the oddness of i
		for (j = 0; j < m + (i+1)%2; j++){
			for (k = 0; k < vrow; k++){
				vector3 v;
				float2 tc;
				v.x = left*quadSize + k*quadSize;
				v.y = ffar*quadSize + j*quadSize;
				// lod mipmap
				if (k==0 || j==0 || k==vrow-1)
					v.z = float(i+2);
				else if (j==1 || k==1 || k==vrow-2)
					v.z = .5f*(2*i+3);
				else
					v.z = float(i+1);
				tc.u= 0.5 + (left+k) * texelSize;
				tc.v= 0.5 + (ffar+j) * texelSize;
				vertices.push_back(v);
				texcoords.push_back(tc);
				if (k>0 && j>0){
					indices.push_back( vcount-1 );
					indices.push_back( vcount-vrow );
					indices.push_back( vcount-vrow-1 );

					indices.push_back( vcount-vrow );
					indices.push_back( vcount-1 );
					indices.push_back( vcount );
				}
				vcount++;
			}
			zbase += 1;
		}

		// [2.]
		// Now make rows down the sides of the interior ring
		int gaprows = (m-1)*2+2-1;
		for (j = 0; j < gaprows; j++){
			// quads left of gap
			for (k = 0; k < m + i%2; k++){
				vector3 v;
				float2 tc;
				v.x = left*quadSize + k*quadSize;
				v.y = zbase*quadSize + j*quadSize;
				if (k==0)
					v.z = float(i+2);
				else if (k==1)
					v.z = .5f * (2*i+3);
				else
					v.z = float(i+1);
				tc.u= 0.5 + (left+k) * texelSize;
				tc.v= 0.5 + (zbase+j) * texelSize;
				vertices.push_back(v);
				texcoords.push_back(tc);
				if (k>0){
					indices.push_back( vcount-1 );
					indices.push_back( vcount-vrow );
					indices.push_back( vcount-vrow-1 );

					indices.push_back( vcount-vrow );
					indices.push_back( vcount-1 );
					indices.push_back( vcount );
				}
				vcount++;
			}
			
			offset = m + i%2;
			// do the degenerate triangle on left side of the gap
			{
				vector3 vm;	// create a vertex in the middle of the right edge
				float2 tcm;
				vm.x = left*quadSize + (offset-1)*quadSize;
				vm.y = zbase*quadSize + (j-.5f)*quadSize;
				vm.z = float(i+1);
				tcm.u = 0.5 + (left + offset -1) * texelSize;
				tcm.v = 0.5 + (zbase + j - .5f)*texelSize;
				// Add midpoint 
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				if (j==0)
					vrow = n; 
				else if (j==1)
					vrow = n+3;
				else
					vrow = 2*m+3 ;

				indices.push_back( vcount-1 );
				indices.push_back( vcount );
				indices.push_back( vcount-vrow-1 );

				vcount++; // increment for this corner vertex
			}
			// If its the first row of the gap, create degenerate triangles at the top
			if (j==0){
				for (int d= 0; d < 2*(m-1)+1; d++){
					vector3 vm;	// create a vertex in the middle of the right edge
					float2 tcm;
					vm.x = left*quadSize + (d+offset-.5f)*quadSize;
					vm.y = zbase*quadSize + (j-1)*quadSize;
					vm.z = float(i+1);
					tcm.u = 0.5 + (left + d + offset - .5f) * texelSize;
					tcm.v = 0.5 + (zbase + j - 1) * texelSize;
					// Add midpoint
					vertices.push_back(vm);
					texcoords.push_back(tcm);

					vrow = n+1; // normal row + extra vert (for the two in the top left quad)

					indices.push_back( vcount-vrow );
					indices.push_back( vcount-vrow-1 );
					indices.push_back( vcount );

					vcount++; // increment for this corner vertex

				}
			}
			else if (j==gaprows-1){ // last row degens
				for (int d= 0; d < 2*(m-1)+1; d++){
					vector3 v,vm;	// create a vertex in the middle of the right edge
					float2 tc,tcm;
					v.x = left*quadSize + (d+offset)*quadSize;
					v.y = zbase*quadSize + j*quadSize;
					v.z = float(i+1);
					tc.u= 0.5 + (left + d + offset) * texelSize;
					tc.v= 0.5 + (zbase + j) * texelSize;
					// midpoint
					vm.x = v.x - .5f*quadSize;
					vm.y = v.y;
					vm.z = float(i+1);
					tcm.u = tc.u - .5f*texelSize;
					tcm.v = tc.v;
					// Add midpoint
					vertices.push_back(vm);
					texcoords.push_back(tcm);
					vertices.push_back(v);
					texcoords.push_back(tc);
					vcount++;

					vrow = 2*m+3 ; // normal row + extra vert (for the two in the top left quad)

					indices.push_back( vcount );
					indices.push_back( vcount-1 );
					if (d==0)
						indices.push_back( vcount-3 );
					else
						indices.push_back( vcount-2 );

					vcount++; // increment for this corner vertex

				}
			}
			
			// do the degen triangle on the right side of the gap
			{
				vector3 v, vm;	// create a vertex in the middle of the left edge
				float2 tc, tcm;
				v.x = left*quadSize + (2*(m-1)+1+offset-1)*quadSize;
				v.y = zbase*quadSize + j*quadSize;
				v.z = float(i+1);
				tc.u= 0.5 + (left + 2*(m-1) + 1 + offset - 1) * texelSize;
				tc.v= 0.5 + (zbase + j) * texelSize;
				// Create the midpoint between this vertex and the previous
				vm.x= v.x;
				vm.y= v.y - .5f*quadSize;
				vm.z = float(i+1);
				tcm.u = tc.u;
				tcm.v = tc.v - .5*texelSize;
				// Add midpoint and bottom corners
				vertices.push_back(vm);
				texcoords.push_back(tcm);
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++; // increment for the left and midpoint verts

				// make degen tri
				if (j==0)
					vrow = n + 3 ;
				else if (j==gaprows-1)
					vrow = 6*m+1;
				else	
					vrow = 2*m + 3;

				indices.push_back( vcount);
				indices.push_back( vcount- vrow);
				indices.push_back( vcount-1 );
				
				vcount++; // increment for the right corner vertex
			}

			// finish row with (n-1) normal quads
			offset += 2*(m-1)+1;

			for (k = 0; k < m-1+(i+1)%2; k++){
				vector3 v;
				float2 tc;
				v.x = left*quadSize + (k+offset)*quadSize;
				v.y = zbase*quadSize + (j)*quadSize;
				if (k == m-1+(i+1)%2-1)
					v.z = float(i+2);
				else if (k == m-1+(i+1)%2-2)
					v.z = .5f * (2*i+3);
				else
					v.z = float(i+1);
				tc.u= 0.5 + (left + k + offset) * texelSize;
				tc.v= 0.5 + (zbase + j) * texelSize;
				vertices.push_back(v);
				texcoords.push_back(tc);

				indices.push_back( vcount-1 );
				indices.push_back( vcount-vrow );
				indices.push_back( vcount-vrow-1 );

				indices.push_back( vcount-vrow );
				indices.push_back( vcount-1 );
				indices.push_back( vcount );

				vcount++;

			}
			
		}

		zbase += gaprows;

		// [3.]
		// Now do the same as the top but in reverse
		// make the rows below the gap
		for (j = 0; j <  m-1 + i%2; j++){
			for (k = 0; k < n; k++){
				vector3 v;
				float2 tc;
				v.x = left*quadSize + k*quadSize;
				v.y = zbase*quadSize + j*quadSize;
				if (k==0 || j==m-1+i%2-1 || k == n-1)
					v.z = float(i+2);
				else if (k==1 || j==m-1+i%2-2 || k == n-2)
					v.z = .5f * (2*i + 3);
				else
					v.z = float(i+1);
				tc.u= 0.5 + (left + k)*texelSize;
				tc.v= 0.5 + (zbase + j)*texelSize;
				vertices.push_back(v);
				texcoords.push_back(tc);

				if (j==0 && k>0 && k <= m+2*(m-1)+i%2){
					if (k<m+i%2){
						indices.push_back( vcount-1 );
						indices.push_back( vcount-vrow );
						indices.push_back( vcount-vrow-1 );

						indices.push_back( vcount-vrow );
						indices.push_back( vcount-1 );
						indices.push_back( vcount );
					}
					else if (k==m+i%2){
						vrow-=2;
						indices.push_back( vcount-1 );
						indices.push_back( vcount-vrow );
						indices.push_back( vcount-vrow-3);

						indices.push_back( vcount-vrow );
						indices.push_back( vcount-1 );
						indices.push_back( vcount );
						vrow-=1;
					}
					else if (k < m+2*(m-1)+i%2){
						vrow-=1;
						indices.push_back( vcount-1 );
						indices.push_back( vcount-vrow -1);
						indices.push_back( vcount-vrow-3);

						indices.push_back( vcount-vrow-1 );
						indices.push_back( vcount-1 );
						indices.push_back( vcount );
					}
					else if (k== m+2*(m-1)+i%2){
						vrow-=1;
						indices.push_back( vcount-1 );
						indices.push_back( vcount-vrow -1);
						indices.push_back( vcount-vrow-3);

						indices.push_back( vcount-vrow-1 );
						indices.push_back( vcount-1 );
						indices.push_back( vcount );
						vrow--;
					}

				}
				else{
					if (k>0){
						indices.push_back( vcount-1 );
						indices.push_back( vcount-vrow );
						indices.push_back( vcount-vrow-1 );

						indices.push_back( vcount-vrow );
						indices.push_back( vcount-1 );
						indices.push_back( vcount );
					}
				}
				vcount++;
			}
		}
	}
	
	printf("\tFurthest point:\t\t%.2f\n",	-ffar*quadSize);
	printf("\tVertex count:\t\t%d\n",(int)vertices.size());
	printf("\tIndex count:\t\t%d\n",(int)indices.size());
	printf("\tPrimitive count:\t%d\n",(int)indices.size()/3);
	*outVerts = new vector3[vertices.size()];
	*outInds  = new GLuint[indices.size()];
	*outTex	  = new float2[texcoords.size()];
	*nVerts   = vertices.size();
	*nIndices = indices.size();
	memcpy(*outVerts, &vertices[0], sizeof(vector3)*(*nVerts));
	memcpy(*outInds, &indices[0], sizeof(GLuint)*(*nIndices));
	memcpy(*outTex, &texcoords[0], sizeof(float2)*(*nVerts));
}
