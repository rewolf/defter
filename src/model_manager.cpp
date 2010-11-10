
#include "constants.h"
using namespace std;
#include "model_manager.h"

//-----------------------------------------------------------------------------
ModelManager::ModelManager(){
}

//-----------------------------------------------------------------------------
ModelManager::~ModelManager(){
	for (map<string,Node*>::iterator i=m_models.begin(); i!=m_models.end(); i++){
		re_DeleteModelData(i->second);
		delete i->second;
	}
}

//-----------------------------------------------------------------------------
bool
ModelManager::LoadModel(string name, string path){
	Node* pModel = NULL;

	// If this model is already loaded
	if (m_models.find(name) != m_models.end()){
		return true;
	}

	// Otherwise load the model
	pModel = re_LoadModel(path);

	if (pModel == NULL){
		fprintf(stderr, "Model Manager could not load model %s!\n", path.c_str());
		return false;
	}

	// Put it into the map
	m_models[name] = pModel;

	return true;
}

//-----------------------------------------------------------------------------
Node*
ModelManager::GetModel(string key){
	map<string, Node*>::iterator ret = m_models.find(key);
	if  (ret == m_models.end())
		return NULL;

	return new Node(*(ret->second));
}
