/*****************************************************************************
 * Header: main
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _PROTO_MAIN_H
#define _PROTO_MAIN_H

#define NUM_PBOS			(1)

enum XferState {
	CHILLED, READY, BUFFERING, RETRIEVING, DONE
};

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

	void		RenderModel		(Node* model, matrix4 view);
	void		RenderNode		(Node* node, matrix4 transform);

public:
	ShaderProg*		m_shSplash;
	GLuint			m_vbo[3];
	GLuint			m_vao;
	GLuint			m_splashmap;

	ShaderProg*		m_shMain;	// use the provided shader program class
	ShaderProg*		m_shInner;
	ShaderProg*		m_shModel;
	Deform*			m_pDeform;
	Skybox*			m_pSkybox;
	Clipmap*		m_pClipmap;
	Caching*		m_pCaching;
	ModelManager*	m_pModels;

	TexData			m_coarsemap;
	int				m_coarsemap_dim;
	GLuint			m_colormap_tex;

	string			m_stampName;
	vector4			m_stampSIRM;
	bool			m_is_hd_stamp;
	bool			m_clicked;
	vector2			m_clickPos;
	vector2			m_clickPosPrev;

	matrix4			m_proj_mat;
	vector3			m_cam_rotate;
	vector3			m_cam_translate;
	vector2			m_clipmap_shift;
	vector3			m_lastPosition;
	vector3			m_frameAcceleration;
	bool			m_hit_ground;
	bool			m_gravity_on;
	bool			m_is_crouching;
	bool			m_is_super_speed;

	// Coarsemap continuous unpacking stuff
	GLushort*		m_elevationData;
	GLushort*		m_elevationDataBuffer;
	GLuint			m_pbo[NUM_PBOS];
	GLuint			m_fboTransfer;
	SDL_Thread*		m_retrieverThread;
	SDL_mutex*		m_elevationDataMutex;
	SDL_sem*		m_waitSem;
	GLushort*		m_bufferPtr;
	
	XferState		m_XferState;
	XferState		m_otherState;
	reTimer			m_deformTimer;
	int				m_cyclesPassed;

	// footprints
	float			m_footprintDT;
	bool			m_flipFoot;
	bool			m_drawing_feet;

	// Use the second shader for inner grid
	bool			m_enableTess;

	// Stuff for awesome screenshot
	matrix4			m_screenshotProj;
	GLuint			m_screenshotTex;
	GLuint			m_screenshotDepth;
	GLuint			m_screenshotFBO;

	// Test model
	Node*			m_pModel;
};

// thread that retrieves the coarsemap from the PBOs
int map_retriever(void* defter);

#endif
