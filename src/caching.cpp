
#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include "caching.h"



//--------------------------------------------------------
Caching::Caching(int clipDim, int coarseDim, float clipRes, int highDim, float highRes){
	//Calculate the tile size and grid dimensions
	m_TileSize		= highDim * highRes;
	m_SizeOfGrid	= (int)((coarseDim * clipRes) / m_TileSize);

	//Create the grid
	m_Grid			= new Tile[m_SizeOfGrid*m_SizeOfGrid];

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
	
}

//--------------------------------------------------------
void
UpdateLoadStatus (bool newStatus, int region){

}
