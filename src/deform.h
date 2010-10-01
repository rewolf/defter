
#ifndef _DEFORM_H_
#define _DEFORM_H_

typedef struct{
	float u,v;
} float2;

typedef GLuint texId;

typedef struct {
	texId		current;
	texId		backup;
	texId		normalmap;
	texId		tangentmap;
} displacer;


class Deform{
public:
	Deform (texId* pHeightmap, texId* pNormalmap, texId* pTangentmap, int width, int height);
	~Deform();

	void 		displace_heightmap			(float2 tex_coord, float scale);
	void 		calculate_normals			(float2 tex_coord, float2 scale);
	void 		bind_displacers				(void);

public:
	ShaderProg*		m_shDeform;
	ShaderProg*		m_shNormal;
	texId* 			m_pHeightmap;
	displacer		m_heightmap;

	// FBOs
	GLuint			m_fbo_heightmap;

	// VAOs & VBOs
	GLuint			m_vao;
	GLuint			m_vbo;

	int				m_heightmap_width;
	int				m_heightmap_height;

	bool			m_no_error;
};


#endif
