/*****************************************************************************
 * Header: shockwave
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _SHOCKWAVE_H_
#define _SHOCKWAVE_H_

enum State {ACTIVE, IDLE};

class Shockwave
{
public:
	Shockwave					(TexData coarsemap, int dimension);
	~Shockwave					(void);

	bool	HasError			(void);
	bool	IsActive			(void);
	bool	IsFirst				(void);
	float	GetHeight			(void);
	vector2	GetEpicenter		(void);
	float	GetAOE				(void);
	void	Update				(float dt);
	void	CreateShockwave		(vector2 position, float areaOfEffect = 50.0f, float height = 0.1f, float velocity = 0.4f);

private:
	ShaderProg*		m_shWave;
	GLuint			m_fbo;
	GLuint			m_vao;
	GLuint			m_vbo;
	GLuint			m_stampTex;
	TexData			m_coarsemap;
	State			m_state;
	bool			m_error;
	int				m_dimension;
	vector2			m_origin;
	float			m_radius;
	float			m_height;
	float			m_decayRate;
	float			m_velocity;
	float			m_AOE;
	bool			m_firstWave;
};

#endif
