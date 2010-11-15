
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
	// Delete textures
	glDeleteTextures(m_textures.size(), &m_textures[0]);
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
	// Pass the model a map containing all textures that must be loaded, and the texture ID variables
	// in which the textures must be referenced.
	map<string, list<GLuint*> > textures;
	pModel = re_LoadModel(path, textures);

	for (map<string, list<GLuint*> >::iterator i = textures.begin(); i!=textures.end(); i++){
		string texname = "models/"+i->first;
		GLuint texID;
		// Load the texture
		if (!LoadTexture(&texID, texname)){
			return false;
		}
		// Store the id with the model manager for deletion later
		m_textures.push_back(texID);
		// Assign the id to all meshes that requested it
		for (list<GLuint*>::iterator j = i->second.begin(); j!=i->second.end(); j++){
			GLuint* meshtex = *j;
			*meshtex = texID;
		}
	}

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
