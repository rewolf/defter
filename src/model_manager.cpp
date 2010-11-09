
#include <map>
#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_model.h"
#include "model_manager.h"

//-----------------------------------------------------------------------------
ModelManager::ModelManager(){
}

//-----------------------------------------------------------------------------
ModelManager::~ModelManager(){
	for (map<string,reModel*>::iterator i = m_models.begin(); i != m_models.end(); i++){
		delete i->second;
	}
}

//-----------------------------------------------------------------------------
bool
ModelManager::LoadModel(string name, string path){
	reModel* pModel = NULL;

	// If this model is already loaded
	if (m_models.find(name) != m_models.end()){
		return true;
	}

	// Otherwise load the model
	pModel = new reModel(path);

	if (pModel == NULL || !pModel->m_loaded){
		fprintf(stderr, "Model Manager could not load model %s!\n", path.c_str());
		return false;
	}

	// Put it into the map
	m_models[name] = pModel;

	return true;
}

//-----------------------------------------------------------------------------
reModel*
ModelManager::GetModel(string key){
	map<string, reModel*>::iterator ret = m_models.find(key);
	if  (ret == m_models.end())
		return NULL;
	return ret->second;
}
