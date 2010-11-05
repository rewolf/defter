#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

struct Mesh{
	string	name;
	GLuint	tex;
	GLuint	vao;
	int		nIndices;
};


class reModel{
	reModel(string filename);
	~reModel();

private:
	bool		load_mesh(FILE* fp,  Mesh* pMesh);

public:
	GLuint		m_vao_list[64];
	GLuint		m_vbo_list[256];
	GLuint		m_tex_list[64];
	Mesh		m_mesh_list[64];
	int			m_nMeshes;
	string		m_name;
	bool		m_loaded;
};



#endif
