/*****************************************************************************
 * caching: Caching system for level of detail and loading of tiles
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include <map>
using namespace std;
#include "re_shader.h"
#include "deform.h"
#include <list>
#include <queue>
#include <assert.h>
#include "caching.h"
#include "FreeImage.h"
#ifdef _WIN32
#	include "direct.h"
#	define	mkdir(x)	_mkdir(x)
#else
#	include "sys/stat.h"
#	define  mkdir(x)	mkdir(x, S_IRWXU)
#endif

#define WRAP(val, dim) 		((val < 0) ? (val + dim) : ((val > (dim - 1)) ? (val - dim) : val))
#define OFFSET(x, y, dim) 	((y) * dim + (x))

#define INITOFFSET		(0)

extern const int SCREEN_W;
extern const int SCREEN_H;
extern const float ASPRAT;

#define LOAD_CYCLES		(4)
#define	UNLOAD_CYCLES	(2)
#define READS_PER_FRAME	(3)
#define LOCK(m)			{ if (SDL_LockMutex(m) == -1) fprintf(stderr, "Mutex Lock Error\n"); }
#define UNLOCK(m)		{ if (SDL_UnlockMutex(m) == -1) fprintf(stderr, "Mutex Unlock Error\n"); }

#define RADAR_OFFSET	(20.0f)
#define RADAR_SIZE		(200.0f)
#define RADAR2_SIZE		(RADAR_SIZE / 2.0f)
#define RADAR_LINE_W	(1.0f / RADAR_SIZE)
#define RADAR2_LINE_W	(1.0f / RADAR2_SIZE)
#define RADAR_DOT_R		(16.0f / (RADAR_SIZE * RADAR_SIZE))
#define RADAR2_DOT_R	(16.0f / (RADAR2_SIZE * RADAR2_SIZE))

#define DEBUG_ON		(0)
#if DEBUG_ON
	#define DEBUG(x)		printf(x)
	#define DEBUG2(x,y)		printf(x,y)
	#define DEBUG3(x,y,z)	printf(x,y,z)
#else
	#define DEBUG(x)		{}
	#define DEBUG2(x,y)		{}
	#define DEBUG3(x,y,z)	{}
#endif

//--------------------------------------------------------
Caching::Caching(Deform* pDeform, int clipDim, int coarseDim, float clipRes, int highDim, float highRes)
{
	// Variables
	m_coarseDim		= coarseDim;
	m_highDim		= highDim;
	m_pDeform		= pDeform;

	// Calculate the tile size and grid dimensions
	m_TileSize		= highDim * highRes;
	float temp		= ((coarseDim * clipRes) / m_TileSize);
	m_GridSize		= (int)temp;

	// Create the grid
	m_Grid			= new Tile[m_GridSize*m_GridSize];

	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		m_Grid[i].m_texID			= -1;
		m_Grid[i].m_modified		= false;
		m_Grid[i].m_LoadedPrevious	= false;
		m_Grid[i].m_LoadedCurrent	= false;
		m_Grid[i].m_row				= i / m_GridSize;
		m_Grid[i].m_col				= i % m_GridSize;
		memset(&m_Grid[i].m_texdata, 0, sizeof(TexData));
	}

	// Calculate the band values
	m_BandWidth		= highDim * highRes * .1f;
	m_BandPercent	= m_BandWidth / m_TileSize;

	// Calculate the offset value for the coarsemap to allow determining of tile index
	m_CoarseOffset	= coarseDim * clipRes * 0.5f;

	// 
	m_metre_to_tex	= 1.0f / (coarseDim * clipRes);

	// Set default values
	m_RegionPrevious	= 0;
	m_TileIndexPrevious	= vector2(.0f);
	m_caching_stats = "";

	// Save out some cool stats about the caching system
	stringstream sstr;
	sstr << "Grid Size:\t\t\t"	<< m_GridSize << "\n";
	sstr << "Tile Size:\t\t\t"	<< m_TileSize << "m\n";
	sstr << "Band Width:\t\t\t"	<< m_BandWidth << "m\n";
	sstr << "Band %:\t\t\t\t"	<< (m_BandPercent * 100) << "%\n";
	m_caching_stats += sstr.str();

	//--------------------------------------------------------
	// Radar Setup
	//--------------------------------------------------------
	// Setup shader
	m_shRadar = new ShaderProg("shaders/radar.vert", "", "shaders/radar.frag");
	glBindAttribLocation(m_shRadar->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shRadar->m_programID, 1, "vert_TexCoord");
	m_shRadar->CompileAndLink();

	m_cellSize			= 1.0f / m_GridSize;
	m_radar_pos			= vector2(float(SCREEN_W), float(SCREEN_H)) - vector2(RADAR_OFFSET + RADAR_SIZE);
	m_radar2_pos		= m_radar_pos + vector2(RADAR2_SIZE / 2.0f, -(RADAR2_SIZE + RADAR_OFFSET));

	// Vertex positions
	GLfloat square[]	= { -1.0f, -1.0f,
							 1.0f, -1.0f,
							 1.0f,  1.0f,
							-1.0f,  1.0f };
	// Texcoords are upside-down to mimic the systems coordinates
	GLfloat texcoords[]	= { 0.0f, 1.0f,
							1.0f, 1.0f,
							1.0f, 0.0f,
							0.0f, 0.0f };
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
	//--------------------------------------------------------

	// Create the default Zero-texture
	GLushort* zeroData = new GLushort[highDim * highDim];
	glGenTextures(1, &m_zeroTex.heightmap);
	glGenTextures(1, &m_zeroTex.pdmap);
	glBindTexture(GL_TEXTURE_2D, m_zeroTex.heightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	memset(zeroData, 0, highDim*highDim);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, highDim, highDim, 0, GL_RED, GL_UNSIGNED_SHORT, zeroData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_zeroTex.pdmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, highDim, highDim, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] zeroData;
	m_pDeform->create_pdmap(m_zeroTex, false);

	// Create the block of 9 texture IDs
	glGenTextures(9, m_cacheHeightmapTex);
	glGenTextures(9, m_cachePDTex);
	for (int i = 0; i < 9; i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_cacheHeightmapTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, highDim, highDim, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, m_cachePDTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, highDim, highDim, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		TexData texID = { 
			m_cacheHeightmapTex[i],
			m_cachePDTex[i],
		};
		m_texQueue.push(texID);
	}

	// Intitialise threading constructs
	m_threadRunning 		= true;
	m_loadQueueMutex		= SDL_CreateMutex();
	m_unloadQueueMutex		= SDL_CreateMutex();
	m_doneLoadQueueMutex	= SDL_CreateMutex();
	m_doneUnloadQueueMutex	= SDL_CreateMutex();
	
	// Initialise PBO pool
	int texSize = sizeof(GLbyte) * highDim * highDim;
	glGenBuffers(PBO_POOL * 2, m_pbos);
	for (int i = 0 ; i < PBO_POOL; i++)
	{
		GLuint pbo;

		// Pack
		pbo = m_pbos[i];
		m_pboPackPool.push(pbo);

		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, texSize, NULL, GL_STREAM_READ);

		// Unpack
		pbo = m_pbos[i+PBO_POOL];
		m_pboUnpackPool.push(pbo);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, texSize, NULL, GL_STREAM_DRAW);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	// Create and launch cache thread
	m_cacheThread 		= SDL_CreateThread(hdd_cacher, (void*)this);
}

//--------------------------------------------------------
Caching::~Caching()
{
	// Unload all currently loaded files
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		if (m_Grid[i].m_modified)
			Unload(m_Grid + i);
	}

	// Wait until all the data has been unloaded if must
	do
	{
		UpdatePBOs();
		SDL_Delay(10);
	} while(m_readyUnloadQueue.size() > 0 ||
			m_busyUnloadQueue.size() > 0 ||
			m_unloadQueue.size() > 0 ||
			m_doneUnloadQueue.size() > 0);

	RE_DELETE(m_shRadar);
	glDeleteBuffers(3, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	m_threadRunning = false;
	SDL_WaitThread(m_cacheThread, NULL);
	SDL_DestroyMutex(m_doneLoadQueueMutex);
	SDL_DestroyMutex(m_doneUnloadQueueMutex);
	SDL_DestroyMutex(m_loadQueueMutex);
	SDL_DestroyMutex(m_unloadQueueMutex);
	glDeleteTextures(1, &m_zeroTex.heightmap);
	glDeleteTextures(1, &m_zeroTex.pdmap);
	delete[] m_Grid;

	// Delete PBO pool
	glDeleteBuffers(PBO_POOL*2, m_pbos);

	// Delete texture ID pool
	glDeleteTextures(9, m_cacheHeightmapTex);
	glDeleteTextures(9, m_cachePDTex);
}

//--------------------------------------------------------
void
Caching::Init(GLuint coarsemapTex, GLuint coarsemapColorTex, vector2 worldPos)
{
	m_coarsemapTex 		= coarsemapTex;
	m_coarsemapColorTex = coarsemapColorTex;

	// Call a preliminary update to initialise all the constructs
	// Repeat this until all cache files have been loaded
	do
	{
		Update(worldPos, vector2(0.0f));
		SDL_Delay(10);
	} while(m_readyLoadQueue.size() > 0);
}

//--------------------------------------------------------
void
Caching::Update(vector2 worldPos, vector2 cam_rotation)
{
	// Store the camera rotation
	m_cam_rotation		= cam_rotation;

	// Get the world position and calculate the tile position
	m_worldPos			= worldPos + vector2(m_CoarseOffset);
	vector2 tilePos		= (m_worldPos / m_TileSize);
	// Get the tile index into the array
	// X = Column : Y = Row
	m_TileIndexCurrent	= tilePos.Floor();

	// Sift out just the fractional part to find location within the tile and offset to centre
	// So that positive -> right of centre or below centre and negative left or above
	tilePos -= (m_TileIndexCurrent + vector2(0.5f));

	// Do this so we can check absolute distance from centre
	vector2 absTilePos = tilePos.Abs();

	// Center block
	if (absTilePos < vector2(m_BandPercent * 0.5f))
	{
		m_RegionCurrent = 4;
	}
	// Vertical Band
	else if (absTilePos.x < m_BandPercent * 0.5f)
	{
		if (tilePos.y < 0)
			m_RegionCurrent = 1;
		else
			m_RegionCurrent = 7;
	}
	// Horizontal Band
	else if (absTilePos.y < m_BandPercent * 0.5f)
	{
		if (tilePos.x < 0)
			m_RegionCurrent = 3;
		else
			m_RegionCurrent = 5;
	}
	// Quads
	else 
	{
		if (tilePos.x < 0.0f)
		{
			if (tilePos.y < 0.0f)
				m_RegionCurrent = 0;
			else
				m_RegionCurrent = 6;
		}
		else
		{
			if (tilePos.y < 0.0f)
				m_RegionCurrent = 2;
			else
				m_RegionCurrent = 8;
		}
	}

	if (m_RegionCurrent != m_RegionPrevious)
	{
		UpdateTiles(false, m_RegionPrevious, m_TileIndexPrevious);
		UpdateTiles(true,  m_RegionCurrent,  m_TileIndexCurrent);

		// Check all tiles for loading/unloading
		for (int i = 0; i < m_GridSize * m_GridSize; i++)
		{
			// If it was loaded
			if (m_Grid[i].m_LoadedPrevious)
			{
				// But need not be loaded anymore
				if (!m_Grid[i].m_LoadedCurrent)
				{
					Unload(m_Grid + i);
				}
			}
			// If it wasn't loaded
			else
			{
				// But needs to be!!!
				if (m_Grid[i].m_LoadedCurrent)
				{
					Load(m_Grid + i);
				}
			}

			//Set the previous state equal to the current one
			m_Grid[i].m_LoadedPrevious = m_Grid[i].m_LoadedCurrent;
		}
	}

	// Identify the new tile region
	m_RegionPrevious 	= m_RegionCurrent;
	m_TileIndexPrevious	= m_TileIndexCurrent;

	// Update the PBO's
	UpdatePBOs();

	// Unbind the buffers
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

//--------------------------------------------------------
void
Caching::DeformHighDetail(vector2 clickPos, string stampName, vector4 stampSIRM)
{
	// Get the tile index into the array
	// X = Column : Y = Row
	vector2 tileIndex	= ((vector2(m_CoarseOffset) + clickPos) / m_TileSize).Floor();
	
	// Give the clickPos within the tile
	clickPos.x = (fmodf((m_CoarseOffset + clickPos.x), m_TileSize));
	clickPos.y = (fmodf((m_CoarseOffset + clickPos.y), m_TileSize));

	// Grab the tile struct
	int index = (int)OFFSET(WRAP(tileIndex.x, m_GridSize), WRAP(tileIndex.y, m_GridSize), m_GridSize);
	Tile& tile = m_Grid[index];

	// If it is in the drawable region
	if (m_Grid[index].m_texID != -1)	
	{
		m_Grid[index].m_modified = true;
		GLuint mapID = tile.m_texdata.heightmap;

		// If the tile already has a texture ID
		if (mapID != 0 && mapID != m_zeroTex.heightmap)
		{
			// Displace it here and now
			m_pDeform->displace_heightmap(tile.m_texdata, clickPos, vector2(0.0f), stampName, stampSIRM, false);
			m_pDeform->calculate_pdmap(tile.m_texdata, clickPos, vector2(0.0f), stampSIRM.x, false);
		}
		// If it's only using the Zero texture
		else if (mapID == m_zeroTex.heightmap)
		{
			// Need a new texture that can be rendered into for this tile
			if (m_texQueue.size())
			{
				TexData newID = m_texQueue.front();
				m_texQueue.pop();
				tile.m_texdata = newID;

				// Displace it now
				m_pDeform->displace_heightmap(tile.m_texdata, clickPos, vector2(0.0f), stampName, stampSIRM, false, m_zeroTex.heightmap);
				m_pDeform->create_pdmap(tile.m_texdata, false);
			}
			else
			{
				// Push this deform operation onto a queue that waits for a texture ID
				float YOU_MUST_STILL_HANDLE_THIS1 = 0;
				assert(YOU_MUST_STILL_HANDLE_THIS1!=0);
			}
		}
		// If it's still waiting for it's ID whilst loading
		else
		{
			// Push this deform operation onto a queue that waits for it's texture ID to be set
			float YOU_MUST_STILL_HANDLE_THIS2 = 0;
			assert(YOU_MUST_STILL_HANDLE_THIS2!=0);
		}
	}
	else
		printf("High Def out of bounds");
}

//--------------------------------------------------------
// Renders the radar into a small viewport on the screen
void
Caching::Render(void)
{
	// Variables for position
	vector2 worldPos	= m_worldPos;
	vector2 tilePos		= (m_worldPos / m_TileSize) - m_TileIndexCurrent;
	worldPos		   *= m_metre_to_tex;

	// Calculate the vision cone values
	matrix2 coneMat;
	coneMat[0] = cosf(m_cam_rotation.y);
	coneMat[1] = -sinf(m_cam_rotation.y);
	coneMat[2] = sinf(m_cam_rotation.y);
	coneMat[3] = cosf(m_cam_rotation.y);

	glUseProgram(m_shRadar->m_programID);
	glUniform1f(glGetUniformLocation(m_shRadar->m_programID, "aspectRatio"), ASPRAT);
	glUniformMatrix2fv(glGetUniformLocation(m_shRadar->m_programID, "viewRotation"), 1,	GL_FALSE, coneMat.m);
	// Variables
	vector2 linePos, offset;
	vector4 tileBounds, cellColor;

	// Store the current viewort
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set GL settings
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind the vertex array
	glBindVertexArray(m_vao);

	// Set the textures
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "colormap"), 1);
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "heightmap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_coarsemapTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_coarsemapColorTex);
	glActiveTexture(GL_TEXTURE0);

	// Change to the new viewport
	glViewport((int)m_radar_pos.x, (int)m_radar_pos.y, (int)RADAR_SIZE, (int)RADAR_SIZE);

	// Set initial uniforms
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "offset"), 1, offset.v);
	glUniform1f(glGetUniformLocation(m_shRadar->m_programID, "scale"), 1.0f);
	glUniform1f(glGetUniformLocation(m_shRadar->m_programID, "dotRadius"), RADAR_DOT_R);
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "currentPos"), 1, worldPos.v);

	// Do first pass to color in background
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 0);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);


	// Fill in all loaded texture cells, skipping the current active one
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 1);
	int curIndex = (int)m_TileIndexCurrent.y * m_GridSize + (int)m_TileIndexCurrent.x;
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		// Only draw tiles currently loaded onto the GPU
		if (m_Grid[i].m_LoadedCurrent)
		{
			// Use a different colour for tiles that are (in)active
			if (m_Grid[i].m_texID == -1)
				cellColor.set(1.0, 0.0, 0.0, 0.5);
			else
			{
				// Draw the current cell in a different color
				if (i == curIndex)
					cellColor.set(0.0, 1.0, 0.0, 0.5);
				else
					cellColor.set(1.0, 1.0, 1.0, 0.5);
			}

			// Send the shader the current colour to use
			glUniform4fv(glGetUniformLocation(m_shRadar->m_programID, "cellColor"), 1, cellColor.v);

			// Send the offsets for the cell to the shader
			tileBounds.set(m_Grid[i].m_col * m_cellSize, m_Grid[i].m_row * m_cellSize, 0.0f, 0.0f);
			tileBounds.z = tileBounds.x + m_cellSize;
			tileBounds.w = tileBounds.y + m_cellSize;
			glUniform4fv(glGetUniformLocation(m_shRadar->m_programID, "tileBounds"), 1, tileBounds.v);

			// Execute a shader pass
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
		}
	}


	// Draw the vision cone
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 2);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);


	// Set the shader to run the line drawing pass
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 3);
	// Loop over all the divisions and draw the lines
	for (int i = 0; i <= m_GridSize; i++)
	{
		linePos.set(m_cellSize * i);
		linePos.x -= RADAR_LINE_W;
		linePos.y += RADAR_LINE_W;
		glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "linePos"), 1, linePos.v);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
	}


	// Draw the current position as a dot
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 4);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);



	// Draw the second radar image
	glViewport((int)m_radar2_pos.x, (int)m_radar2_pos.y, (int)RADAR2_SIZE, (int)RADAR2_SIZE);

	// Set initial uniforms
	offset = m_TileIndexCurrent * m_cellSize;
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "offset"), 1, offset.v);
	glUniform1f(glGetUniformLocation(m_shRadar->m_programID, "scale"), m_cellSize);
	glUniform1f(glGetUniformLocation(m_shRadar->m_programID, "dotRadius"), RADAR2_DOT_R);
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "currentPos"), 1, tilePos.v);


	// Do first pass to color in background
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 0);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);


	// Draw the vision cone
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 2);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);


	// Set the shader to run the line drawing pass
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 3);
	// Draw the top & left boundary
	linePos.set(0.0f);
	linePos.x -= RADAR2_LINE_W;
	linePos.y += RADAR2_LINE_W;
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "linePos"), 1, linePos.v);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
	// Draw the first band line
	linePos.set(0.5f - (m_BandPercent * 0.5f));
	linePos.x -= RADAR2_LINE_W;
	linePos.y += RADAR2_LINE_W;
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "linePos"), 1, linePos.v);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
	// Draw the second band line
	linePos.set(0.5f + (m_BandPercent * 0.5f));
	linePos.x -= RADAR2_LINE_W;
	linePos.y += RADAR2_LINE_W;
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "linePos"), 1, linePos.v);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
	// Draw the bottom & right boundary
	linePos.set(1.0f);
	linePos.x -= RADAR2_LINE_W;
	linePos.y += RADAR2_LINE_W;
	glUniform2fv(glGetUniformLocation(m_shRadar->m_programID, "linePos"), 1, linePos.v);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);


	// Draw the current position as a dot
	glUniform1i(glGetUniformLocation(m_shRadar->m_programID, "pass"), 4);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);



	// Reset the GL settings
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	// Reset the viewport
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

//--------------------------------------------------------
void
Caching::UpdateTiles(bool newStatus, int region, vector2 TileIndex)
{
	switch (region)
	{
	case 0:
		//Top-Left
		m_LeftOffset = TileIndex - vector2(1.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(1.0f), vector2(2.0f));
		break;

	case 1:
		//Top-Centre
		m_LeftOffset = TileIndex - vector2(1.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(3.0f, 2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(0.0f, 1.0f), vector2(1.0f, 2.0f));
		break;

	case 2:
		//Top-Right
		m_LeftOffset = TileIndex - vector2(0.0f, 1.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(0.0f, 1.0f), vector2(2.0f));
		break;

	case 3:
		//Centre-Left
		m_LeftOffset = TileIndex - vector2(1.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(2.0f, 3.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(1.0f, 0.0f), vector2(2.0f, 1.0f));
		break;

	case 4:
		//Centre-Centre
		m_LeftOffset = TileIndex - vector2(1.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(3.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(1.0f));
		break;

	case 5:
		//Centre-Right
		m_LeftOffset = TileIndex - vector2(0.0f, 1.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(2.0f, 3.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(2.0f, 1.0f));
		break;

	case 6:
		//Bottom-Left
		m_LeftOffset = TileIndex - vector2(1.0f, 0.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(1.0f, 0.0f), vector2(2.0f));
		break;

	case 7:
		//Bottom-Centre
		m_LeftOffset = TileIndex - vector2(1.0f, 0.0f);
		SetLoadStatus(newStatus, m_LeftOffset, vector2(3.0f, 2.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(1.0f, 2.0f));
		break;

	case 8:
		//Bottom-Right
		m_LeftOffset = TileIndex;
		SetLoadStatus(newStatus, m_LeftOffset, vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(2.0f));
		break;

	default:
		break;
	}
}

//--------------------------------------------------------
// Returns the index of the the top-left active tile
void
Caching::GetActiveTiles(Tile activeTiles[4]){
	int row = int(m_LeftOffset.y);
	int col = int(m_LeftOffset.x);
	activeTiles[0] = m_Grid[OFFSET(WRAP(col    , m_GridSize), WRAP(row	   , m_GridSize), m_GridSize)];
	activeTiles[0].m_row = row;
	activeTiles[0].m_col = col;
	activeTiles[1] = m_Grid[OFFSET(WRAP(col + 1, m_GridSize), WRAP(row    , m_GridSize), m_GridSize)];
	activeTiles[1].m_row = row;
	activeTiles[1].m_col = col+1;
	activeTiles[2] = m_Grid[OFFSET(WRAP(col    , m_GridSize), WRAP(row	+ 1, m_GridSize), m_GridSize)];
	activeTiles[2].m_row = row+1;
	activeTiles[2].m_col = col;
	activeTiles[3] = m_Grid[OFFSET(WRAP(col + 1, m_GridSize), WRAP(row + 1, m_GridSize), m_GridSize)];
	activeTiles[3].m_row = row+1;
	activeTiles[3].m_col = col+1;
}

//--------------------------------------------------------
//Sets the status for the tiles around the given one
//Starts at TileIndex and iterates based on the value stored in size
void
Caching::SetLoadStatus(bool newStatus, vector2 TileIndex, vector2 size)
{
	for (float row = TileIndex.y; row < TileIndex.y + size.y; row++)
	{
		for (float col = TileIndex.x; col < TileIndex.x + size.x; col++)
		{
			int offset = (int)OFFSET(WRAP(col, m_GridSize), WRAP(row, m_GridSize), m_GridSize);

			m_Grid[offset].m_LoadedCurrent	= newStatus;
		}
	}
}

//--------------------------------------------------------
//Sets the texID for the tiles around the given one
//Starts at TileIndex and iterates based on the value stored in size
void
Caching::SetActiveStatus(bool newStatus, vector2 TileIndex, vector2 size)
{
	int curVal = INITOFFSET;
	for (float row = TileIndex.y; row < TileIndex.y + size.y; row++)
	{
		for (float col = TileIndex.x; col < TileIndex.x + size.x; col++)
		{
			int offset = (int)OFFSET(WRAP(col, m_GridSize), WRAP(row, m_GridSize), m_GridSize);

			m_Grid[offset].m_texID	= (newStatus ? curVal++ : -1);
		}
	}
}

//--------------------------------------------------------
// Called by the GL thread.  Puts the tile on the queue for loading
void
Caching::Load(Tile* tile)
{
	// Create struct
	CacheRequest load;
	load.type = LOAD;
	load.tile = tile;
	DEBUG3("LOAD %d %d\n", tile->m_row, tile->m_col);

	// Check for existing requests for this tile that negate this request and remove all
	/*for (list<CacheRequest>::iterator i = m_readyQueue.begin(); i != m_readyQueue.end(); i++)
	{
		if (i->tile == tile){
			i = m_readyQueue.erase(i);
			i--;
		}
	}*/

	load.m_cycles = 0;
	m_readyLoadQueue.push_back(load);
}

//--------------------------------------------------------
// Called by the GL thread.  Puts the tile on the queue for unloading
void
Caching::Unload(Tile* tile)
{
	// Only unload tiles that have been modified
	if (!tile->m_modified)
	{
		// Releases texdata if not the zero map
		if (tile->m_texdata.heightmap != m_zeroTex.heightmap)
			m_texQueue.push(tile->m_texdata);

		// Return to skip saving to disk
		return;
	}

	// reset modified status
	tile->m_modified = false;

	CacheRequest unload;
	unload.type = UNLOAD;
	unload.tile = tile;
	DEBUG3("UNLOAD %d %d\n", tile->m_row, tile->m_col);

	unload.m_waitCount = 0;
	unload.m_cycles = 0;
	m_readyUnloadQueue.push_back(unload);
}

//--------------------------------------------------------
// Allocates PBOs to Load/Unload requests (ready queue) and signals CPU queue
// also.. retrieves data readied by CPU (done queue)
// This is the only function that alters the PBO pools
void
Caching::UpdatePBOs()
{
	CacheRequest load;
	CacheRequest unload;

	// UNLOADING : Middle phase - Mapping the buffer to system memory to be written to disk
	for (int i = 0; i < 2 && m_busyUnloadQueue.size() ; i++)
	{
		unload = m_busyUnloadQueue.front();
		m_busyUnloadQueue.pop_front();

		// Now that it has been transferring to PBO for some time, try map to sys memory
		glBindBuffer(GL_PIXEL_PACK_BUFFER, unload.pbo);
		unload.ptr = (GLubyte*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		CheckError("Mapping unload/pack buffer");
		// if this failed, worry
		if (!unload.ptr)
		{
			fprintf(stderr, "BAD ERROR: Could not map unload PBO pointer!!!!!\n");
			m_busyUnloadQueue.push_front(unload);
			break;
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		LOCK(m_unloadQueueMutex);
		m_unloadQueue.push_back(unload);
		UNLOCK(m_unloadQueueMutex);
	}

	// LOADING : First phase - Acquire PBOs for loading from sys mem
	while (m_pboUnpackPool.size())
	{
		if (m_readyLoadQueue.size())
		{
			// pop request off queue
			load = m_readyLoadQueue.front();
			m_readyLoadQueue.pop_front();

			// pop PBO
			load.pbo = m_pboUnpackPool.front();
			m_pboUnpackPool.pop();

			// Map PBO to system memory for texture loading
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, load.pbo);
			load.ptr = (GLubyte*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
			CheckError("Mapping load/unpack buffer");
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			if (!load.ptr)
			{
				fprintf(stderr, "BAD ERROR: Could not map PBO pointer!!!!!\n");
				m_readyLoadQueue.push_front(load);
				m_pboUnpackPool.push(load.pbo);
				break;
			}

			// Push request onto the load queue for CPU
			LOCK(m_loadQueueMutex);
			m_loadQueue.push_back(load);
			UNLOCK(m_loadQueueMutex);
		}
		else
		{
			break;
		}
	}

	// UNLOADING : First phase - Acquire a PBO for the request
	list<CacheRequest> skip;
	while (m_pboPackPool.size())
	{
		if (m_readyUnloadQueue.size())
		{
			// pop request off queue
			unload = m_readyUnloadQueue.front();
			m_readyUnloadQueue.pop_front();
			// It's possible that it hasn't fully loaded before it needs to be unloaded
			if (unload.tile->m_texdata.heightmap==0)
			{
				unload.m_waitCount++;
				skip.push_back(unload);
				assert(unload.m_waitCount < 80);
				continue;
			}

			// pop PBO
			unload.pbo = m_pboPackPool.front();
			m_pboPackPool.pop();

			// Begin the transfer to PBO from GPU texture memory
			glBindBuffer(GL_PIXEL_PACK_BUFFER, unload.pbo);
			glBindTexture(GL_TEXTURE_2D, unload.tile->m_texdata.heightmap);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
			DEBUG("Transferring FROM texture\n");
			CheckError("Reading texture data from GPU to PBO");
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

			// Push request onto the busy queue to be checked next frame
			m_busyUnloadQueue.push_back(unload);
		}
		else
		{
			break;
		}
	}
	m_readyUnloadQueue.insert(m_readyUnloadQueue.begin(), skip.begin(), skip.end());

	// LOADING : Final phase - Texture memory transfer commences
	list<CacheRequest> loadsSkipped;
	for (int i = 0; i < READS_PER_FRAME; i++)
	{
		// Get a load-ready struct
		LOCK(m_doneLoadQueueMutex);
		if (m_doneLoadQueue.size())
		{
			load = m_doneLoadQueue.front();
			// before popping off the queue, check if there is an available tex ID
			if (!load.useZero && m_texQueue.size() == 0)
			{
				DEBUG ("---No available texture ID\n");
				UNLOCK(m_doneLoadQueueMutex);
				break;
			}
			m_doneLoadQueue.pop_front();
			// now check if we've allowed this tile a few cycles to load
			if (load.m_cycles < LOAD_CYCLES){
				load.m_cycles++;
				loadsSkipped.push_back(load);
				UNLOCK(m_doneLoadQueueMutex);
				continue;
			}
			DEBUG3("Tile %d %d is ready for upload\n", load.tile->m_row, load.tile->m_col);
			UNLOCK(m_doneLoadQueueMutex);
		}
		else
		{
			UNLOCK(m_doneLoadQueueMutex);
			break;
		}

		// Unmap the PBO from sysmem pointer
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, load.pbo);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		CheckError("Unmapping unpack buffer");
		// Begin transfer
		if (load.useZero)
		{
			DEBUG3("Using Zero texture for tile %d %d\n", load.tile->m_row, load.tile->m_col);
			load.tile->m_texdata = m_zeroTex;
		}
		else
		{
			// Get a texture ID
			load.tile->m_texdata = m_texQueue.front();
			m_texQueue.pop();
			DEBUG3("Transferring TO texture (%d, %d)\n",
					load.tile->m_texdata.heightmap, load.tile->m_texdata.normalmap);
			glBindTexture(GL_TEXTURE_2D, load.tile->m_texdata.heightmap);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_highDim, m_highDim, GL_RED, GL_UNSIGNED_BYTE, 0);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// Release the PBO for someone else to use next frame around. 
		// NB. if another request tries to remap this buffer, it will wait for our
		// transfer to finish -> don't worry that it's pushed onto the pool early
		m_pboUnpackPool.push(load.pbo);

		// Generate normals for this texture
		if (!load.useZero)
			m_pDeform->create_pdmap(load.tile->m_texdata, false);
	}
	LOCK(m_doneLoadQueueMutex);
	m_doneLoadQueue.insert(m_doneLoadQueue.begin(), loadsSkipped.begin(), loadsSkipped.end());
	UNLOCK(m_doneLoadQueueMutex);

	// UNLOADING : Final step of releasing PBO and TEXTURES
	list<CacheRequest> unloadsSkipped;
	for (int i = 0; i < READS_PER_FRAME; i++)
	{
		TexData texID;
		// Get a finished unload struct
		LOCK(m_doneUnloadQueueMutex);
		if (m_doneUnloadQueue.size())
		{
			unload = m_doneUnloadQueue.front();
			m_doneUnloadQueue.pop_front();
			// Give an unload request some time to transfer from GPU
			if (unload.m_cycles < UNLOAD_CYCLES)
			{
				unload.m_cycles++;
				unloadsSkipped.push_back(unload);
				UNLOCK(m_doneUnloadQueueMutex);
				continue;
			}
			texID = unload.tile->m_texdata;
			assert(texID.heightmap!=0);
		}
		else
		{
			UNLOCK(m_doneUnloadQueueMutex);
			break;
		}
		UNLOCK(m_doneUnloadQueueMutex);

		// if it isn't the shared Zero texture, release it into pool
		if (texID.heightmap != m_zeroTex.heightmap)
		{
			DEBUG3("releasing texture ID tuple %d %d\n", int(texID.heightmap), int(texID.normalmap));
			m_texQueue.push(texID);
		}
		memset(&unload.tile->m_texdata, 0, sizeof(TexData));

		// Unmap the PBO from sysmem pointer
		glBindBuffer(GL_PIXEL_PACK_BUFFER, unload.pbo);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		CheckError("Unmapping pack buffer");
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		// Release the PBO for someone else to use next frame around.
		m_pboPackPool.push(unload.pbo);
	}
	LOCK(m_doneUnloadQueueMutex);
	m_doneUnloadQueue.insert(m_doneUnloadQueue.begin(), unloadsSkipped.begin(), unloadsSkipped.end());
	UNLOCK(m_doneUnloadQueueMutex);
}

//--------------------------------------------------------
// A global function that represents the execution of the caching thread whose
// job it is to load and save data from and to files on the hdd.
int
hdd_cacher(void* data)
{
	Caching* pCaching = (Caching*)data;
	CacheRequest load, unload;
	bool gotRequest;
	while (pCaching->m_threadRunning)
	{
		// pop a load request
		gotRequest = false;
		LOCK(pCaching->m_loadQueueMutex);
		if (pCaching->m_loadQueue.size())
		{
			gotRequest = true;
			load = pCaching->m_loadQueue.front();
			pCaching->m_loadQueue.pop_front();
		}
		UNLOCK(pCaching->m_loadQueueMutex);

		// Actually read the image file from disk
		if (gotRequest)
		{
			// load image to sys memory mapped by PBO
			DEBUG3("Loading tile %d %d image into memory\n", load.tile->m_row, load.tile->m_col);
			if (!pCaching->LoadTextureData(load))
			{
				// if this failed, we notify GL thread to use Zero Texture
				load.useZero = true;
			}
			else
			{
				load.useZero = false;
			}

			// notify GL thread that memory is done and ready for transfer (another list ?)
			LOCK(pCaching->m_doneLoadQueueMutex);
			pCaching->m_doneLoadQueue.push_back(load);
			UNLOCK(pCaching->m_doneLoadQueueMutex);
		}
		else
		{
			SDL_Delay(5);
		}

		// ----------------------

		// pop a write request
		gotRequest = false;
		LOCK(pCaching->m_unloadQueueMutex);
		if (pCaching->m_unloadQueue.size())
		{
			gotRequest = true;
			unload = pCaching->m_unloadQueue.front();
			pCaching->m_unloadQueue.pop_front();
		}
		UNLOCK(pCaching->m_unloadQueueMutex);

		// Write the data to disk
		if (gotRequest)
		{
			DEBUG3("Writing tile %d %d to image file\n", unload.tile->m_row, unload.tile->m_col);
			// Save the texture data to disk (and release PBO and texture ID)
			if (!pCaching->SaveTextureData(unload))
				fprintf(stderr," ERROR: :could not write the tile !!!\n");
		}
		else
		{
			SDL_Delay(5);
		}
	}
	return 0;
}

//--------------------------------------------------------
inline bool
Caching::LoadTextureData(CacheRequest load)
{
	FIBITMAP*		image;
	BYTE*			bits;
	char			filename[256];
	
	sprintf(filename, "cache/tile%02d_%02d.png", load.tile->m_row, load.tile->m_col);

	image = FreeImage_Load(FIF_PNG, filename, 0);

	if (!image)
	{
		return false;
	}

	//FreeImage_FlipVertical(image);
	bits = (BYTE*) FreeImage_GetBits(image);

	memcpy(load.ptr, bits, sizeof(GLubyte) * m_highDim * m_highDim);

	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
inline bool
Caching::SaveTextureData(CacheRequest unload)
{
	FIBITMAP* 		image;
	BYTE*			bits;
	char			filename[256];
	
	sprintf(filename, "cache/tile%02d_%02d.png", unload.tile->m_row, unload.tile->m_col);

	mkdir("cache");
	image = FreeImage_Allocate(m_highDim, m_highDim, 16);

	bits = (BYTE*) FreeImage_GetBits(image);
	memcpy(bits, unload.ptr, m_highDim * m_highDim);

	//FreeImage_FlipVertical(image);

	// Release the PBO and texture ID
	LOCK(m_doneUnloadQueueMutex);
	m_doneUnloadQueue.push_back(unload);
	UNLOCK(m_doneUnloadQueueMutex);

	if (!FreeImage_Save(FIF_PNG, image, filename, PNG_Z_BEST_SPEED))
	{
		FreeImage_Unload(image);
		return false;
	}

	FreeImage_Unload(image);
		
	return true;
}
