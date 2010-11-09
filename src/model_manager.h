#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_


class ModelManager{
public:
	ModelManager();
	~ModelManager();
	
	bool		LoadModel		(string name, string path);
	reModel*	GetModel		(string key);

private:
	map<string, reModel*>	m_models;
	map<string, GLuint>		m_textures;
};


#endif
