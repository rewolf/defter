
#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include "re_shader.h"
#include "deform.h"
#include <list>
#include <queue>
#include "caching.h"
#include "FreeImage.h"
#ifdef _WIN32
#	include "direct.h"
#endif

#define WRAP(val, dim) ((val < 0) ? (val + dim) : ((val > (dim - 1)) ? (val - dim) : val))
#define OFFSET(x, y, dim) (y * dim + x)

#define INITOFFSET		(0)

#define READS_PER_FRAME	(3)
#define LOCK(m)			{ if (SDL_LockMutex(m) == -1) fprintf(stderr, "Mutex Lock Error\n"); }
#define UNLOCK(m)		{ if (SDL_UnlockMutex(m) == -1) fprintf(stderr, "Mutex Unlock Error\n"); }

#define DEBUG_ON		(1)
#ifdef DEBUG_ON
	#define DEBUG(x)		printf(x)
	#define DEBUG2(x,y)		printf(x,y)
	#define DEBUG3(x,y,z)	printf(x,y,z)
#else
	#define DEBUG(x)		{}
#endif

//--------------------------------------------------------
Caching::Caching(Deform* pDeform, int clipDim, int coarseDim, float clipRes, int highDim, float highRes){
	m_coarseDim		= coarseDim;
	m_highDim		= highDim;
	m_pDeform		= pDeform;
	//Calculate the tile size and grid dimensions
	m_TileSize		= highDim * highRes;
	float temp		= ((coarseDim * clipRes) / m_TileSize);
	m_GridSize		= (int)temp;

	//Create the grid
	m_Grid			= new Tile[m_GridSize*m_GridSize];

	for (int i = 0; i < m_GridSize * m_GridSize; i++){
		m_Grid[i].m_texID			= -1;
		m_Grid[i].m_modified		= false;
		m_Grid[i].m_LoadedPrevious	= false;
		m_Grid[i].m_LoadedCurrent	= false;
		m_Grid[i].m_row				= i / m_GridSize;
		m_Grid[i].m_col				= i % m_GridSize;
	}

	//Calculate the band values
	m_BandWidth		= (m_TileSize - (clipDim * clipRes)) * 0.9f;
	m_BandPercent	= m_BandWidth / m_TileSize;

	//Calculate the offset value for the coarsemap to allow determining of tile index
	m_CoarseOffset	= coarseDim * clipRes * 0.5f;

	//Set default values
	m_RegionPrevious	= 0;
	m_TileIndexPrevious	= vector2(.0f);
	m_caching_stats = "";

	stringstream sstr;
	sstr << "Grid Size:\t\t\t"	<< m_GridSize << "\n";
	sstr << "Clipmap Size:\t\t"	<< clipDim * clipRes << "\n";
	sstr << "Tile Size:\t\t\t"	<< m_TileSize << "m\n";
	sstr << "Band Width:\t\t\t"	<< m_BandWidth << "m\n";
	sstr << "Band %:\t\t\t\t"	<< (m_BandPercent * 100) << "%\n";
	m_caching_stats += sstr.str();

	// Intitialise threading constructs
	m_threadRunning 	= true;
	m_loadQueueMutex	= SDL_CreateMutex();
	m_unloadQueueMutex	= SDL_CreateMutex();
	m_doneQueueMutex	= SDL_CreateMutex();
	
	// Initialise PBO pool
	int texSize = sizeof(GLbyte) * highDim * highDim;
	glGenBuffers(PBO_POOL*2, m_pbos);
	for (int i = 0 ; i < PBO_POOL; i++){
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
	
	// create and launch cache thread
	m_cacheThread 		= SDL_CreateThread(hdd_cacher, (void*)this);
}

//--------------------------------------------------------
Caching::~Caching(){
	m_threadRunning = false;
	SDL_WaitThread(m_cacheThread, NULL);
	SDL_DestroyMutex(m_doneQueueMutex);
	SDL_DestroyMutex(m_loadQueueMutex);
	SDL_DestroyMutex(m_unloadQueueMutex);
	delete[] m_Grid;

	// delete PBO pool
	glDeleteBuffers(PBO_POOL*2, m_pbos);
}

//--------------------------------------------------------
void
Caching::Update (vector2 worldPos){
	UpdatePBOs();

	bool updateRadar	= false;
	worldPos		   += vector2(m_CoarseOffset);
	vector2 tilePos		= (worldPos/m_TileSize);
	// Get the tile index into the array
	// X = Column : Y = Row
	m_TileIndexCurrent	= tilePos.Floor();

	if (m_TileIndexCurrent != m_TileIndexPrevious)
		updateRadar = true;

	// Sift out just the fractional part to find location within the tile and offset to centre
	// so that positive -> right of centre or below centre and negative left or above
	tilePos -= (m_TileIndexCurrent + vector2(.5f));

	// do this so we can check absolute distance from centre
	vector2 absTilePos = tilePos.Abs();

	// Center block
	if (absTilePos < vector2(m_BandPercent * .5f)){
		m_RegionCurrent = 4;
	}
	// Vertical Band
	else if (absTilePos.x < m_BandPercent * .5f){
		if (tilePos.y < 0)
			m_RegionCurrent = 1;
		else
			m_RegionCurrent = 7;
	}
	// Horizontal Band
	else if (absTilePos.y < m_BandPercent * .5f){
		if (tilePos.x < 0)
			m_RegionCurrent = 3;
		else
			m_RegionCurrent = 5;
	}
	// Quads
	else {
		if (tilePos.x < .0f){
			if (tilePos.y < .0f)
				m_RegionCurrent = 0;
			else
				m_RegionCurrent = 6;
		}
		else{
			if (tilePos.y < .0f)
				m_RegionCurrent = 2;
			else
				m_RegionCurrent = 8;
		}
	}

	if (m_RegionCurrent != m_RegionPrevious){
		UpdateTiles(false, m_RegionPrevious, m_TileIndexPrevious);
		UpdateTiles(true,  m_RegionCurrent,  m_TileIndexCurrent);
		updateRadar = true;

		// Check all tiles for loading/unloading
		for (int i = 0; i < m_GridSize * m_GridSize; i++){
			// If it was loaded
			if (m_Grid[i].m_LoadedPrevious){
				// But need not be loaded anymore
				if (!m_Grid[i].m_LoadedCurrent){
					Unload(m_Grid + i);
				}
			}
			// If it wasn't loaded
			else{
				// But needs to be!!!
				if (m_Grid[i].m_LoadedCurrent){
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

	if (updateRadar)
		DrawRadar();
}

//--------------------------------------------------------
void
Caching::DeformHighDetail(TexData coarseMap, vector2 clickPos, float scale)
{
	// Get the tile index into the array
	// X = Column : Y = Row
	vector2 tileIndex	= ((vector2(m_CoarseOffset) + clickPos) / m_TileSize).Floor();

	int offset = (int)OFFSET(WRAP(tileIndex.x, m_GridSize), WRAP(tileIndex.y, m_GridSize), m_GridSize);
	m_Grid[offset].m_modified = true;
	
	m_pDeform->displace_heightmap(coarseMap, clickPos, scale, true);

	DrawRadar();
}
//--------------------------------------------------------
void
Caching::UpdateTiles(bool newStatus, int region, vector2 TileIndex)
{
	switch (region)
	{
	case 0:
		//Top-Left
		SetLoadStatus(newStatus, TileIndex - vector2(1.0f), vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(1.0f), vector2(2.0f));
		break;

	case 1:
		//Top-Centre
		SetLoadStatus(newStatus, TileIndex - vector2(1.0f), vector2(3.0f, 2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(0.0f, 1.0f), vector2(1.0f, 2.0f));
		break;

	case 2:
		//Top-Right
		SetLoadStatus(newStatus, TileIndex - vector2(0.0f, 1.0f), vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(0.0f, 1.0f), vector2(2.0f));
		break;

	case 3:
		//Centre-Left
		SetLoadStatus(newStatus, TileIndex - vector2(1.0f), vector2(2.0f, 3.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(1.0f, 0.0f), vector2(2.0f, 1.0f));
		break;

	case 4:
		//Centre-Centre
		SetLoadStatus(newStatus, TileIndex - vector2(1.0f), vector2(3.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(1.0f));
		break;

	case 5:
		//Centre-Right
		SetLoadStatus(newStatus, TileIndex - vector2(0.0f, 1.0f), vector2(2.0f, 3.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(2.0f, 1.0f));
		break;

	case 6:
		//Bottom-Left
		SetLoadStatus(newStatus, TileIndex - vector2(1.0f, 0.0f), vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex - vector2(1.0f, 0.0f), vector2(2.0f));
		break;

	case 7:
		//Bottom-Centre
		SetLoadStatus(newStatus, TileIndex - vector2(1.0f, 0.0f), vector2(3.0f, 2.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(1.0f, 2.0f));
		break;

	case 8:
		//Bottom-Right
		SetLoadStatus(newStatus, TileIndex, vector2(2.0f));
		SetActiveStatus(newStatus, TileIndex, vector2(2.0f));
		break;

	default:
		break;
	}
}

//--------------------------------------------------------
//Sets the status for the tiles around the given one
//Starts at TileIndex and iterates based on the value stored in size
void
Caching::SetLoadStatus(bool newStatus, vector2 TileIndex, vector2 size)
{
	for (int row = TileIndex.y; row < (int)(TileIndex.y + size.y); row++)
	{
		for (int col = TileIndex.x; col < (int)(TileIndex.x + size.x); col++)
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
	for (int row = TileIndex.y; row < (int)(TileIndex.y + size.y); row++)
	{
		for (int col = TileIndex.x; col < (int)(TileIndex.x + size.x); col++)
		{
			int offset = (int)OFFSET(WRAP(col, m_GridSize), WRAP(row, m_GridSize), m_GridSize);

			m_Grid[offset].m_texID	= (newStatus ? curVal++ : -1);
		}
	}
}

//--------------------------------------------------------
void
Caching::DrawRadar(void){
	char *radar = new char[m_GridSize * m_GridSize];
	memset(radar, '.', m_GridSize * m_GridSize);
	
	int index = (int)m_TileIndexCurrent.y * (m_GridSize) + (int)m_TileIndexCurrent.x;
	radar[index] = 'X';

	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		if (m_Grid[i].m_modified)
		{
			if (i == index)
				printf("Y");
			else
				printf("M");
		}
		else
			printf("%c", radar[i]);

		if ((i % m_GridSize) != (m_GridSize - 1))
			printf(" ");
		else
			printf("\n");
	}
	printf("-----\n");
	printf("--%d--\n", m_RegionCurrent);
	printf("-----\n");

	memset(radar, '.', m_GridSize * m_GridSize);
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		if (m_Grid[i].m_LoadedCurrent)
			radar[i] = 'L';
		else
			radar[i] = 'U';

		if (m_Grid[i].m_texID != -1)
			radar[i] = 'A';
	}
	
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		printf("%c", radar[i]);
		if ((i % m_GridSize) != (m_GridSize - 1))
			printf(" ");
		else
			printf("\n");
	}
	printf("-----\n");
	printf("-----\n");
}

//--------------------------------------------------------
// Called by the GL thread.  Puts the tile on the queue for loading
void
Caching::Load(Tile* tile){
	// Create struct
	CacheRequest load;
	load.type = LOAD;
	load.tile = tile;

	// Check for existing requests for this tile that negate this request and remove all
	/*for (list<CacheRequest>::iterator i = m_readyQueue.begin(); i != m_readyQueue.end(); i++){
		if (i->tile == tile){
			i = m_readyQueue.erase(i);
			i--;
		}
	}*/

	m_readyQueue.push_back(load);
	DEBUG3("Pushed tile %d %d\n", load.tile->m_row, load.tile->m_col);
}

//--------------------------------------------------------
// Called by the GL thread.  Puts the tile on the queue for unloading
void
Caching::Unload(Tile* tile){
	// Remove any existing requests for this tile

	// Check if tile has been modified
	
	// if so...

	// Get a PBO and Bind

	// Begin DMA transfer from GPU texture memory into PBO

	// Push request for CPU thread to write data to disk

	// Unbind 
}

//--------------------------------------------------------
// Allocates PBOs to Load/Unload requests (ready queue) and signals CPU queue
// also.. retrieves data readied by CPU (done queue)
// This is the only function that alters the PBO semaphore
void
Caching::UpdatePBOs(){
	CacheRequest load;

	// Allocate PBOs to Load/Unload requests
	while (m_pboUnpackPool.size()){
		if (m_readyQueue.size()){
			// pop request of queue
			load = m_readyQueue.front();
			m_readyQueue.pop_front();

			// pop PBO
			load.pbo = m_pboUnpackPool.front();
			m_pboUnpackPool.pop();

			// Map PBO to system memory for texture loading
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, load.pbo);
			load.ptr = (GLubyte*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
			if (!load.ptr){
				fprintf(stderr, "BAD ERROR: Could not map PBO pointer!!!!!\n");
				m_readyQueue.push_front(load);
				m_pboUnpackPool.push(load.pbo);
			}

			// Push request onto the load queue for CPU
			LOCK(m_loadQueueMutex);
			m_loadQueue.push_back(load);
			UNLOCK(m_loadQueueMutex);
		}
		else{
			break;
		}
	}

	// process maximum R
	for (int i = 0; i < READS_PER_FRAME; i++){
		// Get a load-ready struct
		LOCK(m_doneQueueMutex);
		if (m_doneQueue.size()){
			load = m_doneQueue.front();
			m_doneQueue.pop_front();
			DEBUG3("Tile %d %d is ready for upload\n", load.tile->m_row, load.tile->m_col);
		}
		else{
			UNLOCK(m_doneQueueMutex);
			break;
		}
		UNLOCK(m_doneQueueMutex);

		// Unmap the PBO from sysmem pointer
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, load.pbo);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		CheckError("Unmapping buffer\n");
		// Begin transfer
		DEBUG("Begin transfer\n");

		// Release the PBO for someone else to use next frame around.
		m_pboUnpackPool.push(load.pbo);
	}
}

//--------------------------------------------------------
// A global function that represents the execution of the caching thread whose
// job it is to load and save data from and to files on the hdd.
int
hdd_cacher(void* data){
	Caching* pCaching = (Caching*)data;
	CacheRequest load, unload;
	bool gotRequest;
	while (pCaching->m_threadRunning){
		// pop a load request
		gotRequest = false;
		LOCK(pCaching->m_loadQueueMutex);
		if (pCaching->m_loadQueue.size()){
			gotRequest = true;
			load = pCaching->m_loadQueue.front();
			pCaching->m_loadQueue.pop_front();
		}
		UNLOCK(pCaching->m_loadQueueMutex);

		// Actually read the image file from disk
		if (gotRequest){
			// load image to sys memory mapped by PBO
			DEBUG3("Loading tile %d %d image into memory\n", load.tile->m_row, load.tile->m_col);
			pCaching->LoadTextureData(load.tile, load.ptr);

			// notify GL thread that memory is done and ready for transfer (another list ?)
			LOCK(pCaching->m_doneQueueMutex);
			pCaching->m_doneQueue.push_back(load);
			UNLOCK(pCaching->m_doneQueueMutex);
			DEBUG("Pushing to done queue\n");
		}else{
			SDL_Delay(10);
		}

		// ----------------------

		// pop a write request
	}
	return 0;
}

//--------------------------------------------------------
inline bool
Caching::LoadTextureData(Tile* tile, GLubyte* data){
	FIBITMAP*		image;
	BYTE*			bits;
	int				width;
	int				height;
	int				bitdepth;
	char			filename[256];
	
	sprintf(filename, "cache/tile%02d_%02d.png", tile->m_row, tile->m_col);

	image = FreeImage_Load(FIF_PNG, filename, 0);

	if (!image){
		fprintf(stderr, "Error\n\tCould not load PNG: %s\n", filename);
		return false;
	}

	bits = (BYTE*) FreeImage_GetBits(image);

	memcpy(data, bits, sizeof(GLubyte) * m_highDim * m_highDim);

	FreeImage_Unload(image);
}

