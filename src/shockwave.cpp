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
Shockwave::Shockwave(TexData coarsemap, int dimension, Deform* deformer)
{
	m_pDeform		= deformer;
	m_error			= true;
	m_coarsemap		= coarsemap;
	m_dimension		= dimension;

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

	// Kill all the wavelet objects
	for (int i = 0; i < m_shockwavelets.size(); i++)
		RE_DELETE(m_shockwavelets.at(i));
}

//--------------------------------------------------------
bool
Shockwave::HasError(void)
{
	return (m_error);
}

//--------------------------------------------------------
void
Shockwave::Update(float dt)
{
	int currentViewport[4];

	for (int i = 0; i < m_shockwavelets.size(); i++)
	{
		Shockwavelet* wave = m_shockwavelets.at(i);

		if (wave->m_state == IDLE)
			continue;

		// Set the Scale for the wave
		vector4 SIRM;
		SIRM.x = wave->m_AOE;

		// Always skip deletion of first wave
		if (wave->m_firstWave)
		{
			wave->m_firstWave = false;
		}
		else
		{
			glGetIntegerv(GL_VIEWPORT, currentViewport);
			glViewport(0, 0, m_dimension, m_dimension);

			// Bind shader, framebuffer and VAO
			glUseProgram(m_shWave->m_programID);
			glUniform1f(glGetUniformLocation(m_shWave->m_programID, "R"),  wave->m_radius);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_stampTex, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glBindVertexArray(m_vao);

			// write to currentTex
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);	
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


			SIRM.y = -0.75f * wave->m_height;
			m_pDeform->EdgeDeform(m_coarsemap, wave->m_origin, SIRM, "Shockwave");
		}
	}

	for (int i = 0; i < m_shockwavelets.size(); i++)
	{
		Shockwavelet* wave = m_shockwavelets.at(i);

		if (wave->m_state == IDLE)
			continue;

		// Set the Scale for the wave
		vector4 SIRM;
		SIRM.x = wave->m_AOE;

		// Move wave
		wave->m_radius += (wave->m_velocity * dt);
		// if its large enough, start decaying
		// H(n+1) = c^{dt} * H(n)
		// => H(t) = c^{t} * H(0)   // where t=0 is when decaying starts
		// => t = log(H(t)/H(0)) / log( c^{100} )  // 
		// The height only starts to decay after the radius is larger than 0.1 (10% max)
		if (wave->m_radius > SWNODECAYRADIUS)
			wave->m_height *= powf(wave->m_decayRate, dt);
	
		if (wave->m_height < SWTARGETHEIGHT)
			wave->m_state = IDLE;
		else
		{
			glGetIntegerv(GL_VIEWPORT, currentViewport);
			glViewport(0, 0, m_dimension, m_dimension);

			// Bind shader, framebuffer and VAO
			glUseProgram(m_shWave->m_programID);
			glUniform1f(glGetUniformLocation(m_shWave->m_programID, "R"),  wave->m_radius);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_stampTex, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glBindVertexArray(m_vao);

			// write to currentTex
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);	
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

			SIRM.y = 0.75f * wave->m_height;
			m_pDeform->EdgeDeform(m_coarsemap, wave->m_origin, SIRM, "Shockwave");
		}
	}
}

//--------------------------------------------------------
void
Shockwave::CreateShockwave(vector2 pos, float AOE, float h, float vel)
{
	for (int i = 0; i < m_shockwavelets.size(); i++)
	{
		if (m_shockwavelets.at(i)->m_state == IDLE)
		{
			m_shockwavelets.at(i)->Reset(pos, AOE, h, vel);
			return;
		}
	}

	if (m_shockwavelets.size() >= SWMAXWAVELETS)
	{
		printf("Too many waves!!!!\n");
		return;
	}

	// If getting here then no idle shockwavelets found =( so create one =)
	m_shockwavelets.push_back(new Shockwavelet(pos, AOE, h, vel));
}
