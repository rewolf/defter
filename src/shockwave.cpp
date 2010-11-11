/*****************************************************************************
 * shockwave: Creates and updates a shockwave in the terrain
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "constants.h"
using namespace std;
#include "deform.h"
#include "shockwave.h"

//--------------------------------------------------------
Shockwave::Shockwave(TexData coarsemap, int dimension)
{
	m_state			= IDLE;
	m_error			= true;
	m_coarsemap		= coarsemap;
	m_dimension		= dimension;
	m_height		= 1.0f;
	m_decayRate		= 1.0f;
	m_velocity		= 0.0f;

	// Setup shader
	m_shWave = new ShaderProg("shaders/shockwave.vert", "", "shaders/shockwave.frag");
	glBindAttribLocation(m_shWave->m_programID, 0, "vert_Position");
	if (!m_shWave->CompileAndLink())
		return;

	glUseProgram(m_shWave->m_programID);

	// Initialise Render Quad
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	const float renderQuad[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},{1.0f, 1.0f}};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, renderQuad, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// Create textures
	glGenTextures(1, &m_stampTex);

	// Create texture for initial state
	float* initialData =	new float[dimension * dimension];
	for (int y = 0; y < dimension; y++){
		for (int x = 0; x < dimension; x++){
			int i = y * dimension +x;
			initialData[i] = .0f;
		}
	}
	glBindTexture(GL_TEXTURE_2D, m_stampTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dimension, dimension, 0, GL_RED, GL_FLOAT, initialData);
	delete[] initialData;

	// Setup framebuffer
	glGenFramebuffers(1, &m_fbo);

	// Add the shockwave stamp
	if (!GetStampMan()->AddDynstamp("Shockwave", m_stampTex))
		return;

	m_error = false;
}

//--------------------------------------------------------
Shockwave::~Shockwave()
{
	glDeleteBuffers(1, &m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteTextures(1, &m_stampTex);
}

//--------------------------------------------------------
bool
Shockwave::HasError(void)
{
	return (m_error);
}

//--------------------------------------------------------
bool
Shockwave::IsActive(void)
{
	return (m_state == ACTIVE);
}

//--------------------------------------------------------
bool
Shockwave::IsFirst(void)
{
	if (m_firstWave)
	{
		m_firstWave ^= true;
		return true;
	}
	else
		return false;
}

//--------------------------------------------------------
float
Shockwave::GetHeight(void)
{
	return (m_height);
}

//--------------------------------------------------------
vector2
Shockwave::GetEpicenter(void)
{
	return (m_origin);
}

//--------------------------------------------------------
float
Shockwave::GetAOE(void)
{
	return (m_AOE);
}

//--------------------------------------------------------
void
Shockwave::Update(float dt)
{
	int currentViewport[4];

	if (m_state == IDLE)
		return;

	glGetIntegerv(GL_VIEWPORT, currentViewport);
	glViewport(0, 0, m_dimension, m_dimension);

	// Bind shader, framebuffer and VAO
	glUseProgram(m_shWave->m_programID);
	glUniform1f(glGetUniformLocation(m_shWave->m_programID, "R"),  m_radius);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_stampTex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindVertexArray(m_vao);

	// write to currentTex
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Move wave
	m_radius += (m_velocity * dt);
	// if its large enough, start decaying
	// H(n+1) = c^{dt} * H(n)
	// => H(t) = c^{t} * H(0)   // where t=0 is when decaying starts
	// => t = log(H(t)/H(0)) / log( c^{100} )  // 
	// The height only starts to decay after the radius is larger than 0.1 (10% max)
	if (m_radius > SWNODECAYRADIUS)
		m_height *= powf(m_decayRate, dt);
	if (m_height < SWTARGETHEIGHT)
		m_state = IDLE;
}

//--------------------------------------------------------
void
Shockwave::CreateShockwave(vector2 position, float areaOfEffect, float height, float velocity)
{
	m_state		= ACTIVE;
	m_firstWave	= true;
	m_origin	= position;
	m_radius	= 0.0f;
	m_AOE		= areaOfEffect;
	m_height	= height;
	m_velocity	= velocity;
	// r = vt   ;  H(t) = c^{t} H(0)
	// t = (R-R0)/v  => time taken since the decay starts
	// H( (R-R0)/v  )  <  eps;
	m_decayRate = powf(SWTARGETHEIGHT / m_height, m_velocity / (1.0f - SWNODECAYRADIUS));
}
