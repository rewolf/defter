#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_


class ModelManager{
public:
	ModelManager();
	~ModelManager();
	
	bool		LoadModel		(string name, string path);
	Node*		GetModel		(string key);

private:
	map<string, Node*>		m_models;
};


#endif

