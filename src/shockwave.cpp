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
float
Shockwave::GetHeight(void)
{
	return (m_height);
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
	m_radius += (0.4f * dt);
	// if its large enough, start decaying
	if (m_radius > .2f)
		m_height *= .98f;
	if (m_height < .02f)
		m_state = IDLE;
}

//--------------------------------------------------------
void
Shockwave::CreateShockwave(vector3 position)
{
	m_state = ACTIVE;
	m_origin= vector2(position.x, position.z);
	m_radius= .0f;
	m_height= .1f;
}
