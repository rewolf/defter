/*****************************************************************************
 * Header: deform
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _DEFORM_H_
#define _DEFORM_H_

class Deform
{
public:
	Deform					(int coarseDim, int highDim, float metre_to_tex, float metre_to_detail_tex);
	~Deform();

	bool HasError			(void);
	void init_backups		(void);

	// Two methods, one takes in a stamp ID the other the stamp name
	void displace_heightmap	(TexData texdata, vector2 clickPos, vector2 clickOffset, vector4 SIRM, bool isCoarse, string stampName = "", GLuint copySrcTex = 0);

	void calculate_pdmap	(TexData texdata, vector2 clickPos, vector2 clickOffset, float scale, bool isCoarse, bool init = false);
	void create_pdmap		(TexData texdata, bool isCoarse);

	GLuint			m_coarseBackup;
	GLuint			m_highBackup;
	map<string, Stamp> stampCollection;
private:
	ShaderProg*		m_shPDMapper;

	bool			m_error;

	// FBOs
	GLuint			m_fbo_heightmap;

	// VAOs & VBOs
	GLuint			m_vao;
	GLuint			m_vbo;

	int				m_coarseDim;
	int				m_highDim;
	float			m_metre_to_tex;
	float			m_metre_to_detail_tex;
	bool			m_initialised;
};

#endif
