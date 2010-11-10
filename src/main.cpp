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

class Deform;

#include "constants.h"
using namespace std;
#include "deform.h"
#include "clipmap.h"
#include "skybox.h"
#include "caching.h"
#include "main.h"

/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[])
{
	AppConfig conf;
	conf.VSync		= VSYNC;
	conf.gl_major	= 3;
	conf.gl_minor	= 2;
	conf.fsaa		= FSAA;
	conf.sleepTime	= SLEEP_TIME;
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

	PRINT_PROF;

	return 0;
}


//--------------------------------------------------------
DefTer::DefTer(AppConfig& conf) : reGL3App(conf)
{
	m_shManager			  = NULL;
	m_pDeform 			  = NULL;
	m_pClipmap			  = NULL;
	m_pCaching			  = NULL;
	m_pSkybox 			  = NULL;
	m_elevationData		  = NULL;
	m_elevationDataBuffer = NULL;
}

//--------------------------------------------------------
DefTer::~DefTer()
{
	// signal thread to check "isRunning" status
	SDL_SemPost(m_waitSem);
	SaveCoarseMap("images/last_shit_coarsemap.png");
	FreeImage_DeInitialise();
	glUseProgram(0);
	KillUtil();
	RE_DELETE(m_shManager);
	RE_DELETE(m_pDeform);
	RE_DELETE(m_pSkybox);
	RE_DELETE(m_pClipmap);
	RE_DELETE(m_pCaching);
	if (m_elevationData)
		delete [] m_elevationData;
	if (m_elevationDataBuffer)
		delete [] m_elevationDataBuffer;
	glDeleteBuffers(NUM_PBOS, m_pbo);
	glDeleteFramebuffers(1,&m_fboTransfer);
	glDeleteFramebuffers(1,&m_screenshotFBO);
	glDeleteTextures(1, &m_coarsemap.heightmap);
	glDeleteTextures(1, &m_coarsemap.pdmap);
	glDeleteTextures(1, &m_colormap_tex);
	glDeleteTextures(1, &m_screenshotTex);
	glDeleteRenderbuffers(1, &m_screenshotDepth);
	SDL_DestroyMutex(m_elevationDataMutex);
	SDL_DestroySemaphore(m_waitSem);
}

//--------------------------------------------------------
bool
DefTer::InitGL()
{
	bool error;
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
	glClearColor(0.0f, 0.0f, .0f, 1.0f);
	if (!CheckError(""))
		return false;
	printf("Done\n");
	
	//Initialise FreeImage
	FreeImage_Initialise();

	// Init the splash screen info
	printf("Initialising Splash screen...\t");
	m_pSplash = new Splash();
	if (m_pSplash->HasError())
	{
		printf("Error\n\tSplash screen initialisation error\n");
		return false;
	}
	printf("Done\n");

	// Render the splash screen
	printf("Rendering splash screen...\t");
	m_pSplash->Render(m_pWindow);
	if (!CheckError("Splash rendering"))
		return false;
	printf("Done\n");

	// Init projection matrix
	m_proj_mat		= perspective_proj(PI*.5f, ASPRAT, NEAR_PLANE, FAR_PLANE);

	// Init the cameras position such that it is in the middle of a tile
	float halfTile  = HIGH_DIM * HIGH_RES * 0.5f;
	m_cam_translate.set(-halfTile, 0.0f, -halfTile);
	m_lastPosition  = m_cam_translate;

	// Set the initial stamp mode and clicked state
	m_stampIndex	= 0;
	m_stampSIRM		= vector4(20.0f, 0.2f, 0.0f, 0.0f);
	m_is_hd_stamp	= false;
	m_clicked		= false;
	m_clickPos		= vector2(0.0f);
	m_clickPosPrev	= vector2(0.0f);

	// Init the world settings
	m_gravity_on	= true;
	m_is_crouching	= false;
	m_hit_ground	= false;
	m_footprintDT	= 0.0f;
	m_flipFoot		= false;
	m_drawing_feet	= false;
	m_is_wireframe	= false;

	// Set initial shader index values
	m_shmSimple		= 0;
	m_shmParallax	= 0;
	m_shmGeomTess	= 0;

	// Init Shaders
	printf("Initialising Shader Manager...\t");
	m_shManager		 = new ShaderManager();
	error			 = !m_shManager->AddShader("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag", &m_shmSimple);
	error			&= !m_shManager->AddShader("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag", &m_shmParallax);
	error			&= !m_shManager->AddShader("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag", &m_shmGeomTess);
	m_hdShaderIndex	 = m_shmSimple;

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	m_shManager->BindAttrib("vert_Position", 0);
	m_shManager->BindAttrib("vert_TexCoord", 1);
	if (error || !CheckError("Binding shader attributes"))
	{
		printf("Error\n\tError adding shaders to shader manager\n");
		return false;
	}
	printf("Done\n");

	// NB. must be done after binding attributes
	printf("Compiling shaders...\t\t");
	if (!m_shManager->CompileAndLink())
	{
		printf("Error\n\tWill not continue without working shaders\n");
		return false;
	}

	// Assign samplers to texture units
	m_shManager->UpdateUni1i("heightmap", 0);
	m_shManager->UpdateUni1i("pdmap", 1);
	m_shManager->UpdateUni1i("colormap", 2);
	m_shManager->UpdateUni1i("detail0", 3);
	m_shManager->UpdateUni1i("detail1", 4);
	m_shManager->UpdateUni1i("detail2", 5);
	m_shManager->UpdateUni1i("detail3", 6);
	m_shManager->UpdateUniMat4fv("projection", m_proj_mat.m);
	m_shManager->UpdateUni1f("is_hd_stamp", (m_is_hd_stamp ? 1.0f : 0.0f));

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
	"g\t"		"= Toggle Gravity\n"
	"Space\t"	"= Jump/Float\n"
	"c\t"		"= Crouch/Sink\n"
	"f\t"		"= Toggle footprint deforms\n"
	"h\t"		"= High Detail Toggle\n"
	"Shift\t"	"= En/Disable Super Speed\n"
	"R-Mouse\t"	"= Pick Deform location\n"
	"L-Mouse\t" "= Rotate Camera\n"
	"Wheel\t"	"= Deform\n"
	"8/9/0\t"	"= HD Shader: Simple/Parallax/Geom\n"
	"F12\t"		"= Screenshot\n"
	"Esc\t"		"= Quit\n"
	);
	printf("-----------------------------------------\n");
	printf("-------------Stamp  Controls-------------\n");
	printf("-----------------------------------------\n");
	printf(
	"[/]\t"		"= Cycle Stamps\n"
	"Pg Up/Dn"	"= Stamp Scale\n"
	"+/-\t"		"= Stamp Intensity\n"
	"m\t"		"= Stamp Mirror\n"
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
	// Init the elevation data mutex
	m_elevationDataMutex = SDL_CreateMutex();
	m_waitSem			 = SDL_CreateSemaphore(0);

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

	// Create the deformer object
	printf("Creating deformer...\t\t");
	m_pDeform = new Deform(m_coarsemap_dim, HIGH_DIM, m_pClipmap->m_metre_to_tex, 1.0f/(HIGH_DIM * HIGH_RES));
	if (m_pDeform->HasError())
	{
		fprintf(stderr, "Error\n\tCould not create deformer\n");
		return false;
	}
	printf("Done\n");

	// Shader uniforms (Clipmap data)
	m_shManager->UpdateUni2f("scales",  m_pClipmap->m_tex_to_metre, m_pClipmap->m_metre_to_tex);

	// Generate the normal map and run a zero deform to init shaders
	printf("Creating initial deform...\t");
	m_pDeform->displace_heightmap(m_coarsemap, vector2(0.5f), vector2(0.0f), m_stampIndex, vector4(0.0f), true);
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
	if (m_pSkybox->HasError())
	{
		fprintf(stderr, "\t\tError\n\tCould not create skybox\n");
		return false;
	}
	printf("Done\n");

	// Assign some more to shader uniforms
	vector2 hdasq_its;
	hdasq_its.x = float(HD_AURA_SQ) / (CLIPMAP_RES * m_coarsemap_dim * CLIPMAP_RES * m_coarsemap_dim);
	hdasq_its.y = 1.0f / (HIGH_RES * HIGH_DIM * m_pClipmap->m_metre_to_tex);

	m_shManager->UpdateUni2fv("hdasq_its", hdasq_its.v);

	// Init stuff pertaining to the download of changed heightmap data for collision purposes
	m_XferState			 = CHILLED;
	m_otherState		 = CHILLED;
	m_cyclesPassed		 = -1;

	// Init the PBOs
	glGenBuffers(NUM_PBOS, m_pbo);
	for (int i = 0; i < NUM_PBOS; i++){
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[i]);
		glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(GLushort) * m_coarsemap_dim * m_coarsemap_dim, NULL,	GL_STREAM_READ);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Create the FBO
	glGenFramebuffers(1, &m_fboTransfer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboTransfer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// start the retriever thread
	m_retrieverThread	 = SDL_CreateThread(&map_retriever, this);

	// Create stuff for awesome screenshot
	glGenTextures(1, &m_screenshotTex);
	glBindTexture(GL_TEXTURE_2D, m_screenshotTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREENSHOT_W, SCREENSHOT_H, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glGenRenderbuffers(1, &m_screenshotDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_screenshotDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCREENSHOT_W, SCREENSHOT_H);
	glGenFramebuffers(1, &m_screenshotFBO);
	float asprat = float(SCREENSHOT_W)/SCREENSHOT_H;
	m_screenshotProj = perspective_proj(PI*.5f, asprat, NEAR_PLANE, FAR_PLANE);

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
	SDL_mutexP(m_elevationDataMutex);
	m_elevationData 	  = new GLushort[m_coarsemap_dim * m_coarsemap_dim];
	m_elevationDataBuffer = new GLushort[m_coarsemap_dim * m_coarsemap_dim];
	if (bitdepth==8){
		for (int i = 0; i < m_coarsemap_dim * m_coarsemap_dim; i++){
			m_elevationData[i] = m_elevationDataBuffer[i] = (GLushort)(USHRT_MAX * (bits[i] * 1.0f/255.0f));
		}
	}
	else{
		memcpy(m_elevationData, 		bits, m_coarsemap_dim*m_coarsemap_dim * sizeof(GLushort));
		memcpy(m_elevationDataBuffer, 	bits, m_coarsemap_dim*m_coarsemap_dim * sizeof(GLushort));
	}
	SDL_mutexV(m_elevationDataMutex);

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
bool
DefTer::SaveCoarseMap(string filename)
{
GLushort* 	mapdata = new GLushort[m_coarsemap_dim * m_coarsemap_dim];

	glBindTexture(GL_TEXTURE_2D, m_coarsemap.heightmap);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, mapdata);

	if (!SavePNG((char*)filename.c_str(), (GLubyte*)mapdata, 16, 1, m_coarsemap_dim, m_coarsemap_dim, true))
		return false;


	delete[] mapdata;
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

		m_shManager->UpdateUni2fv("click_pos", temp.v);
	}
}

//--------------------------------------------------------
// Interpolates the height of the coarsemap at the given location in world space
float
DefTer::InterpHeight(vector2 worldPos)
{
	const float scale = VERT_SCALE * 1.0f / USHRT_MAX;
	worldPos = (worldPos * m_pClipmap->m_metre_to_tex + vector2(0.5f)) * float(m_coarsemap_dim);
	int x0 	= int(worldPos.x);
	int y0 	= int(worldPos.y);
	float fx= worldPos.x - x0;
	float fy= worldPos.y - y0;

	int x1  = x0 < m_coarsemap_dim - 1 ? x0 + 1 : 0;
	int y1  = y0 < m_coarsemap_dim - 1 ? y0 + 1 : 0;
	
	SDL_mutexP(m_elevationDataMutex);
	float top = (1-fx) * m_elevationData[x0  + m_coarsemap_dim * (y0)	] * scale 
		      + (  fx) * m_elevationData[x1  + m_coarsemap_dim * (y0)	] * scale;
	float bot = (1-fx) * m_elevationData[x0  + m_coarsemap_dim * (y1)	] * scale
		      + (  fx) * m_elevationData[x1  + m_coarsemap_dim * (y1)	] * scale;
	SDL_mutexV(m_elevationDataMutex);

	return (1-fy) * top + fy * bot + EYE_HEIGHT * (m_is_crouching ? .5f : 1.0f);
}

//--------------------------------------------------------
// Handles the GPU side of transferring the updated coarsemap to the CPU
void
DefTer::UpdateCoarsemapStreamer(){
	static reTimer streamerTimer;
	streamerTimer.start();
	
	// Check if there has been a deform while transferring
	if (m_otherState != CHILLED && m_XferState <= READY){
		m_XferState = READY;
		m_otherState = CHILLED;
	}


	// If a deformation hasn't been made in a short while, but the map is different from the client's
	if (m_XferState==READY && m_deformTimer.peekElapsed() > MAP_TRANSFER_WAIT){
		DEBUG("__________\nStart transfer\n");
		// Setup PBO and FBO
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[0]);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboTransfer);
		if (m_cyclesPassed < 0)
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					m_coarsemap.heightmap, 0);

		// Commence transfer to PBO
		glReadPixels(0, 0, m_coarsemap_dim, m_coarsemap_dim, GL_RED, GL_UNSIGNED_SHORT, 0);

		// Reset buffer state
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		m_cyclesPassed = 0;
		m_XferState    = BUFFERING;
	}
	// Otherwise if we are transferring
	else if (m_XferState == BUFFERING){
		m_cyclesPassed++;
		if ( m_cyclesPassed > MAP_BUFFER_CYCLES ){
			// map buffer to sys memory
			DEBUG("map buffer\n");
			glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[0]);
			m_bufferPtr 	= (GLushort*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			m_XferState 	= RETRIEVING;
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

			// Signal retriever thread to take data from buffer:
			SDL_SemPost(m_waitSem);
		}
		else
			return;
	}
	// If the thread has finished with the buffer
	else if (m_XferState == DONE){
		// Unmap the buffer
		DEBUG("Unmap the buffer\n");
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[0]);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		m_XferState = CHILLED;
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	else{
		return;
	}
	CheckError("coarsemap streamer\n");
	DEBUG("Streamer took %.3fms this frame\n", streamerTimer.getElapsed()*1000);
}

//--------------------------------------------------------
// Process user input
void
DefTer::ProcessInput(float dt)
{
	int wheel_ticks 	 = m_input.GetWheelTicks();
	MouseDelta move 	 = m_input.GetMouseDelta();
	float terrain_height = InterpHeight(vector2(m_cam_translate.x, m_cam_translate.z));

	// Rotate Camera
	if (m_input.IsButtonPressed(1))
	{
		// Pitch
		m_cam_rotate.x += dt*move.y*PI*.1f;
		// Yaw
		m_cam_rotate.y += dt*move.x*PI*.1f;

		//Clamp the camera to prevent the user flipping
		//upside down messing up everything
		if (m_cam_rotate.x < -PI * 0.5f)
			m_cam_rotate.x = -PI * 0.5f;
		if (m_cam_rotate.x > PI * 0.5f)
			m_cam_rotate.x = PI * 0.5f;
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
		vector3 p = perspective_unproj_world(frag, float(SCREEN_W), float(SCREEN_H), NEAR_PLANE, FAR_PLANE, 1.0f, inverse);

		// Check that the position is in valid range when using coarse stamping
		if (!m_is_hd_stamp && vector2(p.x, p.z).Mag() > COARSE_AURA)
		{
			m_clicked = false;
		}
		else
		{
			// Factor in the camera translation
			m_stampSIRM.z	 = PI / 2.0f + atan2f(p.z, p.x);
			p				+= m_cam_translate;
			m_clickPos		 = vector2(p.x, p.z);
			m_clicked		 = true;
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
		vector2 clickDiff	 = m_clickPos - vector2(m_cam_translate.x, m_cam_translate.z);
		vector4 stampSIRM	 = m_stampSIRM;
		stampSIRM.y			*= wheel_ticks;

		// Check if in wireframe mode and remember to switch to fill mode
			if (m_is_wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Perform either  a HD or coarse deformation
		if (m_is_hd_stamp)
		{
			m_pCaching->DeformHighDetail(m_clickPos, m_stampIndex, stampSIRM);
		}
		else
		{
			vector2 areaMin(m_clickPos - vector2(stampSIRM.x / 2.0f));
			vector2 areaMax(areaMin	+ stampSIRM.x);

			areaMin *= m_pClipmap->m_metre_to_tex;
			areaMin += vector2(0.5f);
			areaMax *= m_pClipmap->m_metre_to_tex;

			areaMax += vector2(0.5f);

			list<vector2> fuck;

			// Left-Col
			if (areaMin.x < 0.0)
			{
				// Left-Top
				if (areaMin.y < 0.0)
					fuck.push_back(vector2(1.0f, 1.0f));

				// Left-Centre
				if (areaMax.y > 0.0 && areaMin.y < 1.0)
					fuck.push_back(vector2(1.0f, 0.0f));

				// Left-Bottom
				if (areaMax.y > 1.0)
					fuck.push_back(vector2(1.0f, -1.0f));
			}

			// Centre-Col
			if (areaMin.x < 1.0 && areaMax.x > 0.0)
			{
				// Centre-Top
				if (areaMin.y < 0.0)
					fuck.push_back(vector2(0.0f, 1.0f));

				// Centre-Centre
				if (areaMax.y > 0.0 && areaMin.y < 1.0)
					fuck.push_back(vector2(0.0f));
				
				// Centre-Bottom
				if (areaMax.y > 1.0)
					fuck.push_back(vector2(0.0f, -1.0f));
			}

			// Right-Col
			if (areaMax.x > 1.0)
			{
				// Right-Top
				if (areaMin.y < 0.0)
					fuck.push_back(vector2(-1.0f, 1.0f));

				// Right-Centre
				if (areaMax.y > 0.0 && areaMin.y < 1.0)
					fuck.push_back(vector2(-1.0f, 0.0f));

				// Right-Bottom
				if (areaMax.y > 1.0)
					fuck.push_back(vector2(-1.0f, -1.0f));
			}
			
			// Displace the heightmap
			for (list<vector2>::iterator shit = fuck.begin(); shit != fuck.end(); shit++)
				m_pDeform->displace_heightmap(m_coarsemap, m_clickPos, *shit, m_stampIndex, stampSIRM, true);

			// Calculate the normals
			for (list<vector2>::iterator shit = fuck.begin(); shit != fuck.end(); shit++)
				m_pDeform->calculate_pdmap(m_coarsemap, m_clickPos, *shit, stampSIRM.x, true);
			
			// Once this is finally complete, change variables relating to streaming the coarsemap
			// to the CPU for collision detection
			// Restart timer
			m_deformTimer.start();
			m_otherState = READY;
		}

		// Reset to wireframe mode
		if (m_is_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Take screenshot
	static int lastScreenshot = 1;
	if (m_input.WasKeyPressed(SDLK_F12))
	{
		char  filename[256];
		int   currentViewport[4];
		glGetIntegerv(GL_VIEWPORT, currentViewport);
		glViewport(0, 0, SCREENSHOT_W, SCREENSHOT_H);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_screenshotFBO);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				m_screenshotTex, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
				m_screenshotDepth);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		matrix4 temp = m_proj_mat;
		m_proj_mat = m_screenshotProj;

		m_shManager->UpdateUniMat4fv("projection", m_proj_mat.m);
		Render(.0f);
		m_proj_mat = temp;
		m_shManager->UpdateUniMat4fv("projection", m_proj_mat.m);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		
		GLubyte* framebuffer = new GLubyte[3 * SCREENSHOT_W * SCREENSHOT_H];

		glBindTexture(GL_TEXTURE_2D, m_screenshotTex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, framebuffer);
		mkdir("screenshots");
		sprintf(filename, "screenshots/screenshot%05d.png", lastScreenshot++);
		if (SavePNG(filename, framebuffer, 8, 3, SCREENSHOT_W, SCREENSHOT_H, false))
			printf("Wrote screenshot to %s\n", filename);
		else
			fprintf(stderr, "Failed to write screenshot\n");

		glBindTexture(GL_TEXTURE_2D, 0);
		glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);	
		delete[] framebuffer;		
	}


	// Toggle footprints
	if (m_input.WasKeyPressed(SDLK_f)){
		m_drawing_feet ^= true;
		printf("Footprints: %s\n", m_drawing_feet ? "ON" : "OFF");
	}


	// Toggle wireframe
	if (m_input.WasKeyPressed(SDLK_l))
	{
		m_is_wireframe ^= true;
		if (m_is_wireframe)
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

		m_shManager->UpdateUni1f("is_hd_stamp", (m_is_hd_stamp ? 1.0f : 0.0f));

		printf("HD Mode: %s\n", m_is_hd_stamp ? "ON" : "OFF");
	}


	// Controls to change the HDShader
	if (m_input.WasKeyPressed(SDLK_8))
	{
		m_hdShaderIndex = m_shmSimple;
		printf("HD Shader: None\n");
	}
	else if (m_input.WasKeyPressed(SDLK_9))
	{
		m_hdShaderIndex = m_shmParallax;
		printf("HD Shader: Parallax Mapping\n");
	}
	else if (m_input.WasKeyPressed(SDLK_0))
	{
		m_hdShaderIndex = m_shmGeomTess;
		printf("HD Shader: Geometry Tessellation\n");
	}

	// Toggle the stamp values
	if (m_input.WasKeyPressed(SDLK_RIGHTBRACKET))
	{
		m_stampIndex = WRAP((m_stampIndex + 1), STAMPCOUNT);
		printf("Stamp: %s\n", GetStampMan()->GetStampName(m_stampIndex).c_str());
	}
	else if (m_input.WasKeyPressed(SDLK_LEFTBRACKET))
	{
		m_stampIndex = WRAP((m_stampIndex - 1), STAMPCOUNT);
		printf("Stamp: %s\n", GetStampMan()->GetStampName(m_stampIndex).c_str());
	}
	// Change the scale of the stamp
	if (m_input.IsKeyPressed(SDLK_PAGEUP))
	{
		m_stampSIRM.x = min(m_stampSIRM.x + (20.0f * dt), 200.0f);
		printf("Stamp Scale: %.1f\n", m_stampSIRM.x);
	}
	else if (m_input.IsKeyPressed(SDLK_PAGEDOWN))
	{
		m_stampSIRM.x = max(m_stampSIRM.x - (20.0f * dt), 1.0f);
		printf("Stamp Scale: %.1f\n", m_stampSIRM.x);
	}
	// Change the intensity of the stamp
	if (m_input.IsKeyPressed(SDLK_PLUS) || m_input.IsKeyPressed(SDLK_KP_PLUS))
	{
		m_stampSIRM.y = min(m_stampSIRM.y + (0.5f * dt), 1.0f);
		printf("Stamp Intensity: %.2f\n", m_stampSIRM.y);
	}
	else if (m_input.IsKeyPressed(SDLK_MINUS) || m_input.IsKeyPressed(SDLK_KP_MINUS))
	{
		m_stampSIRM.y = max(m_stampSIRM.y - (0.5f * dt), 0.01f);
		printf("Stamp Intensity: %.2f\n", m_stampSIRM.y);
	}
	// Toggle stamp mirroring
	if (m_input.WasKeyPressed(SDLK_m))
	{
		if (m_stampSIRM.w == 0.0f)
			m_stampSIRM.w = 1.0f;
		else
			m_stampSIRM.w = 0.0f;

		printf("Stamp Mirroring: %s\n",  (m_stampSIRM.w == 1.0f) ? "ON" : "OFF");
	}


	// Toggle gravity
	if (m_input.WasKeyPressed(SDLK_g))
	{
		m_gravity_on ^= true;
		m_lastPosition = m_cam_translate;	// velocity = 0
		printf("Gravity: %s\n", m_gravity_on ? "ON" : "OFF");
	}

	// Controls to handle movement of the camera
	// Speed in m/s (average walking speed)
	vector3 moveDirection;
	matrix4 rotation;
	if (!m_gravity_on)
		rotation = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f) * rotate_tr(-m_cam_rotate.x, 1.0f, .0f, .0f);
	else
		rotation = rotate_tr(-m_cam_rotate.y, .0f, 1.0f, .0f);
	if (m_input.IsKeyPressed(SDLK_w))
		moveDirection += rotation * vector3( 0.0f, 0.0f,-1.0f);

	if (m_input.IsKeyPressed(SDLK_s))
		moveDirection += rotation * vector3( 0.0f, 0.0f, 1.0f);

	if (m_input.IsKeyPressed(SDLK_a))
		moveDirection += rotation * vector3(-1.0f, 0.0f, 0.0f);

	if (m_input.IsKeyPressed(SDLK_d))
		moveDirection += rotation * vector3( 1.0f, 0.0f, 0.0f);

	if (moveDirection.Mag2() > 1.0e-3){
		moveDirection.Normalize();
		if (m_input.IsKeyPressed(SDLK_LSHIFT))
			moveDirection *= 3.0f;
		m_frameAcceleration += moveDirection *= ACCELERATION;
	}


	// Controls for jumping (Floating)
	if (m_input.WasKeyPressed(SDLK_SPACE))
	{
		// Only jump if on the ground
		if (m_hit_ground ||  m_cam_translate.y - EYE_HEIGHT - terrain_height < .1f)
			m_lastPosition.y -= 8.0f * DT;
	//		m_velocity.y = 8.0f;
	}
	else if (m_input.IsKeyPressed(SDLK_SPACE) && !m_gravity_on)
	{
		// Float if gravity off
		m_cam_translate.y += 5.0f * dt;
		m_lastPosition.y  += 5.0f * dt;
	}
	// Controls for crouching (Sinking)
	if (m_input.IsKeyPressed(SDLK_c))
	{
		// Sink if gravity off
	   if (!m_gravity_on)
	   {
			m_cam_translate.y -= 5.0f * dt;
			m_lastPosition.y  -= 5.0f * dt;
	   }
	   // Crouch - Drop camera down
	   else if (!m_is_crouching)
	   {
		   m_is_crouching = true;
		   m_cam_translate.y -= EYE_HEIGHT * .5f;
		   m_lastPosition.y   = m_cam_translate.y;
	   }
	}
	// Disable crouching if it was previously enabled and no longer pressing Ctrl
	if (m_is_crouching && !m_input.IsKeyPressed(SDLK_c))
	{
		m_is_crouching = false;
	}
	
	reGL3App::ProcessInput(dt);
}

//--------------------------------------------------------
void
DefTer::Logic(float dt)
{
	float speed2, terrain_height;

	// Increase game speed
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		dt *= 5.0f;

	// Update the caching system
	m_pCaching->Update(vector2(m_cam_translate.x, m_cam_translate.z), vector2(m_cam_rotate.x, m_cam_rotate.y));

	// Use a fixed time-step for physics, so that the more accurate Verlet method can be used
	static float compoundDT = .0f;
	compoundDT += dt;
	while (compoundDT > DT){
		// Perform camera physics
		vector3 accel 	= m_frameAcceleration;
		vector3 velocity= (m_cam_translate - m_lastPosition) * invDT; // (f(t)-f(t-1))/(dt)
		if (m_gravity_on)
			accel += GRAVITY;
		if (m_hit_ground)
			accel += - FRICTION * vector3(velocity.x, .0f, velocity.z);
		else
			accel += - AIR_DRAG * velocity;

		//m_velocity 		+= accel * DT;
		vector3 temp 	= m_cam_translate;
		m_cam_translate	+= m_cam_translate - m_lastPosition + accel * DT * DT;
		velocity		= (m_cam_translate - m_lastPosition) * invDT * .5f; // (f(t+1)-f(t-1))/(2dt)
		m_lastPosition 	= temp;
		speed2			= velocity.Mag2();

		// If the camera is moving we may need to drag the selection to within the HD Aura
		if (speed2 > 1.0e-5)
			UpdateClickPos();

		// Boundary check for wrapping position
		static float boundary = m_coarsemap_dim* m_pClipmap->m_quad_size * .5f;
		if (m_cam_translate.x > boundary){
			m_cam_translate.x -= boundary * 2.0f;
			m_lastPosition.x  -= boundary * 2.0f;
		}
		else if (m_cam_translate.x < -boundary){
			m_cam_translate.x += boundary * 2.0f;
			m_lastPosition.x  += boundary * 2.0f;
		}
		if (m_cam_translate.z > boundary){
			m_cam_translate.z -= boundary * 2.0f;
			m_lastPosition.z  -= boundary * 2.0f;
		}
		else if (m_cam_translate.z < -boundary){
			m_cam_translate.z += boundary * 2.0f;
			m_lastPosition.z  += boundary * 2.0f;
		}

		// Don't let player go under the terrain
		terrain_height = InterpHeight(vector2(m_cam_translate.x, m_cam_translate.z));

		if (m_cam_translate.y < terrain_height)
		{
			m_lastPosition.y	= 
			m_cam_translate.y 	= terrain_height;
			m_hit_ground		= true;
		} else{
			m_hit_ground		= false;
		}
		compoundDT -= DT;
	}

	// Create footprints
	m_footprintDT += dt;
	if (m_drawing_feet && m_gravity_on && speed2 > .025f)
	{ 
		if (m_cam_translate.y-EYE_HEIGHT - terrain_height < .1f && m_footprintDT > STEP_TIME){
			vector4 stampSIRM= vector4(0.5f, 2.0f, m_cam_rotate.y, m_flipFoot ? 1.0f : 0.0f);
			vector2 foot 	 = vector2(m_cam_translate.x, m_cam_translate.z);
			foot 			+= rotate_tr2(m_cam_rotate.y) * vector2(m_flipFoot ? 0.3f : -0.3f, 0.0f);
			m_footprintDT 	 = 0.0f;
			m_flipFoot		^= true;
			//m_pCaching->DeformHighDetail(foot, "leftfoot", stampSIRM);
		}
	}

	// Pass the camera's texture coordinates and the shift amount necessary
	// cam = x and y   ;  shift = z and w
	vector3 pos 	  = m_cam_translate * m_pClipmap->m_metre_to_tex;
	m_clipmap_shift.x = -fmodf(m_cam_translate.x, 32*m_pClipmap->m_quad_size);
	m_clipmap_shift.y = -fmodf(m_cam_translate.z, 32*m_pClipmap->m_quad_size);
	
	m_shManager->UpdateUni1f("cam_height", m_cam_translate.y);
	m_shManager->UpdateUni4f("cam_and_shift", pos.x, pos.z, m_clipmap_shift.x, m_clipmap_shift.y);

	m_frameAcceleration.set(.0f);
}

//--------------------------------------------------------
void
DefTer::Render(float dt)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	matrix4 rotate, rotateclamp, cullviewproj, viewproj, translate;

	translate= translate_tr(.0f, -m_cam_translate.y, .0f);
	rotateclamp  = rotate_tr(max(.0f, m_cam_rotate.x), 1.0f, .0f, .0f) * rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);
	cullviewproj = m_proj_mat * rotateclamp * translate;

	rotate   = rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f) * rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);
	viewproj = m_proj_mat * rotate;

	// Bind coarse heightmap and its corresponding normal and colour maps
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.heightmap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_coarsemap.pdmap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_colormap_tex);
	

	// Cull invisible blocks and render clipmap
	m_pClipmap->cull(cullviewproj, m_clipmap_shift);

	// Block of four tiles where the 0th tile is the top left active tile
	Tile activeTiles[4];
	m_pCaching->GetActiveTiles(activeTiles);
	int firstTile[2] = {activeTiles[0].m_row, activeTiles[0].m_col};


	// Set the view matrix.
	m_shManager->UpdateUniMat4fv("view", rotate.m);
	m_shManager->UpdateUni2i("tileOffset", firstTile[1], firstTile[0]);

	// Set the active shader to be the current HD shader chosen
	m_shManager->SetActiveShader(m_hdShaderIndex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, activeTiles[0].m_texdata.heightmap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, activeTiles[1].m_texdata.heightmap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, activeTiles[2].m_texdata.heightmap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, activeTiles[3].m_texdata.heightmap);

	BEGIN_PROF;
	m_pClipmap->render_inner();
	// Switch to the simple shader and render the rest
	m_shManager->SetActiveShader(m_shmSimple);
	m_pClipmap->render_levels();
	
	// Only render these when not in wireframe mode
	if (!m_is_wireframe)
		m_pSkybox->render(viewproj);

	if (!m_is_wireframe)
		m_pCaching->Render();
	END_PROF;

	// Get the lastest version of the coarsemap from the GPU for the next frame
	UpdateCoarsemapStreamer();

	// Swap windows to show the rendered data
	SDL_GL_SwapWindow(m_pWindow);
}

//--------------------------------------------------------
int 
map_retriever(void* defter)
{
	DefTer* main = (DefTer*) defter;
	float scale = VERT_SCALE * 1.0f / USHRT_MAX;
	int dim = main->m_coarsemap_dim;
	reTimer copyTimer;

	while(main->m_isRunning)
	{
		// unlock mutex, wait for a signal to transfer and then get mutex
		SDL_SemWait(main->m_waitSem);
		copyTimer.start();
		// We can unlock it and continue to load into the array buffer
		if (!main->m_isRunning)
		{
			break;
		}
		DEBUG("retrieving\n");
		
		// copy data and transform
		memcpy(main->m_elevationDataBuffer, main->m_bufferPtr, main->m_coarsemap_dim*main->m_coarsemap_dim * sizeof(GLushort));

		// finally lock the data array, and swap the pointers
		SDL_mutexP(main->m_elevationDataMutex);
		GLushort * temp 			= main->m_elevationData;
		main->m_elevationData 		= main->m_elevationDataBuffer;
		main->m_elevationDataBuffer = temp;
		main->m_XferState 			= DONE;
		DEBUG("Retriever took %.3fms to copy into sys mem\n", copyTimer.getElapsed() * 1000);
		SDL_mutexV(main->m_elevationDataMutex);
	}

	return 0;
}
