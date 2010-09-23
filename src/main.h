#ifndef _PROTO_MAIN_H
#define _PROTO_MAIN_H


class DefTer : public reGL3App{
public:
	DefTer(AppConfig& conf);
	~DefTer();

	void		ProcessInput	(float dt);	//override
	void		Logic			(float dt); //override
	void		Render			(float dt); //override

private:
	bool		Init			(void);
	bool		InitGL			(void);

	void		CreateClipmap	(vector3**, float2**, GLuint**, int*, int*);
	bool		LoadHeightmap	(string filename); 

public:
	ShaderProg*		m_shMain;	// use the provided shader program class
	Deform*			m_deform;
	Skybox*			m_pSkybox;

	GLuint			m_vao;
	GLuint			m_vbo[3];	// VBOs for vertices, normals, colors, tex coords, indices
	GLuint			m_heightmap_tex;
	GLuint			m_normalmap_tex;
	int				m_heightmap_width;
	int				m_heightmap_height;
	GLuint			m_colormap_tex;
	int				m_nIndices;
	float			m_scale_metreToTex;
	float			m_quadSize;

	matrix4			m_proj_mat;
	vector3			m_cam_rotate;
	vector3			m_cam_translate;
};


#endif
