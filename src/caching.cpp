
#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include "re_shader.h"
#include "deform.h"
#include <list>
#include "caching.h"

#define WRAP(val, dim) ((val < 0) ? (val + dim) : ((val > (dim - 1)) ? (val - dim) : val))
#define OFFSET(x, y, dim) (y * dim + x)

#define INITOFFSET	0

//--------------------------------------------------------
Caching::Caching(Deform* pDeform, int clipDim, int coarseDim, float clipRes, int highDim, float highRes){
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

	m_threadRunning 	= true;
	m_cacheThread 		= SDL_CreateThread(hdd_cacher, (void*)this);
	m_cacheQueueMutex	= SDL_CreateMutex();
}

//--------------------------------------------------------
Caching::~Caching(){
	m_threadRunning = false;
	SDL_WaitThread(m_cacheThread, NULL);
	SDL_DestroyMutex(m_cacheQueueMutex);
	delete[] m_Grid;
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
					Unload(m_Grid[i]);
				}
			}
			// If it wasn't loaded
			else{
				// But needs to be!!!
				if (m_Grid[i].m_LoadedCurrent){
					Load(m_Grid[i]);
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

	int index = (int)OFFSET(WRAP(tileIndex.x, m_GridSize), WRAP(tileIndex.y, m_GridSize), m_GridSize);

	if (m_Grid[index].m_texID != -1)
	{
		m_Grid[index].m_modified = true;
		m_pDeform->displace_heightmap(coarseMap, clickPos, scale, true);

		DrawRadar();
	}
	else
		printf("High Def out of bounds");
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
Caching::Load(Tile& tile){
	// Removed any existing requests for this tile
		// lock write queue
		// search and remove
		// unlock write queue
		// check read queue ??

	// Grab PBO from pool
		// wait on semaphore for a PBO

	// Map PBO for data transfer

	// Push a request for CPU to load image data into mapped PBO array
		// set pointer to mapped PBO in request struct
		// lock read queue
		// push request onto queue
		// unlock queue
}

//--------------------------------------------------------
// Called by the GL thread.  Puts the tile on the queue for unloading
void
Caching::Unload(Tile& tile){
	// Remove any existing requests for this tile

	// Check if tile has been modified
	
	// if so...

	// Get a PBO and Bind

	// Begin DMA transfer from GPU texture memory into PBO

	// Push request for CPU thread to write data to disk

	// Unbind 
}

//--------------------------------------------------------
void
Caching::UpdatePBOs(){
	// Allocate PBOs to Load/Unload requests
	// ??

	// process maximum 4
	for (int i = 0; i < 4; i++){
		// check if any transfers from sysmem are ready
			// lock ready queue
			// check size -> break if empty
			// grab ready struct
			// unlock ready queue

		// Unmap the PBO 

		// Begin transfer
			// call glTex(Sub)Image

		// Release the PBO for someone else to use next frame around.
			// push PBO onto PBO queue/pool
			// increment semaphore
	}
}

//--------------------------------------------------------
// A global function that represents the execution of the caching thread whose
// job it is to load and save data from and to files on the hdd.
int
hdd_cacher(void* data){
	Caching* pCaching = (Caching*)data;
	while (pCaching->m_threadRunning){
		// pop a read request
			// lock read queue
			// pop request
			// unlock read queue

		// load image to sys memory

		// memcpy image to mapped PBO buffer memory

		// notify GL thread that memory is ready for transfer (another list ?)
			// lock ready queue
			// push struct
			// unlock ready queue

		// ------------

		// pop a write request
		SDL_Delay(10);
	}
	return 0;
}

