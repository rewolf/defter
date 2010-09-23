
#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_shader.h"
#include "skybox.h"
#include "util.h"


//--------------------------------------------------------
Skybox::Skybox(){
	int w,h;
	m_no_error = true;
	m_shSky = new ShaderProg("shaders/skybox.vert","","shaders/skybox.frag");

	m_no_error &= m_shSky->CompileAndLink();
	glUseProgram(m_shSky->m_programID);
	glUniform1i(glGetUniformLocation(m_shSky->m_programID, "sky"), 0);

	// Load texture
	LoadTexturePNG(&m_tex, &w, &h, "images/skybox002.png");

	// make the box
	const float boxverts[12 * 3] = {
		-.5f,	.7f,	-.5f, // top
		 .5f,	.7f,	-.5f,
		 .5f,	.7f,	 .5f,
		-.5f,	.7f,	 .5f,
		-.5f,	-.3f,	-.5f, // far flap
		 .5f,	-.3f,	-.5f,
		 .5f,	-.3f,	-.5f, // right flap
		 .5f,	-.3f,	 .5f,
		 .5f,	-.3f,	 .5f, // near flap
		-.5f,	-.3f,	 .5f,
		-.5f,	-.3f,	 .5f, // left flap
		-.5f,	-.3f,	-.5f
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
}


//--------------------------------------------------------
Skybox::~Skybox(){
	RE_DELETE(m_shSky);
	glDeleteBuffers(3, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
}

//--------------------------------------------------------
void
Skybox::render(matrix4 &transform){
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glUseProgram(m_shSky->m_programID);

	glUniformMatrix4fv(glGetUniformLocation(m_shSky->m_programID, "mvpMatrix"), 1, GL_FALSE,
			transform.m);

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_BYTE, 0);
}

