
#ifndef _DEFORM_H_
#define _DEFORM_H_

typedef struct{
	float u,v;
} float2;


class Deform{
public:
	Deform (int coarseDim, int highDim);
	~Deform();

	void		displace_heightmap			(TexData texdata, float2 tex_coord, float scale, bool isCoarse);
	void 		calculate_normals			(TexData texdata, float2 tex_coord, vector2 scale, bool	isCoarse);

public:
	ShaderProg*		m_shDeform;
	ShaderProg*		m_shNormal;

	// FBOs
	GLuint			m_fbo_heightmap;

	// VAOs & VBOs
	GLuint			m_vao;
	GLuint			m_vbo;

	int				m_coarseDim;
	int				m_highDim;
	GLuint			m_coarseBackup;
	bool			m_initialised;

	bool			m_no_error;
};


#endif
