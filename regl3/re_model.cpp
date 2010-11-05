
#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_model.h"


#define 	__fscanf(fp, ...)		if (fscanf(fp, __VA_ARGS__) == EOF){ \
										fprintf(stderr,"EOF loading model");\
										return;\
									}
#define 	__fscanF(fp, ...)		if (fscanf(fp, __VA_ARGS__) == EOF){ \
										fprintf(stderr,"EOF loading model");\
										return false;\
									}


//--------------------------------------------------------
reModel::reModel(string filename){
	FILE*	fp;
	char	a_string[256];
	char	b_string[256];
	float	flt;
	int 	nMaterials;
	int		nModels;

	m_loaded	= false;
	m_nMeshes	= 0;
	memset(m_vao_list, 0, sizeof(GLuint) * MESH_MAX);
	memset(m_vbo_list, 0, sizeof(GLuint) * MESH_MAX * 4);
	memset(m_tex_list, 0, sizeof(GLuint) * MESH_MAX);
	//memset(m_mesh_list, 0, sizeof(GLuint) * MESH_MAX);
	
	fp 	= fopen(filename.c_str(), "r");
	if (!fp){
		fprintf(stderr, "\nERROR: Model file %s does not exist\n", filename.c_str());
		return;
	}

	// Read header
	__fscanf(fp, "%s", a_string);
	if (strcmp(a_string, "reMoA") != 0){
		fprintf(stderr, "\nERROR: Model type is not reMoA\n");
		return;
	}
	__fscanf(fp, "%s %s %s %s", b_string, a_string, b_string, b_string);
	if (strcmp(a_string, "1.0b") != 0){
		fprintf(stderr, "\nERROR: Only support version 1.0b of model loader\n");
		return;
	}

	// ignore materials for now
	__fscanf(fp, "%d %d", &nMaterials, &nModels);
	for (int i = 0; i < nMaterials; i++){
		__fscanf(fp, "%s %s %s", b_string, b_string, b_string);
		__fscanf(fp, "%f %f %f %f %f %f %f", &flt, &flt, &flt, &flt, &flt, &flt, &flt);
	}

	if (nModels != 1){
		printf("\nWARNING: Model file %s contains more than one model, only taking first.",
				filename.c_str());
	}

	// Process model
	__fscanf(fp, "%s %s", b_string, b_string);
	__fscanf(fp, "%s %s", b_string, a_string);
	while (strcmp(b_string, "MESH") == 0){
		Mesh* pMesh = &m_mesh_list[m_nMeshes];
		pMesh->name = a_string;
		load_mesh(fp, pMesh);

		m_nMeshes ++;
		// read end string
		__fscanf(fp, "%s %s", b_string, b_string);
		// read next mesh
		__fscanf(fp, "%s %s", b_string, a_string);
	}


	if (strcmp(b_string, "ENDMODEL") != 0){
		fprintf(stderr, "\nERROR: Unexpected end of model file %s\n", filename.c_str());
	}
	// End off
	fclose(fp);
	m_loaded = true;
}


//--------------------------------------------------------
reModel::~reModel(){
	glDeleteVertexArrays (MESH_MAX,	m_vao_list);
	glDeleteBuffers (MESH_MAX * 4, 	m_vbo_list);
	glDeleteTextures (MESH_MAX,		m_tex_list);
}

//--------------------------------------------------------
bool
reModel::load_mesh(FILE* fp, Mesh* pMesh){
	vector3 	translate;
	vector3 	rotate;
	vector3 	scale;
	int 		nVertices, nNormals, nTexCoords, nIndices;
	char		a_string[256];
	char		b_string[256];
	vector3*	vertices;
	vector3*	normals;
	vector3*	tcoords;
	GLuint*		indices;

	// Transform
	__fscanF(fp, "%f %f %f", &translate.x, &translate.y, &translate.z);
	__fscanF(fp, "%f %f %f", &rotate.x, &rotate.y, &rotate.z);
	__fscanF(fp, "%f %f %f", &scale.x, &scale.y, &scale.z);
	pMesh->transform	= translate_tr(translate)
						* rotate_tr(rotate.x, 1.0f, .0f, .0f)
						* rotate_tr(rotate.y, .0f, 1.0f, .0f)
						* rotate_tr(rotate.z, .0f, .0f, 1.0f)
						* scale_tr(scale);

	// scrap material
	__fscanF(fp, "%s", b_string);
	
	__fscanF(fp, "%d %d %d %d", &nVertices, &nNormals, &nTexCoords, &nIndices);
	vertices	= new vector3[nVertices];
	normals		= new vector3[nNormals];
	tcoords		= new vector3[nTexCoords];
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

	delete[] vertices;
	delete[] normals;
	delete[] tcoords;
	delete[] indices;

	return true;
}
