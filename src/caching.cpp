
#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include "re_shader.h"
#include "deform.h"
#include "caching.h"

#define WRAP(val, dim) ((val < 0) ? (val + dim) : ((val > (dim - 1)) ? (val - dim) : val))
#define OFFSET(x, y, dim) (y * dim + x)

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
//		m_Grid[i].m_TexID = 
		m_Grid[i].m_LoadedPrevious = false;
		m_Grid[i].m_LoadedCurrent  = false;
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
	sstr << "Band %:\t\t\t\t"	<< m_BandPercent << "%\n";
	m_caching_stats += sstr.str();
}

//--------------------------------------------------------
Caching::~Caching(){
	delete[] m_Grid;
}

//--------------------------------------------------------
void
Caching::Update (vector2 worldPos){
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
		UpdateLoadStatus(false, m_RegionPrevious, m_TileIndexPrevious);
		UpdateLoadStatus(true,  m_RegionCurrent,  m_TileIndexCurrent);
		updateRadar = true;
	}

	// Check all tiles for loading/unloading
	for (int i = 0; i < m_GridSize * m_GridSize; i++){
		// If it was loaded
		if (m_Grid[i].m_LoadedPrevious){
			// But need not be loaded anymore
			if (!m_Grid[i].m_LoadedCurrent){
				// UNLOAD
			}
		}
		// If it wasn't loaded
		else{
			// But needs to be!!!
			if (m_Grid[i].m_LoadedCurrent){
				// LOAD
			}
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
Caching::UpdateLoadStatus (bool newStatus, int region, vector2 TileIndex){
	switch (region)
	{
	case 0:
		//Top-Left
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y - 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y - 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 1:
		//Top-Centre
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 2:
		//Top-Right
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y - 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y - 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 3:
		//Centre-Left
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	- 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	- 1	, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	- 1	, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 4:
		//Centre-Centre
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	- 1	, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	+ 1	, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	- 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	+ 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	- 1	, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	+ 1	, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 5:
		//Centre-Right
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	+ 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	+ 1	, m_GridSize), WRAP(TileIndex.y	- 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x	+ 1	, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 6:
		//Bottom-Left
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y + 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y + 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 7:
		//Bottom-Centre
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x - 1	, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y	+ 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	case 8:
		//Bottom-Right
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y		, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x		, m_GridSize), WRAP(TileIndex.y + 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		m_Grid[(int)OFFSET(WRAP(TileIndex.x + 1	, m_GridSize), WRAP(TileIndex.y + 1	, m_GridSize), m_GridSize)].m_LoadedCurrent = newStatus;
		break;

	default:
		break;
	}
}

void
Caching::DrawRadar(void){
	char *radar = new char[m_GridSize * m_GridSize];
	memset(radar, '0', m_GridSize * m_GridSize);
	
	radar[(int)m_TileIndexCurrent.y * (m_GridSize) + (int)m_TileIndexCurrent.x] = 'X';

	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		printf("%c", radar[i]);
		if ((i % m_GridSize) != (m_GridSize - 1))
			printf(" ");
		else
			printf("\n");
	}
	printf("-----\n");
	printf("--%d--\n", m_RegionCurrent);
	printf("-----\n");

	memset(radar, '0', m_GridSize * m_GridSize);
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		if (m_Grid[i].m_LoadedCurrent)
			radar[i] = 'L';
		else
			radar[i] = 'U';
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
