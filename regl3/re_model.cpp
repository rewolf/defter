
#include "regl3.h"
#include "re_math.h"
using namespace std;


#define 	__fscanf(fp, ...)		if (fscanf(fp, __VA_ARGS__) == EOF){ \
										fprintf(stderr,"EOF loading model");\
										return;
									}
#define 	__fscanF(fp, ...)		if (fscanf(fp, __VA_ARGS__) == EOF){ \
										fprintf(stderr,"EOF loading model");\
										return false;
									}


//--------------------------------------------------------
reModel::reModel(string filename){
	FILE*	fp;
	
}


//--------------------------------------------------------
reModel::~reModel(){
}

//--------------------------------------------------------
bool
reModel::load_mesh(FILE* fp, Mesh* pMesh){
}
