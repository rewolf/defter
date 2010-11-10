
#include "regl3.h"
#include "re_math.h"
#include "re_model.h"


#define 	__fscanf(fp, ...)		if (fscanf(fp, __VA_ARGS__) == EOF){ \
										fprintf(stderr,"EOF loading model");\
										return NULL;\
									}
#define 	__fscanF(fp, ...)		if (fscanf(fp, __VA_ARGS__) == EOF){ \
										fprintf(stderr,"EOF loading model");\
										return false;\
									}

//--------------------------------------------------------
Node::Node(){
	m_pChild 			= NULL;
	m_pSibling			= NULL;
	m_transform.valid	= false;
}

//--------------------------------------------------------
Node::Node(Node& copy){
	m_pChild 		= NULL;
	m_pSibling		= NULL;

	// let this have the same mesh properties and gpu buffers
	m_mesh 		= copy.m_mesh;
	// let it copy the current transform
	m_transform	= copy.m_transform;
	m_transform.valid = false;		// regen cache
	// same name for the node
	m_name		= copy.m_name;

	// Lets a duplicate the sibling, which duplicates the next sibling
	if (copy.m_pSibling)
		m_pSibling = new Node(*copy.m_pSibling);
	// If there is a child, duplicate it and the child will duplicate its siblings
	if (copy.m_pChild)
		m_pChild = new Node(*copy.m_pChild);

}

//--------------------------------------------------------
void
Node::InvalidateCache(){
	m_transform.valid = false;
}

//--------------------------------------------------------
Node*
Node::GetNode(string name){
	Node* found = NULL;

	// If the node is me, return myself
	if (m_name==name)
		return this;

	// If it's a sibling, return him
	if (m_pSibling){
		found = m_pSibling->GetNode(name);
		if (found != NULL)
			return found;
	}

	// If it's a descendent, return it
	if (m_pChild){
		found = m_pChild->GetNode(name);
		if (found != NULL)
			return found;
	}

	// Not found --> return nothing :(
	return NULL;
}

//--------------------------------------------------------
Node::~Node(){
	// recursive deletes (only the nodes - not gpu data)
	if (m_pChild)
		delete m_pChild;
	if (m_pSibling)
		delete m_pSibling;
}

//--------------------------------------------------------
Node*
re_LoadModel(string filename){
	FILE*	fp;
	char	a_string[256];
	char	b_string[256];
	int		nModels;
	Node*	pRoot;

	fp 	= fopen(filename.c_str(), "r");
	if (!fp){
		fprintf(stderr, "\nERROR: Model file %s does not exist\n", filename.c_str());
		return NULL;
	}

	// Read header
	__fscanf(fp, "%s", a_string);
	if (strcmp(a_string, "reMoA") != 0){
		fprintf(stderr, "\nERROR: Model type is not reMoA\n");
		return NULL;
	}
	__fscanf(fp, "%s %s %s %s", b_string, a_string, b_string, b_string);
	if (strcmp(a_string, "1.2a") != 0){
		fprintf(stderr, "\nERROR: Only support version 1.2a of model loader\n");
		return NULL;
	}

	// ignore materials for now
	__fscanf(fp, "%d", &nModels);

	if (nModels != 1){
		printf("\nWARNING: Model file %s contains more than one model, only taking first.",
				filename.c_str());
	}

	// Process model
	__fscanf(fp, "%s %s", b_string, b_string);	// MODEL
	pRoot = new Node;
	re_LoadChildren(fp, pRoot);


	// End off
	fclose(fp);

	return pRoot;
}

//--------------------------------------------------------
bool
re_LoadChildren(FILE* fp, Node* pRoot){
	char	a_string[256];
	char	b_string[256];
	Node*	pNode;

	pNode = NULL;

	__fscanf(fp, "%s %s", b_string, a_string);	// MESH or ENDMESH (of parent)
	// This will loop until we get an ENDMESH line or an ENDMODEL
	// which indicates the end of this generation
	while(strcmp(b_string, "MESH") == 0){
		if (pNode == NULL)
			// Load the first child
			pNode = pRoot;
		else{
			// Create a sibling to load
			pNode->m_pSibling = new Node;
			pNode = pNode->m_pSibling;
		}
		pNode->m_name = a_string;

		// Load the mesh data and allocate on GPU
		if (!re_LoadMeshData(fp, pNode)){
			fprintf(stderr, "Current state of model loader is undefined\n");
		}
	
		// Load children if they exist
		pNode->m_pChild = new Node;
		if (!re_LoadChildren(fp, pNode->m_pChild)){
			delete pNode->m_pChild;
			pNode->m_pChild = NULL;
		}
		// We have reached the end of the mesh (ENDMESH has been read)

		// Prepare for another mesh
		__fscanf(fp, "%s %s", b_string, a_string);	// MESH or ENDMESH (sibling or parent-mesh end)
	}

	// Return whether we loaded any meshes
	return (pNode != NULL);
}

//--------------------------------------------------------
bool
re_LoadMeshData(FILE* fp, Node* pNode){
	vector3 	translate;
	vector3 	rotate;
	vector3 	scale;
	int 		nVertices, nNormals, nTexCoords, nIndices, nQuadIndices;
	char		a_string[256];
	char		szTexture[256];
	char		szNormalMap[256];
	vector3*	vertices;
	vector3*	normals;
	vector2*	tcoords;
	GLuint*		indices;
	GLuint		vao;
	GLuint*		vbo;

	// Transform
	__fscanF(fp, "%f %f %f", &translate.x, &translate.y, &translate.z);
	__fscanF(fp, "%f %f %f", &rotate.x, &rotate.y, &rotate.z);
	__fscanF(fp, "%f %f %f", &scale.x, &scale.y, &scale.z);
	pNode->m_transform.translate = translate;
	pNode->m_transform.rotate = rotate;
	pNode->m_transform.scale = scale;

	// Textures
	__fscanF(fp, "%s", szTexture);
	__fscanF(fp, "%s", szNormalMap);

	// Material
	__fscanF(fp, "%s", a_string);
	__fscanF(fp, "%f %f %f", &pNode->m_mesh.diffuse.x, &pNode->m_mesh.diffuse.y, &pNode->m_mesh.diffuse.z);
	__fscanF(fp, "%f %f %f", &pNode->m_mesh.ambient.x, &pNode->m_mesh.ambient.y, &pNode->m_mesh.ambient.z);
	if (strcmp(a_string, "phong")==0){
		__fscanF(fp, "%f %f %f", &pNode->m_mesh.specular.x, &pNode->m_mesh.specular.y, &pNode->m_mesh.specular.z);
		__fscanF(fp, "%f", &pNode->m_mesh.specPower);
	}
	else if (strcmp(a_string, "blinn")==0){
		printf("\n\tWARNING: No support for blinn yet\n");
		float dummy;
		__fscanF(fp, "%f %f %f", &pNode->m_mesh.specular.x, &pNode->m_mesh.specular.y, &pNode->m_mesh.specular.z);
		__fscanF(fp, "%f %f", &dummy, &dummy);
	}
	else{
		// specular colour should be initialised to zero already
		pNode->m_mesh.specPower = .0f;
	}

	
	__fscanF(fp, "%d %d %d %d %d", &nVertices, &nNormals, &nTexCoords, &nIndices, &nQuadIndices);
	if (nQuadIndices != 0){
		fprintf(stderr, "\nERROR: Model loader does not support quads");
		return false;
	}
	if (nNormals != 0 && nNormals != nVertices){
		fprintf(stderr, "\nERROR: Number of vertices and normals differ");
		return false;
	}
	if (nTexCoords != 0 && nTexCoords != nVertices){
		fprintf(stderr, "\nERROR: Number of vertices and texture coordinates differ");
		return false;
	}
	vertices	= new vector3[nVertices];
	normals		= new vector3[nNormals];
	tcoords		= new vector2[nTexCoords];
	indices		= new GLuint[nIndices];
	
	// Read in vertices
	for (int i = 0 ; i < nVertices; i++)
		__fscanF(fp, "%f %f %f", &vertices[i].x, &vertices[i].y, &vertices[i].z);
	// Read in normals
	for (int i = 0 ; i < nNormals; i++)
		__fscanF(fp, "%f %f %f", &normals[i].x, &normals[i].y, &normals[i].z);
	// Read in texture coordinates
	for (int i = 0 ; i < nTexCoords; i++)
		__fscanF(fp, "%f %f", &tcoords[i].x, &tcoords[i].y);
	// Read in indices
	for (int i = 0 ; i < nIndices; i++)
		__fscanF(fp, "%d", &indices[i]);

	// Create the VAO and VBOs
	glGenVertexArrays(1, &pNode->m_mesh.vao);
	glGenBuffers(4, pNode->m_mesh.vbo);
	vao = pNode->m_mesh.vao; 
	vbo = pNode->m_mesh.vbo;

	glBindVertexArray(vao);
	// Fill the Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVertices, vertices, GL_STATIC_READ);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Fill the Normal Buffer
	if (nNormals > 0){
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVertices, normals, GL_STATIC_READ);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	// Fill the Texture Coordinate Buffer
	if (nTexCoords > 0){
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vector2) * nVertices, tcoords, GL_STATIC_READ);
		glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
	}
	// Fill the Index Buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * nIndices, indices, GL_STATIC_READ);
			
	// Setup the reset of the mesh struct
	pNode->m_mesh.nIndices = nIndices;

	delete[] vertices;
	delete[] normals;
	delete[] tcoords;
	delete[] indices;

	return true;
}

//--------------------------------------------------------
// Deletes the model's gpu memory BUT NOT structure
void
re_DeleteModelData(Node* pNode){
	if (pNode->m_pChild)
		re_DeleteModelData(pNode->m_pChild);
	if (pNode->m_pSibling)
		re_DeleteModelData(pNode->m_pSibling);
	glDeleteTextures(1, &pNode->m_mesh.tex);
	glDeleteBuffers(4, pNode->m_mesh.vbo);
	glDeleteVertexArrays(1, &pNode->m_mesh.vao);
}

