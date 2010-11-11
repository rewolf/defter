/*****************************************************************************
 * skybox: Handles code for rendering a skybox in the program
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "constants.h"
#include "skybox.h"

//--------------------------------------------------------
Skybox::Skybox()
{
	m_error = true;
	m_shSky = new ShaderProg("shaders/skybox.vert","","shaders/skybox.frag");

	if (!m_shSky->CompileAndLink())
		return;

	glUseProgram(m_shSky->m_programID);
	glUniform1i(glGetUniformLocation(m_shSky->m_programID, "sky"), 0);

	// Load texture
	if (!LoadPNG(&m_tex, SKYBOX_TEXTURE))
		return;

	// make the box
	const float boxverts[12 * 3] = {
		-500.0f,	700.0f,		-500.0f, // top
		 500.0f,	700.0f,		-500.0f,
		 500.0f,	700.0f,		 500.0f,
		-500.0f,	700.0f,		 500.0f,
		-500.0f,	-300.0f,	-500.0f, // far flap
		 500.0f,	-300.0f,	-500.0f,
		 500.0f,	-300.0f,	-500.0f, // right flap
		 500.0f,	-300.0f,	 500.0f,
		 500.0f,	-300.0f,	 500.0f, // near flap
		-500.0f,	-300.0f,	 500.0f,
		-500.0f,	-300.0f,	 500.0f, // left flap
		-500.0f,	-300.0f,	-500.0f
	};

	const float boxtex[12 * 2] = {
		 .25f,		 .25f,
		 .75f,		 .25f,
		 .75f,		 .75f,
		 .25f,		 .75f,
		 .25f,		 .0f,
		 .75f,		 .0f,
		 1.0f,		 .25f,
		 1.0f,		 .75f,
		 .75f,		 1.0f,
		 .25f,		 1.0f,
		 .0f,		 .75f,
		 .0f,		 .25f
	};

	const GLubyte indices[5 * 6] = {
		 2, 3, 0,	 2, 0, 1,
		 1, 0, 4,	 1, 4, 5,
		 2, 1, 6,	 2, 6, 7,
		 3, 2, 8,	 3, 8, 9,
		 0, 3,10,	 0,10,11
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(3, m_vbo);
	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxverts), boxverts, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxtex), boxtex, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	m_error = false;
}


//--------------------------------------------------------
Skybox::~Skybox()
{
	RE_DELETE(m_shSky);
	glDeleteBuffers(3, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
}

//--------------------------------------------------------
bool
Skybox::HasError(void)
{
	return (m_error);
}

//--------------------------------------------------------
void
Skybox::render(matrix4 &transform)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glUseProgram(m_shSky->m_programID);

	glUniformMatrix4fv(glGetUniformLocation(m_shSky->m_programID, "mvpMatrix"), 1, GL_FALSE, transform.m);

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_BYTE, 0);
}
