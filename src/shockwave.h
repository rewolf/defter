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

struct Shockwavelet
{
	State			m_state;
	vector2			m_origin;
	float			m_radius;
	float			m_height;
	float			m_decayRate;
	float			m_velocity;
	float			m_AOE;
	bool			m_firstWave;

	Shockwavelet(vector2 position, float areaOfEffect, float height, float velocity)
	{
		Reset(position, areaOfEffect, height, velocity);
	}

	void Reset(vector2 position, float areaOfEffect, float height, float velocity)
	{
		m_state		= ACTIVE;
		m_firstWave = true;
		m_radius	= 0.0f;

		m_origin	= position;
		m_AOE		= areaOfEffect;
		m_height	= height;
		m_velocity	= velocity;

		// r = vt   ;  H(t) = c^{t} H(0)
		// t = (R-R0)/v  => time taken since the decay starts
		// H( (R-R0)/v  )  <  eps;
		m_decayRate = powf(SWTARGETHEIGHT / m_height, m_velocity / (1.0f - SWNODECAYRADIUS));
	}
};

class Shockwave
{
public:
	Shockwave					(TexData coarsemap, int dimension, Deform* deformer);
	~Shockwave					(void);

	bool	HasError			(void);
	void	Update				(float dt);
	void	CreateShockwave		(vector2 position, float areaOfEffect = 50.0f, float height = 0.1f, float velocity = 0.4f);

private:
	ShaderProg*		m_shWave;
	Deform*			m_pDeform;
	GLuint			m_fbo;
	GLuint			m_vao;
	GLuint			m_vbo;
	GLuint			m_stampTex;
	TexData			m_coarsemap;
	bool			m_error;
	int				m_dimension;

	vector<Shockwavelet*> m_shockwavelets;
};

#endif
