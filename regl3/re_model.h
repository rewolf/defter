#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

#define MESH_MAX	64


//--------------------------------------------------------
struct Mesh{
	GLuint	tex;
	GLuint	vao;
	GLuint	vbo[4];
	int		nIndices;

	vector3	diffuse;
	vector3 ambient;
	vector3 specular;
	float	specPower;
};

//--------------------------------------------------------
struct Transform{
	vector3		translate;
	vector3		rotate;
	vector3		scale;
	matrix4		cache;
	bool		valid;
};

//--------------------------------------------------------
class Node{
public:
	Node	();
	Node	(Node&);	// Copy constructor
	~Node	();

	Node*	GetNode			(string name);

public:
	Node*			m_pChild;		// First child
	Node*			m_pSibling;		// First sibling
	Mesh			m_mesh;
	Transform		m_transform;
	string			m_name;
};

//--------------------------------------------------------
Node* 		reLoadModel			(string filename);
bool 		reLoadChildren		(FILE* fp, Node*);
bool		reLoadMeshData		(FILE* fp, Node*);
void		reDeleteModelData	(Node* root);

#endif
