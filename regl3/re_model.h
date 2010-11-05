#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

#define MESH_MAX	64

struct Mesh{
	string	name;
	GLuint	tex;
	GLuint	vao;
	int		nIndices;
	matrix4	transform;
};


class reModel{
public:
	reModel(string filename);
	~reModel();

private:
	bool		load_mesh(FILE* fp,  Mesh* pMesh);

public:
	GLuint		m_vao_list[MESH_MAX];
	GLuint		m_vbo_list[MESH_MAX * 4];
	GLuint		m_tex_list[MESH_MAX];
	Mesh		m_mesh_list[MESH_MAX];
	int			m_nMeshes;
	string		m_name;
	bool		m_loaded;
};



#endif
