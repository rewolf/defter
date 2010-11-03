
/*****************************************************************************
 * shockwave: Creates and updates a shockwave in the terrain
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include <limits.h>
#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include <map>
using namespace std;
#include "re_shader.h"
#include "util.h"
#include "deform.h"
#include "shockwave.h"

//--------------------------------------------------------
Shockwave::Shockwave(TexData coarsemap, int dimension){
	m_state			= IDLE;
	m_no_error		= true;
	m_coarsemap		= coarsemap;
	m_dimension		= dimension;
	m_height		= 1.0f;

	// Setup shader
	m_shWave = new ShaderProg("shaders/shockwave.vert", "", "shaders/shockwave.frag");
	glBindAttribLocation(m_shWave->m_programID, 0, "vert_Position");
	m_no_error &= m_shWave->CompileAndLink();

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dimension, dimension, 0, GL_RED, GL_FLOAT,
			initialData);
	delete[] initialData;

	// Setup framebuffer
	glGenFramebuffers(1, &m_fbo);
}

//--------------------------------------------------------
Shockwave::~Shockwave(){
	glDeleteBuffers(1, &m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteTextures(1, &m_stampTex);
}

//--------------------------------------------------------
void
Shockwave::update(float dt){
	int currentViewport[4];

	if (m_state == IDLE )
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
	m_radius += .4*dt;
	// if its large enough, start decaying
	if (m_radius > .2f)
		m_height *= .98f;
	if (m_height < .05f)
		m_state = IDLE;
}

//--------------------------------------------------------
void
Shockwave::create(vector3 position){
	m_state = ACTIVE;
	m_origin= vector2(position.x, position.z);
	m_radius= .0f;
	m_height= 1.0f;
}






