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
#include "model_manager.h"
#include "game_entity.h"
#include "shockwave.h"
#include "fighterjet.h"
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
	conf.fullscreen = true;
	if (argc > 1 && strcmp(argv[1], "record")==0)
		conf.demo	= RE_DEMO_RECORD;
	if (argc > 1 && strcmp(argv[1], "play")==0)
		conf.demo	= RE_DEMO_PLAY;

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
	m_shModel			  = NULL;
	m_shFlash			  = NULL;
	m_shHUD				  = NULL;
	m_shManager			  = NULL;
	m_pDeform 			  = NULL;
	m_pClipmap			  = NULL;
	m_pCaching			  = NULL;
	m_pSkybox 			  = NULL;
	m_pModelManager		  = NULL;
	m_pShockwave		  = NULL;
	m_pFighterJet		  = NULL;
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
	RE_DELETE(m_shModel);
	RE_DELETE(m_shFlash);
	RE_DELETE(m_shHUD);
	KillUtil();
	RE_DELETE(m_shManager);
	RE_DELETE(m_pDeform);
	RE_DELETE(m_pSkybox);
	RE_DELETE(m_pClipmap);
	RE_DELETE(m_pCaching);
	RE_DELETE(m_pModelManager);
	RE_DELETE(m_pCamera);
	RE_DELETE(m_pShockwave);
	RE_DELETE(m_pFighterJet);
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
	glDeleteTextures(1, &m_bombXTex);
	glDeleteTextures(1, &m_crosshairTex);
	glDeleteTextures(1, &m_muzzleFlashTex);
	glDeleteTextures(1, &m_cursorTex);
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

	InitCursor();

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

	// Initialise to edit mode
	m_useMode		= EDIT_MODE;
	m_activeWeapon	= GUN;
	m_isShooting	= false;
	m_mouseCompensate.x	= 0.0f;
	m_mouseCompensate.y	= 0.0f;

	// Set the initial stamp mode and clicked state
	m_stampSIRM		= vector4(20.0f, 0.2f, 0.0f, 0.0f);
	m_is_hd_stamp	= false;
	m_clicked		= false;
	m_clickPos		= vector2(0.0f);
	m_clickPosPrev	= vector2(0.0f);

	// Init the world settings
	m_gravity_on	= true;
	m_is_crouching	= false;
	m_footprintDT	= 0.0f;
	m_flipFoot		= false;
	m_drawing_feet	= false;

	// Set initial shader index values
	m_shmSimple		= 0;
	m_shmParallax	= 0;
	m_shmGeomTess	= 0;

	// Init Shaders
	printf("Initialising shader manager...\t");
	m_shManager		 = new ShaderManager();
	error			 = !m_shManager->AddShader("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag", &m_shmSimple);
	error			&= !m_shManager->AddShader("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag", &m_shmParallax);
	error			&= !m_shManager->AddShader("shaders/simple.vert","shaders/simple.geom","shaders/simple.frag", &m_shmGeomTess);
	m_hdShaderIndex	 = m_shmSimple;

	m_shModel		= new ShaderProg("shaders/model.vert", "", "shaders/model.frag");
	m_shFlash		= new ShaderProg("shaders/flash.vert", "", "shaders/flash.frag");
	m_shHUD			= new ShaderProg("shaders/hud.vert", "", "shaders/hud.frag");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	m_shManager->BindAttrib("vert_Position", 0);
	m_shManager->BindAttrib("vert_TexCoord", 1);
	glBindAttribLocation(m_shModel->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shModel->m_programID, 1, "vert_Normal");
	glBindAttribLocation(m_shModel->m_programID, 2, "vert_TexCoord");
	glBindAttribLocation(m_shFlash->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shHUD->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shHUD->m_programID, 1, "vert_TexCoord");

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
	if (!m_shModel->CompileAndLink() || !m_shFlash->CompileAndLink() || !m_shHUD->CompileAndLink()){
		printf("Error\n\tWill not continue without working MODEL shaders\n");
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
	m_shManager->UpdateUni1i("curStamp",11);
	m_shManager->UpdateUniMat4fv("projection", m_proj_mat.m);
	m_shManager->UpdateUni1f("is_hd_stamp", (m_is_hd_stamp ? 1.0f : 0.0f));

	glUseProgram(m_shModel->m_programID);
	glUniform1i(glGetUniformLocation(m_shModel->m_programID, "colormap"),  0);
	glUniformMatrix4fv(glGetUniformLocation(m_shModel->m_programID, "projection"), 1, GL_FALSE,	m_proj_mat.m);

	glUseProgram(m_shHUD->m_programID);
	glUniform1i(glGetUniformLocation(m_shModel->m_programID, "HUDelem"),  0);

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

	// Load textures for HUD
	printf("Loading bomb X texture...\t");
	if (!LoadPNG(&m_bombXTex, "images/bombX.png"))
		return false;
	if (!LoadPNG(&m_crosshairTex, "images/crosshair.png"))
		return false;
	if (!LoadPNG(&m_muzzleFlashTex, "images/muzzleflash.png"))
		return false;
	if (!LoadPNG(&m_cursorTex, "images/cursor.png"))
		return false;
	printf("Done\n");


	// Initialise Stamp Manager
	printf("Initialising stamp manager...\t");
	InitStampMan();
	if (GetStampMan()->HasError())
	{
		fprintf(stderr, "Error\n\tCould not initialise stamp man\n");
		return false;
	}
	printf("Done\n");

	// Create & initialise the clipmap
	printf("Creating clipmap...\t\t");
	m_pClipmap = new Clipmap(CLIPMAP_DIM, CLIPMAP_RES, CLIPMAP_LEVELS, m_coarsemap_dim);
	printf("Done\n");
	printf("Initialising clipmap...\t\t");
	m_pClipmap->Init();
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

	// Create the model manager
	glActiveTexture(GL_TEXTURE0);
	printf("Loading models...\t\t");
	bool noModelError = true;
	m_pModelManager = new ModelManager();
	noModelError &= m_pModelManager->LoadModel("gun", 		"models/gun.reMo");
	noModelError &= m_pModelManager->LoadModel("bomb", 		"models/bomb.reMo");
	noModelError &= m_pModelManager->LoadModel("fighterjet","models/fighterjet.reMo");
	if (noModelError){
		printf("Done\n");
	}
	
	// Init the cameras position such that it is in the middle of a tile
	m_pCamera 		= new Camera(m_pModelManager->GetModel("gun"));
	float halfTile  = HIGH_DIM * HIGH_RES * 0.5f;
	m_pCamera->SetTranslate(vector3(-halfTile, 0.0f, -halfTile));

	// Create the jet fighter
	m_pFighterJet	= new FighterJet(m_pModelManager->GetModel("fighterjet"), &m_bombs, m_pModelManager);

	// Create the Shockwave object that will allow shockwaves to happen
	printf("Creating shockwave...\t\t");
	fflush(stdout);
	m_pShockwave = new Shockwave(m_coarsemap, m_coarsemap_dim/2, m_pDeform);
	if (m_pShockwave->HasError())
	{
		fprintf(stderr, "Error\n\tCould not create shockwave\n");
		return false;
	}
	printf("Done\n");

	// Shader uniforms (Clipmap data)
	m_shManager->UpdateUni2f("scales",  m_pClipmap->m_tex_to_metre, m_pClipmap->m_metre_to_tex);

	// Generate the normal map and run a zero deform to init shaders
	printf("Creating initial deform...\t");
	m_pDeform->displace_heightmap(m_coarsemap, vector2(0.5f), vector2(0.0f), vector4(0.0f), true);
	m_pDeform->create_pdmap(m_coarsemap, true);
	if (!CheckError("Creating initial deform"))
		return false;
	printf("Done\n");

	// Create & initialise the caching system
	printf("Creating caching system...\t");
	m_pCaching = new Caching(m_pDeform, CACHING_DIM, m_coarsemap_dim, CLIPMAP_RES, HIGH_DIM, HIGH_RES);
	printf("Done\n");
	printf("Initialising caching system...\t");
	m_pCaching->Init(m_coarsemap.heightmap, m_colormap_tex, m_pCamera->GetHorizPosition());
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
	m_XferWaitState		 = CHILLED;
	m_cyclesPassed		 = -1;

	// Init the PBOs
	glGenBuffers(NUM_PBOS, m_pbo);
	for (int i = 0; i < NUM_PBOS; i++){
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[i]);
		glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(float) * m_coarsemap_dim * m_coarsemap_dim, NULL,	GL_STREAM_READ);
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
	
	// Create elevationData for camera collisions
	SDL_mutexP(m_elevationDataMutex);
	m_elevationData 	  = new float[m_coarsemap_dim * m_coarsemap_dim];
	m_elevationDataBuffer = new float[m_coarsemap_dim * m_coarsemap_dim];
	if (bitdepth==8){
		for (int i = 0; i < m_coarsemap_dim * m_coarsemap_dim; i++){
			m_elevationData[i] = m_elevationDataBuffer[i] =  VERT_SCALE * bits[i] * 1.0f/255.0f;
		}
	}
	else if (bitdepth==16){
		for (int i = 0; i < m_coarsemap_dim * m_coarsemap_dim; i++){
			m_elevationData[i] = m_elevationDataBuffer[i] =  VERT_SCALE * bits[i] * 1.0f/USHRT_MAX;
		}
	}
	else
	{
		fprintf(stderr, "Error\n\tCannot load files with bitdepths other than 8- or 16-bit: %s\n",
				filename.c_str());
		SDL_mutexV(m_elevationDataMutex);
		return false;
	}
	SDL_mutexV(m_elevationDataMutex);

	// Upload to GPU Texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RED, GL_FLOAT, m_elevationData);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

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

	// Can only save 0 -> VERT_SCALE range of values. Clamps the rest to this range
	for (int i = 0; i < m_coarsemap_dim * m_coarsemap_dim; i++){
		mapdata[i] = (GLushort)(max(min(m_elevationData[i]/VERT_SCALE, 1.0f), .0f) * USHRT_MAX);
	}

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
			vector2 camPos = m_pCamera->GetHorizPosition();
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
	worldPos = (worldPos * m_pClipmap->m_metre_to_tex + vector2(0.5f)) * float(m_coarsemap_dim);
	int x0 	= int(worldPos.x);
	int y0 	= int(worldPos.y);
	float fx= worldPos.x - x0;
	float fy= worldPos.y - y0;

	int x1  = x0 < m_coarsemap_dim - 1 ? x0 + 1 : 0;
	int y1  = y0 < m_coarsemap_dim - 1 ? y0 + 1 : 0;
	
	SDL_mutexP(m_elevationDataMutex);
	float top = (1-fx) * m_elevationData[x0  + m_coarsemap_dim * (y0)	]
		      + (  fx) * m_elevationData[x1  + m_coarsemap_dim * (y0)	];
	float bot = (1-fx) * m_elevationData[x0  + m_coarsemap_dim * (y1)	]
		      + (  fx) * m_elevationData[x1  + m_coarsemap_dim * (y1)	];
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
	if (m_XferWaitState != CHILLED && m_XferState <= READY){
		m_XferState = READY;
		m_XferWaitState = CHILLED;
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
		glReadPixels(0, 0, m_coarsemap_dim, m_coarsemap_dim, GL_RED, GL_FLOAT, 0);

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
			m_bufferPtr 	= (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
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
	int wheel_ticks 		= m_input.GetWheelTicks();
	MouseDelta tempDelta	= m_input.GetMouseDelta();
	vector2 mouseDelta		= vector2(tempDelta.x, tempDelta.y);
	float terrain_height	= InterpHeight(m_pCamera->GetHorizPosition());

	// Change Mode
	if (m_input.WasKeyPressed(SDLK_COMMA))
	{
		if (m_useMode == EDIT_MODE)
		{
			printf("Switched to GAME MODE\n");
			
			// Centre mouse to screen without compensation such that view does not move
			CentreMouse(false);
			m_input.GetMouseDelta();
			// Switch to game mode
			m_useMode = GAME_MODE;

			// Enable a model for the camera
			m_pCamera->m_pModel = m_pCamera->m_pGunModel;
			m_activeWeapon 	= GUN;
			m_mouseLook 	= true;

			// Disable edit stuff
			m_is_hd_stamp	= false;
			m_clicked		= false;
			m_shManager->UpdateUni1f("is_hd_stamp", (m_is_hd_stamp ? 1.0f : 0.0f));

#ifdef WIN32
			ShowCursor(0);
#endif
		}
		else
		{
			printf("Switched to EDIT MODE\n");

			// Switch to edit mode
			m_useMode = EDIT_MODE;

			// Disable the model for the camera
			m_pCamera->m_pModel = NULL;
#ifdef WIN32
			ShowCursor(1);
#endif
		}
	}

	// Call input processing function based on mode (EDIT or GAME)
	if (m_useMode == EDIT_MODE)
		EditModeInput(dt, mouseDelta, wheel_ticks);
	else
		GameModeInput(dt, mouseDelta, wheel_ticks);

	// Increase the game speed
	if (m_input.IsKeyPressed(SDLK_LSHIFT) || m_input.IsKeyPressed(SDLK_RSHIFT))
	{
		dt *= 5.0f;
		m_is_super_speed = true;
	}
	if (m_is_super_speed && !(m_input.IsKeyPressed(SDLK_LSHIFT) || m_input.IsKeyPressed(SDLK_RSHIFT)))
	{
		m_is_super_speed = false;
	}


	// Take screenshot
	static int lastScreenshot = 1;
	if (m_input.WasKeyPressed(SDLK_F12) || m_config.demo == RE_DEMO_PLAY)
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
		WIREFRAMEON ^= true;
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

	// Toggle gravity
	if (m_input.WasKeyPressed(SDLK_g))
	{
		m_gravity_on ^= true;
		m_pCamera->ZeroVelocity();	// velocity = 0
		printf("Gravity: %s\n", m_gravity_on ? "ON" : "OFF");
	}

	// Controls to handle movement of the camera
	// Speed in m/s (average walking speed)
	vector3 moveDirection;
	matrix4 rotation;
	if (!m_gravity_on)
		rotation = rotate_tr(m_pCamera->m_rotate.y, .0f, 1.0f, .0f) 
				 * rotate_tr(m_pCamera->m_rotate.x, 1.0f, .0f, .0f);
	else
		rotation = rotate_tr(m_pCamera->m_rotate.y, .0f, 1.0f, .0f);
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
		m_pCamera->m_frameAcceleration += moveDirection * ACCELERATION;
	}


	// Controls for jumping (Floating)
	if (m_input.WasKeyPressed(SDLK_SPACE))
	{
		// Only jump if on the ground
		if (m_pCamera->m_onGround ||  m_pCamera->m_translate.y - EYE_HEIGHT - terrain_height < .1f)
			m_pCamera->AddVelocity(vector3(.0f, 8.0f, .0f));
	}
	else if (m_input.IsKeyPressed(SDLK_SPACE) && !m_gravity_on)
	{
		// Float if gravity off
		m_pCamera->m_translate.y += 5.0f * dt;
		m_pCamera->ZeroVelocity();
	}
	// Controls for crouching (Sinking)
	if (m_input.IsKeyPressed(SDLK_c))
	{
		// Sink if gravity off
	   if (!m_gravity_on)
	   {
			m_pCamera->m_translate.y -= 5.0f * dt;
			m_pCamera->ZeroVelocity();
	   }
	   // Crouch - Drop camera down
	   else if (!m_is_crouching)
	   {
			m_is_crouching = true;
			m_pCamera->m_translate.y -= EYE_HEIGHT*.5f;
			m_pCamera->ZeroVelocity();
	   }
	}
	// Disable crouching if it was previously enabled and no longer pressing Ctrl
	if (m_is_crouching && !m_input.IsKeyPressed(SDLK_c))
	{
		m_is_crouching = false;
	}
	
	reGL3App::ProcessInput(dt);
	m_input.ClearStates();
}

//--------------------------------------------------------
void
DefTer::GameModeInput(float dt, vector2 mouseDelta, int ticks)
{
	// Ignore the first mouse movement due to hardware glitch
	if (m_mouseFirstMove &&  mouseDelta != 0.0f)
	{
		m_mouseFirstMove = false;
		mouseDelta = vector2(0.0f);
	}

	// If in mouse look mode, do this
	if (m_mouseLook)
	{
		// Call input processing function based on mode (GAME or EDIT)
		mouseDelta -= m_mouseCompensate;
		m_mouseCompensate = vector2(0.0f);

		// Pitch
		m_pCamera->m_rotate.x -= dt * mouseDelta.y * PI * 0.1f;
		// Yaw
		m_pCamera->m_rotate.y -= dt * mouseDelta.x * PI * 0.1f;

		//Clamp the camera to prevent the user flipping
		//upside down messing up everything
		if (m_pCamera->m_rotate.x < -PI * 0.5f)
			m_pCamera->m_rotate.x = -PI * 0.5f;
		if (m_pCamera->m_rotate.x > PI * 0.5f)
			m_pCamera->m_rotate.x = PI * 0.5f;
	}

	// Iterate through weapons with []'s
	bool weaponChanged = false;
	if (m_input.WasKeyPressed(SDLK_RIGHTBRACKET))
	{
		m_activeWeapon = (WeaponMode)WRAP(m_activeWeapon + 1, N_WEAPONS);
		printf ("Weapon: %s\n", WEAPON_NAME[(int)m_activeWeapon]);
		weaponChanged = true;
	}		
	else if (m_input.WasKeyPressed(SDLK_LEFTBRACKET))
	{
		m_activeWeapon = (WeaponMode)WRAP(m_activeWeapon - 1, N_WEAPONS);
		printf ("Weapon: %s\n", WEAPON_NAME[(int)m_activeWeapon]);
		weaponChanged = true;
	}
	if (ticks){
		m_activeWeapon = (WeaponMode)WRAP(m_activeWeapon + ticks, N_WEAPONS);
		printf ("Weapon: %s\n", WEAPON_NAME[(int)m_activeWeapon]);
		weaponChanged = true;
	}


	// FIRE!!
	if (m_input.WasButtonPressed(1))
	{
		MousePos p;
		vector2 pos;
		switch(m_activeWeapon)
		{
			case BOMB:
				if (m_pFighterJet->m_state != INACTIVE)
				{
					printf("Airstrike already in process\n");
					break;
				}
				// check if its on the radar
				p	= m_input.GetMousePos();
				pos = m_pCaching->RadarToWorldPos(vector2(p.x, SCREEN_H - p.y));
				// Pos returns 99999 if out of bounds
				if (pos.x < 90000)
				{
					m_activeWeapon = GUN;
					weaponChanged = true;
					m_pFighterJet->CarpetBomb(m_pCamera->GetHorizPosition(), pos);
				}
				break;
			case GUN:

				break;
		}
	}
	if (m_input.IsButtonPressed(1))
	{
		switch(m_activeWeapon)
		{
			case BOMB:
				break;
			case GUN:
				m_isShooting = true;
				MousePos pos = m_input.GetMousePos();
				float val;

				// Get value in z-buffer
				glReadPixels((GLint)pos.x, (GLint)(SCREEN_H - pos.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &val);

				vector3 frag(pos.x, pos.y, val);
				// Derive inverse of view transform (could just use transpose of view matrix
				matrix4 inverse = rotate_tr(m_pCamera->m_rotate.y, .0f, 1.0f, .0f) 
								* rotate_tr(m_pCamera->m_rotate.x, 1.0f, .0f, .0f);

				// Request unprojected coordinate
				vector3 p = perspective_unproj_world(frag, float(SCREEN_W), float(SCREEN_H), NEAR_PLANE, FAR_PLANE, 1.0f, inverse)
						  + m_pCamera->m_translate;

				m_pDeform->EdgeDeform(m_coarsemap, vector2(p.x, p.z), vector4(.3f, -.02f, .0f, .0f), "Gaussian");
				//m_pCaching->DeformHighDetail(vector2(p.x, p.z), vector4(1.0f, .2f, .0f, .0f),
						//"Gaussian");
				break;
		}
	}else{
		m_isShooting = false;
	}

	// Change weapon
	if (weaponChanged)
	{
		switch(m_activeWeapon)
		{
			case BOMB:
				m_mouseLook = false;
				m_pCamera->m_pModel = NULL;
				break;
			case GUN:
				m_mouseLook = true;
				m_pCamera->m_pModel = m_pCamera->m_pGunModel;
				break;
		}

		CentreMouse(false);
	}

	// Centre mouse on screen if in mouse look mode
	if (m_mouseLook)
	{
		CentreMouse();
	}
}

//--------------------------------------------------------
// Centre the mouse to the middle of the screen, calculate the offset
void
DefTer::CentreMouse(bool compensate)
{
		int wx,wy,ww,wh;
		SDL_GetWindowPosition(m_pWindow, &wx, &wy);
		SDL_GetWindowSize(m_pWindow, &ww, &wh);
#ifdef WIN32
		POINT mousep;
		GetCursorPos(&mousep);

		if (compensate)
		{
			m_mouseCompensate.x = wx+ww/2 - mousep.x;
			m_mouseCompensate.y = wy+wh/2 - mousep.y;
		}
		else
		{
			m_mouseFirstMove = true;
		}

		SetCursorPos(wx+ww/2, wy+wh/2);
#else
		int px,py, dummy;


		Window pWin, pRoot;

		XSelectInput(m_X_dpy, m_X_root_win, KeyReleaseMask);
		XQueryPointer(m_X_dpy, m_X_root_win, &pRoot, &pWin, &px, &py, &dummy, &dummy, (unsigned int*)&dummy);
		XWarpPointer(m_X_dpy, NULL, m_X_root_win, 0, 0, 0, 0, wx+ww/2, wy+wh/2);
		XSync(m_X_dpy, False);
		XFlush(m_X_dpy);	
		if (compensate)
		{
			m_mouseCompensate.x = wx+ww/2 - px;
			m_mouseCompensate.y = wy+wh/2 - py;
		}
		else
		{
			m_mouseFirstMove = true;
		}
#endif
}

//--------------------------------------------------------
void
DefTer::EditModeInput(float dt, vector2 mouseDelta, int ticks){
	// Rotate Camera
	if (m_input.IsButtonPressed(1))
	{	
		// Pitch
		m_pCamera->m_rotate.x -= dt * mouseDelta.y * PI * 0.1f;
		// Yaw
		m_pCamera->m_rotate.y -= dt * mouseDelta.x * PI * 0.1f;

		// Clamp the camera to prevent the user flipping
		// upside down messing up everything
		if (m_pCamera->m_rotate.x < -PI * 0.5f)
			m_pCamera->m_rotate.x = -PI * 0.5f;
		if (m_pCamera->m_rotate.x > PI * 0.5f)
			m_pCamera->m_rotate.x = PI * 0.5f;
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
		matrix4 inverse = rotate_tr(m_pCamera->m_rotate.y, .0f, 1.0f, .0f) 
						* rotate_tr(m_pCamera->m_rotate.x, 1.0f, .0f, .0f);
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
			p				+= m_pCamera->m_translate;
			m_clickPos		 = vector2(p.x, p.z);
			m_clicked		 = true;
		}

		//Update the clicked position in shaders, etc...
		UpdateClickPos();
	}
	

	// Change the selected deformation location
	if (m_clicked && ticks != 0)
	{
		//vector2 clickDiff	 = m_clickPos - vector2(m_cam_translate.x, m_cam_translate.z);
		vector4 stampSIRM	 = m_stampSIRM;
		stampSIRM.y			*= ticks;

		// Perform either  a HD or coarse deformation
		if (m_is_hd_stamp)
		{
			m_pCaching->DeformHighDetail(m_clickPos, stampSIRM);
		}
		else
		{
			//EdgeDeform(m_clickPos, stampSIRM);
			m_pDeform->EdgeDeform(m_coarsemap, m_clickPos, stampSIRM);

			// Once this is finally complete, change variables relating to streaming the coarsemap
			// to the CPU for collision detection
			// Restart timer
			m_deformTimer.start();
			m_XferWaitState = READY;
		}

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
	
	// Toggle the stamp values
	if (m_input.WasKeyPressed(SDLK_RIGHTBRACKET))
		printf("Stamp: %s\n", GetStampMan()->NextStamp().c_str());
	else if (m_input.WasKeyPressed(SDLK_LEFTBRACKET))
		printf("Stamp: %s\n", GetStampMan()->PrevStamp().c_str());

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

	// Apply a shockwave
	if (m_input.WasKeyPressed(SDLK_F5))
		m_pShockwave->CreateShockwave(m_clickPos, 100.0f);
}

//--------------------------------------------------------
void
DefTer::Logic(float dt)
{
	float terrain_height;

	terrain_height = InterpHeight(m_pCamera->GetHorizPosition());

	// Increase game speed
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		dt *= 5.0f;

	// Update the caching system
	m_pCaching->Update(m_pCamera->GetHorizPosition(), vector2(-m_pCamera->m_rotate.x, -m_pCamera->m_rotate.y));

	// Global environment forces
	if (m_gravity_on)
		m_pCamera->m_frameAcceleration += GRAVITY;
	for (list<GameEntity*>::iterator i = m_bombs.begin(); i != m_bombs.end(); i++){
		(*i)->m_frameAcceleration += GRAVITY;
	}

	// Use a fixed time-step for physics, so that the more accurate Verlet method can be used
	static float compoundDT = .0f;
	compoundDT += dt;
	while (compoundDT > DT){
		// Update camera position
		m_pCamera->Update();

		// Update bombs
		for (list<GameEntity*>::iterator i = m_bombs.begin(); i != m_bombs.end(); i++){
			(*i)->Update();
		}
		
		// Update jet
		m_pFighterJet->Update();

		WrapEntity(m_pCamera);

		// Don't let player go under the terrain
		terrain_height = InterpHeight(m_pCamera->GetHorizPosition());

		if (m_pCamera->m_translate.y < terrain_height)
		{
			m_pCamera->m_translate.y = terrain_height;
			m_pCamera->m_lastTranslate.y = terrain_height;
			m_pCamera->m_onGround	= true;
		} else{
			m_pCamera->m_onGround	= false;
		}

		// Check if bombs hit terrain
		for (list<GameEntity*>::iterator i = m_bombs.begin(); i != m_bombs.end(); i++){
			if ((*i)->m_translate.y < terrain_height /*- EYE_HEIGHT*/){
				float c[] = { 100.0f, -.6f, .0f,  .0f};
				m_XferWaitState = READY;
				vector4 SIRM(c);
				m_pDeform->EdgeDeform(m_coarsemap, (*i)->GetHorizPosition(), SIRM, "Gaussian");
				SIRM.x = 30.0f;
				SIRM.y = .15f;
				m_pDeform->EdgeDeform(m_coarsemap, (*i)->GetHorizPosition(), SIRM, "Mess");
				m_pShockwave->CreateShockwave((*i)->GetHorizPosition(), 400.0f, .2f);
				// Flash screen
				FlashScreen();
				delete (*i);
				i = m_bombs.erase(i);
				if (i==m_bombs.end())
					break;
			}
		}

		compoundDT -= DT;
	}

	// If the camera is moving we may need to drag the selection to within the HD Aura
	UpdateClickPos();

	// Create footprints
	m_footprintDT += dt;
	if (m_drawing_feet && m_gravity_on && m_pCamera->GetVelocity().Mag2() > .025f)
	{ 
		if (m_pCamera->m_translate.y - EYE_HEIGHT - terrain_height < .1f && m_footprintDT > STEP_TIME){
			vector4 stampSIRM= vector4(0.5f, 2.0f, -m_pCamera->m_rotate.y, m_flipFoot ? 1.0f : 0.0f);
			vector2 foot 	 = m_pCamera->GetHorizPosition();
			foot 			+= rotate_tr2(-m_pCamera->m_rotate.y) * vector2(m_flipFoot ? 0.3f : -0.3f, 0.0f);
			m_footprintDT 	 = 0.0f;
			m_flipFoot		^= true;
			m_pCaching->DeformHighDetail(foot, stampSIRM, "Footprint");
		}
	}

	// Update the shockwave class
	m_pShockwave->Update(dt);


	// Pass the camera's texture coordinates and the shift amount necessary
	// cam = x and y   ;  shift = z and w
	vector3 pos 	  = m_pCamera->m_translate * m_pClipmap->m_metre_to_tex;
	m_clipmap_shift.x = -fmodf(m_pCamera->m_translate.x, 32*m_pClipmap->m_quad_size);
	m_clipmap_shift.y = -fmodf(m_pCamera->m_translate.z, 32*m_pClipmap->m_quad_size);

	m_shManager->UpdateUni1f("cam_height", m_pCamera->m_translate.y);
	m_shManager->UpdateUni4f("cam_and_shift", pos.x, pos.z, m_clipmap_shift.x, m_clipmap_shift.y);

	// Reset forces for next frame
	m_pCamera->m_frameAcceleration.set(.0f);
	for (list<GameEntity*>::iterator i = m_bombs.begin(); i != m_bombs.end(); i++){
		(*i)->m_frameAcceleration.set(.0f);
	}

	// Animate the on-screen flash
	if (m_flash.enabled)
	{
		float elapsed = m_flash.timer.peekElapsed();
		if (elapsed > m_flash.inlength + m_flash.outlength)
			m_flash.enabled = false;
		else
		{
			float dmid = elapsed - m_flash.inlength;
			float dend = dmid - m_flash.outlength;
			if (elapsed < m_flash.inlength)
				m_flash.color.w = m_flash.maxAlpha * (1.0f - dmid*dmid/(m_flash.inlength*m_flash.inlength));
			else
				m_flash.color.w = m_flash.maxAlpha * (1.0f - dmid*dmid/(m_flash.outlength*m_flash.outlength));
		}
	}
}

//--------------------------------------------------------
void
DefTer::Render(float dt)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	matrix4 rotate, rotateclamp, cullviewproj, viewproj, translate;

	translate= translate_tr(.0f, -m_pCamera->m_translate.y, .0f);
	rotateclamp  = rotate_tr(max(.0f, -m_pCamera->m_rotate.x), 1.0f, .0f, .0f) 
				 * rotate_tr(-m_pCamera->m_rotate.y, .0f, 1.0f, .0f);
	cullviewproj = m_proj_mat * rotateclamp * translate;

	rotate   = rotate_tr(-m_pCamera->m_rotate.x, 1.0f, .0f, .0f) 
			 * rotate_tr(-m_pCamera->m_rotate.y, .0f, 1.0f, .0f);
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

	// If the ground is clicked then show the hologram
	if (m_clicked)
	{
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, GetStampMan()->GetCurrentStamp()->GetTexID());
		// Calculate the transform matrix for the stamp, scale it up and then rotate it as need be
		matrix2 transform = (m_coarsemap_dim * CLIPMAP_RES / m_stampSIRM.x) * rotate_tr2(-m_stampSIRM.z);
		// Mirror the stamp transform if need be
		if (m_stampSIRM.w != 0.0f)
			transform = reflect_tr2(true) * transform;
		m_shManager->UpdateUniMat2fv("stampTransform", transform.m);
	}

	BEGIN_PROF;

	// Enable wireframe if needed
	if (WIREFRAMEON)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_pClipmap->render_inner();
	// Switch to the simple shader and render the rest
	m_shManager->SetActiveShader(m_shmSimple);
	m_pClipmap->render_levels();

	glActiveTexture(GL_TEXTURE0);
	//glDisable(GL_CULL_FACE);
	RenderModel(m_pCamera, rotate*translate_tr(-m_pCamera->m_translate));
	for (list<GameEntity*>::iterator i = m_bombs.begin(); i != m_bombs.end(); i++)
		RenderModel((*i), rotate*translate_tr(-m_pCamera->m_translate));
	if (m_pFighterJet->m_state != INACTIVE)
		RenderModel(m_pFighterJet, rotate*translate_tr(-m_pCamera->m_translate));
	//glEnable(GL_CULL_FACE);
	
	// Disable wireframe if was enabled
	if (WIREFRAMEON)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_pSkybox->render(viewproj);
	m_pCaching->Render();
	END_PROF;

	// Get the lastest version of the coarsemap from the GPU for the next frame
	UpdateCoarsemapStreamer();

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	if (m_flash.enabled)
	{
		glUseProgram(m_shFlash->m_programID);
		glUniform4fv(glGetUniformLocation(m_shFlash->m_programID, "color"),  1, m_flash.color.v);
		glBindVertexArray(GetStandardVAO());

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	

	}

	if (m_useMode == GAME_MODE)
	{
		// Draw HUD elements for game mode
		if (m_activeWeapon == BOMB)
		{
			// Draw X at mouse cursor when BOMB is weapon (unless there is a bomb request already active)
			matrix2 transform = rotate_tr2(.0f) * scale_tr2(0.02f, ASPRAT*.02f);
			
			vector2 p((float*)&m_input.GetMousePos());
			p.x = p.x/SCREEN_W * 2.0f - 1.0f;
			p.y = 1.0f - p.y/SCREEN_H * 2.0f;
			
			glBindTexture(GL_TEXTURE_2D, m_bombXTex);
			glUseProgram(m_shHUD->m_programID);
			glUniformMatrix2fv(glGetUniformLocation(m_shHUD->m_programID, "transform"), 1, GL_FALSE, transform.m);
			glUniform2f(glGetUniformLocation(m_shHUD->m_programID, "offset"),  p.x, p.y);
			glBindVertexArray(GetStandardVAO());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		else if (m_activeWeapon == GUN)
		{
			// Draw the crosshair for the gun
			matrix2 transform = rotate_tr2(.0f) * scale_tr2(0.04f, ASPRAT*.04f);
			
			glBindTexture(GL_TEXTURE_2D, m_crosshairTex);
			glUseProgram(m_shHUD->m_programID);
			glUniformMatrix2fv(glGetUniformLocation(m_shHUD->m_programID, "transform"), 1, GL_FALSE, transform.m);
			glUniform2f(glGetUniformLocation(m_shHUD->m_programID, "offset"),  .0f, .0f);
			glBindVertexArray(GetStandardVAO());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			
			if (m_isShooting){
				// Draw a muzzle flash
				transform = (float(rand())/RAND_MAX*.5f+.5f) * scale_tr2(0.2f, ASPRAT*.2f) * rotate_tr2(float(rand())/RAND_MAX * PI) ;
				
				glBindTexture(GL_TEXTURE_2D, m_muzzleFlashTex);
				glUseProgram(m_shHUD->m_programID);
				glUniformMatrix2fv(glGetUniformLocation(m_shHUD->m_programID, "transform"), 1, GL_FALSE, transform.m);
				glUniform2f(glGetUniformLocation(m_shHUD->m_programID, "offset"),  .19f, -.15f);
				glBindVertexArray(GetStandardVAO());
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}


		}

		if (m_pFighterJet->m_state != INACTIVE)
		{
			// Draw the bomb X texture at the current bomb target
			vector2 p = m_pCaching->WorldPosToRadar(m_pFighterJet->m_target);
			p.x = p.x/SCREEN_W * 2.0f - 1.0f;
			p.y = p.y/SCREEN_H * 2.0f - 1.0f;
			matrix2 transform = rotate_tr2(.0f) * scale_tr2(0.02f, ASPRAT*.02f);
			
			
			glBindTexture(GL_TEXTURE_2D, m_bombXTex);
			glUseProgram(m_shHUD->m_programID);
			glUniformMatrix2fv(glGetUniformLocation(m_shHUD->m_programID, "transform"), 1, GL_FALSE, transform.m);
			glUniform2f(glGetUniformLocation(m_shHUD->m_programID, "offset"),  p.x, p.y);
			glBindVertexArray(GetStandardVAO());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}
	else{
		// Render the regular arrow cursor of size 70p x 70p
		vector2 pixel_size	= vector2(70.0f, 70.0f * ASPRAT) / SCREEN_W;
		vector2 hotspot		= vector2(.227f * 70.0f, -.461f * 70.0f) / SCREEN_W;

		matrix2 transform = rotate_tr2(.0f) * scale_tr2(pixel_size * .5f );
		
		vector2 p((float*)&m_input.GetMousePos());
		p.x = p.x/SCREEN_W * 2.0f - 1.0f;
		p.y = 1.0f - p.y/SCREEN_H * 2.0f;
		
		glBindTexture(GL_TEXTURE_2D, m_cursorTex);
		glUseProgram(m_shHUD->m_programID);
		glUniformMatrix2fv(glGetUniformLocation(m_shHUD->m_programID, "transform"), 1, GL_FALSE, transform.m);
		glUniform2f(glGetUniformLocation(m_shHUD->m_programID, "offset"),  p.x + hotspot.x, p.y + hotspot.y);
		glBindVertexArray(GetStandardVAO());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	// Swap windows to show the rendered data
	SDL_GL_SwapWindow(m_pWindow);
}

//--------------------------------------------------------
void
DefTer::RenderModel(GameEntity* pEnt, matrix4 view)
{
	// If no model active then return
	if (!pEnt->m_pModel)
		return;

	matrix4 model_tr;

	model_tr = translate_tr(pEnt->m_translate)
			 * rotate_tr(pEnt->m_rotate.z, .0f, .0f, 1.0f)
			 * rotate_tr(pEnt->m_rotate.y, .0f, 1.0f, .0f)
			 * rotate_tr(pEnt->m_rotate.x, 1.0f, .0f, .0f)
			 * scale_tr(pEnt->m_scale);
	
	glUseProgram(m_shModel->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shModel->m_programID, "view"), 1, GL_FALSE, view.m);

	RenderNode(pEnt->m_pModel, view * model_tr);
}

//--------------------------------------------------------
void
DefTer::RenderNode(Node* pNode, matrix4 parent_tr)
{
	matrix4 model_tr;

	if (pNode->m_transform.valid)
	{
		model_tr = pNode->m_transform.cache;
	}
	else
	{
		model_tr = translate_tr(pNode->m_transform.translate)
				 * rotate_tr(pNode->m_transform.rotate.z, .0f, .0f, 1.0f)
				 * rotate_tr(pNode->m_transform.rotate.y, .0f, 1.0f, .0f)
				 * rotate_tr(pNode->m_transform.rotate.x, 1.0f, .0f, .0f)
				 * scale_tr(pNode->m_transform.scale);
	}
	pNode->m_transform.cache = model_tr;
	pNode->m_transform.valid = true;

	matrix4 transform = parent_tr * model_tr;
	
	if (pNode->m_pSibling)
		RenderNode(pNode->m_pSibling, parent_tr);
	if (pNode->m_pChild)
		RenderNode(pNode->m_pChild, transform);

	glBindTexture(GL_TEXTURE_2D, pNode->m_mesh.tex);
	glBindVertexArray(pNode->m_mesh.vao);
	glUniformMatrix4fv(glGetUniformLocation(m_shModel->m_programID, "mvp"), 
			1, GL_FALSE, (m_proj_mat * transform).m);
	glUniformMatrix4fv(glGetUniformLocation(m_shModel->m_programID, "modelview"), 1, GL_FALSE, transform.m);

	if (pNode->m_mesh.tex==0)
		glUniform1i(glGetUniformLocation(m_shModel->m_programID, "useTex"), 0); 
	else
		glUniform1i(glGetUniformLocation(m_shModel->m_programID, "useTex"), 1);
	glUniform3fv(glGetUniformLocation(m_shModel->m_programID, "diffuseC"), 1, pNode->m_mesh.diffuse.v);
	glUniform3fv(glGetUniformLocation(m_shModel->m_programID, "ambientC"), 1, pNode->m_mesh.ambient.v);
	glUniform4f(glGetUniformLocation(m_shModel->m_programID, "specularC"), 
			pNode->m_mesh.specular.x, pNode->m_mesh.specular.y, pNode->m_mesh.specular.z,
			pNode->m_mesh.specPower);

	glDrawElements(GL_TRIANGLES, pNode->m_mesh.nIndices, GL_UNSIGNED_INT, 0);
	
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
		memcpy(main->m_elevationDataBuffer, main->m_bufferPtr, main->m_coarsemap_dim*main->m_coarsemap_dim * sizeof(float));

		// finally lock the data array, and swap the pointers
		SDL_mutexP(main->m_elevationDataMutex);
		float * temp 				= main->m_elevationData;
		main->m_elevationData 		= main->m_elevationDataBuffer;
		main->m_elevationDataBuffer = temp;
		main->m_XferState 			= DONE;
		DEBUG("Retriever took %.3fms to copy into sys mem\n", copyTimer.getElapsed() * 1000);
		SDL_mutexV(main->m_elevationDataMutex);
	}

	return 0;
}

//--------------------------------------------------------
void
DefTer::WrapEntity(GameEntity* pEnt){
	// Boundary check for wrapping position
	static float boundary = m_coarsemap_dim* m_pClipmap->m_quad_size * .5f;
	if (pEnt->m_translate.x > boundary){
		pEnt->m_translate.x -= boundary * 2.0f;
		pEnt->m_lastTranslate.x  -= boundary * 2.0f;
	}
	else if (pEnt->m_translate.x < -boundary){
		pEnt->m_translate.x += boundary * 2.0f;
		pEnt->m_lastTranslate.x  += boundary * 2.0f;
	}
	if (pEnt->m_translate.z > boundary){
		pEnt->m_translate.z -= boundary * 2.0f;
		pEnt->m_lastTranslate.z  -= boundary * 2.0f;
	}
	else if (pEnt->m_translate.z < -boundary){
		pEnt->m_translate.z += boundary * 2.0f;
		pEnt->m_lastTranslate.z  += boundary * 2.0f;
	}
}

//--------------------------------------------------------
void
DefTer::FlashScreen		(float inTime, float outTime, float maxAlpha, vector4 color)
{
	m_flash.enabled		= true;
	m_flash.timer.start();
	m_flash.inlength	= inTime;
	m_flash.outlength	= outTime;
	m_flash.maxAlpha	= maxAlpha;
	m_flash.color		= color;
}

//--------------------------------------------------------
void 
DefTer::InitCursor(){
#ifndef WIN32
	/* vars to make blank cursor */
	Pixmap blank;
	XColor dummy;
	char data[1] = {0};
	m_X_dpy = XOpenDisplay(0);
	m_X_root_win = XRootWindow(m_X_dpy, 0);

	/* make a blank cursor */
	blank = XCreateBitmapFromData (m_X_dpy, m_X_root_win, data, 1, 1);
	if(blank == None) fprintf(stderr, "error: out of memory.\n");
	m_X_cursor = XCreatePixmapCursor(m_X_dpy, blank, blank, &dummy, &dummy, 0, 0);
	XDefineCursor(m_X_dpy, m_X_root_win, m_X_cursor);
	XFreePixmap (m_X_dpy, blank);
	XSync(m_X_dpy, False);
	XFlush(m_X_dpy);	
#else
	ShowCursor(0);
#endif
}

