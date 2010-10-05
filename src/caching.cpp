
#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include "caching.h"

#define WRAP(val, dim) ((val < 0) ? (val + dim) : ((val > (dim - 1)) ? (val - dim) : val))
#define OFFSET(x, y, dim) (y * dim + x)

//--------------------------------------------------------
Caching::Caching(int clipDim, int coarseDim, float clipRes, int highDim, float highRes){
	//Calculate the tile size and grid dimensions
	m_TileSize		= highDim * highRes;
	m_GridSize		= (int)((coarseDim * clipRes) / m_TileSize);

	//Create the grid
	m_Grid			= new Tile[m_GridSize*m_GridSize];

	//Calculate the band values
	m_BandWidth		= (m_TileSize - (clipDim * clipRes)) * 0.9f;
	m_BandPercent	= m_BandWidth / m_TileSize;

	//Calculate the offset value for the coarsemap to allow determining of tile index
	m_CoarseOffset	= coarseDim * clipRes * 0.5f;

	//Set default values
	m_RegionPrevious= 0;
	m_RegionCurrent	= 0;
}

//--------------------------------------------------------
Caching::~Caching(){
	delete[] m_Grid;
}

//--------------------------------------------------------
void
Caching::Update (vector2 worldPos){
	worldPos          += vector2(m_CoarseOffset);
	vector2 tilePos    = (worldPos/m_TileSize);
	// Get the tile index into the array
	vector2 tileIndex  = tilePos.Floor();

	// Sift out just the fractional part to find location within the tile and offset to centre
	// so that positive -> right of centre or below centre and negative left or above
	tilePos -= (tileIndex + vector2(.5f));

	// do this so we can check absolute distance from centre
	vector2 absTilePos = tilePos.Abs();

	// Identify the new tile region
	m_RegionPrevious = m_RegionCurrent;

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
		UpdateLoadStatus(false, m_RegionPrevious, tileIndex);
		UpdateLoadStatus(true,  m_RegionCurrent,  tileIndex);
	}


}

//--------------------------------------------------------
void
Caching::UpdateLoadStatus (bool newStatus, int region, vector2 TileIndex){
	OFFSET(WRAP(3, 4), 3, 7);
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
