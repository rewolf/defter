
#include "regl3.h"
#include "re_math.h"
#include "util.h"
using namespace reMath;
#include "caching.h"



//--------------------------------------------------------
Caching::Caching(int coarseDim, float clipRes, int highDim, float highRes){
}

//--------------------------------------------------------
Caching::~Caching(){
	delete[] m_Grid;
}

//--------------------------------------------------------
void
Caching::Update (vector2 worldPos){
	
}
