/*****************************************************************************
 * Header: main
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _PROTO_MAIN_H
#define _PROTO_MAIN_H

#define NUM_PBOS			(4)

class DefTer : public reGL3App
{
public:
	DefTer(AppConfig& conf);
	~DefTer();

	void		ProcessInput	(float dt);	//override
	void		Logic			(float dt); //override
	void		Render			(float dt); //override

private:
	bool		Init			(void);
	bool		InitGL			(void);

	bool		InitSplash		(void);
	void		RenderSplash	(void);
	bool		LoadCoarseMap	(string filename); 
	bool		SaveCoarseMap	(string filename);
	void		UpdateClickPos	(void);
	void		UpdateCoarsemapStreamer();
	float		InterpHeight	(vector2 worldPos);

public:
	ShaderProg*			m_shSplash;
	GLuint				m_vbo[3];
	GLuint				m_vao;
	GLuint				m_splashmap;
	float*				m_elevationData;

	ShaderProg*		m_shMain;	// use the provided shader program class
	Deform*			m_pDeform;
	Skybox*			m_pSkybox;
	Clipmap*		m_pClipmap;
	Caching*		m_pCaching;

	TexData			m_coarsemap;
	int				m_coarsemap_dim;
	GLuint			m_colormap_tex;

	string			m_stampName;
	float			m_stampIntensity;
	float			m_stampScale;
	bool			m_is_hd_stamp;
	bool			m_clicked;
	vector2			m_clickPos;
	vector2			m_clickPosPrev;

	matrix4			m_proj_mat;
	vector3			m_cam_rotate;
	vector3			m_cam_translate;
	vector2			m_clipmap_shift;
	float			m_fall_speed;
	bool			m_gravity_on;
	bool			m_is_crouching;
	bool			m_is_super_speed;

	// Coarsemap continuous unpacking stuff
	GLuint			m_pbo[NUM_PBOS];
	GLuint			m_fboTransfer;
	SDL_mutex*		m_elevationDataMutex;
	bool			m_packedCoarseDeform;
	bool			m_isTransferring;
	reTimer			m_deformTimer;
	int				m_cyclesPassed;
};

#endif
