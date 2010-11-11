/*****************************************************************************
 * Header: main
 *
 * Copyright � 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _PROTO_MAIN_H
#define _PROTO_MAIN_H

#define NUM_PBOS			(1)

enum XferState {
	CHILLED, READY, BUFFERING, RETRIEVING, DONE
};

enum UseMode {
	EDIT_MODE, GAME_MODE
};

struct Flash{
	reTimer	timer;
	vector4	color;
	float	maxAlpha;
	float	inlength;
	float	outlength;
	bool	enabled;
};

class DefTer : public reGL3App
{
public:
	DefTer(AppConfig& conf);
	~DefTer();

	void		ProcessInput	(float dt);	//override
	void		Logic			(float dt); //override
	void		Render			(float dt); //override

	void		FlashScreen	(float inTime = .02f, float outTime = 1.8f, float maxAlpha = 0.9f, 
							 vector4 color=vector4(1.0, .95f, 1.0f, 1.0f));

private:
	bool		Init			(void);
	bool		InitGL			(void);

	bool		LoadCoarseMap	(string filename); 
	bool		SaveCoarseMap	(string filename);
	void		UpdateClickPos	(void);
	void		UpdateCoarsemapStreamer();
	float		InterpHeight	(vector2 worldPos);
	void		WrapEntity		(GameEntity* pEnt);

	void		RenderModel		(GameEntity* pEnt, matrix4 view);
	void		RenderNode		(Node* node, matrix4 transform);

public:
	ShaderProg*		m_shModel;
	ShaderProg*		m_shFlash;
	ShaderManager*	m_shManager;
	int				m_shmSimple;
	int				m_shmParallax;
	int				m_shmGeomTess;
	int				m_hdShaderIndex;
	Splash*			m_pSplash;
	Deform*			m_pDeform;
	Skybox*			m_pSkybox;
	Clipmap*		m_pClipmap;
	Caching*		m_pCaching;
	ModelManager*	m_pModelManager;
	UseMode			m_useMode;

	TexData			m_coarsemap;
	int				m_coarsemap_dim;
	GLuint			m_colormap_tex;

	int				m_stampIndex;
	vector4			m_stampSIRM;
	bool			m_is_hd_stamp;
	bool			m_clicked;
	vector2			m_clickPos;
	vector2			m_clickPosPrev;

	Camera*			m_pCamera;
	matrix4			m_proj_mat;
	vector2			m_clipmap_shift;
	bool			m_gravity_on;
	bool			m_is_crouching;
	bool			m_is_super_speed;
	bool			m_is_wireframe;

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

	// Footprints
	float			m_footprintDT;
	bool			m_flipFoot;
	bool			m_drawing_feet;

	// Stuff for awesome screenshot
	matrix4			m_screenshotProj;
	GLuint			m_screenshotTex;
	GLuint			m_screenshotDepth;
	GLuint			m_screenshotFBO;

	// Handles a screen flash
	Flash			m_flash;
};

// thread that retrieves the coarsemap from the PBOs
int map_retriever(void* defter);

#endif
