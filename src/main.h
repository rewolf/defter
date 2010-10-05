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

	bool		LoadHeightmap	(string filename); 

public:
	ShaderProg*		m_shMain;	// use the provided shader program class
	Deform*			m_pDeform;
	Skybox*			m_pSkybox;
	Clipmap*		m_pClipmap;
	Caching*		m_pCaching;

	GLuint			m_coarsemap_tex;
	GLuint			m_normalmap_tex;
	GLuint			m_tangentmap_tex;
	int				m_coarsemap_width;
	int				m_coarsemap_height;
	GLuint			m_colormap_tex;

	matrix4			m_proj_mat;
	vector3			m_cam_rotate;
	vector3			m_cam_translate;
	vector2			m_clipmap_shift;
};


#endif
