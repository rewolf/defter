
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

	// Setup shader
	m_shWave = new ShaderProg("shaders/shockwave.vert", "", "shaders/shockwave.frag");
	glBindAttribLocation(m_shWave->m_programID, 0, "vert_Position");
	m_no_error &= m_shWave->CompileAndLink();

	glUseProgram(m_shWave->m_programID);
	glUniform1i(glGetUniformLocation(m_shWave->m_programID, "previous"), 0);
	glUniform1i(glGetUniformLocation(m_shWave->m_programID, "current"),  1);

	glUniform2f(glGetUniformLocation(m_shWave->m_programID, "factors"), .5f, 1.0f/m_dimension);

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
	glGenTextures(1, &m_nextTex);
	glGenTextures(1, &m_currentTex);
	glGenTextures(1, &m_previousTex);
	glGenTextures(1, &m_initialTex);

	// Create texture for initial state
	float* initialData =	new float[dimension * dimension];
	for (int y = 0; y < dimension; y++){
		for (int x = 0; x < dimension; x++){
			int i = y * dimension +x;
			int dx = dimension/2 - x;
			int dy = dimension/2 - y;
			initialData[i] = expf(-(dx*dx + dy*dy) * .0009f);

		}
	}
	glBindTexture(GL_TEXTURE_2D, m_initialTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dimension, dimension, 0, GL_RED, GL_FLOAT,
			initialData);
	glBindTexture(GL_TEXTURE_2D, m_nextTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dimension, dimension, 0, GL_RED, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, m_currentTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dimension, dimension, 0, GL_RED, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, m_previousTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dimension, dimension, 0, GL_RED, GL_FLOAT, NULL);
	delete[] initialData;

	// Setup framebuffer
	glGenFramebuffers(1, &m_fbo);
}

//--------------------------------------------------------
Shockwave::~Shockwave(){
	glDeleteBuffers(1, &m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteTextures(1, &m_nextTex);
	glDeleteTextures(1, &m_currentTex);
	glDeleteTextures(1, &m_previousTex);
	glDeleteTextures(1, &m_initialTex);
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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nextTex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindVertexArray(m_vao);

	// Bind textures
	if (m_state == INIT){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_initialTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_initialTex);
		m_state = SECOND;
	}
	else if (m_state == SECOND){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_initialTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_currentTex);
		m_state = ACTIVE;

	}else{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_previousTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_currentTex);
	}

	// write to currentTex
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	// swap pointers to buffers
	GLuint temp   = m_nextTex;
	m_nextTex	  = m_previousTex;
	m_previousTex = m_currentTex;
	m_currentTex  = temp;
}

//--------------------------------------------------------
void
Shockwave::create(vector3 position){
	m_state = INIT;
}






