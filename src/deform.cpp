
#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_shader.h"
#include "util.h"
#include "deform.h"

//--------------------------------------------------------
Deform::Deform(int coarseDim, int highDim){
	const float renderQuad[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
		{1.0f, 1.0f}};


	m_no_error = true;
	m_coarseDim = coarseDim;
	m_highDim = highDim;

/*
	// Create the pingpong texture
	glGenTextures(1, &m_heightmap.backup);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.backup);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	*/

	// Setup heightmap framebuffer
	glGenFramebuffers(1, &m_fbo_heightmap);
	// Setup shader
	m_shDeform = new ShaderProg("shaders/deform.vert", "", "shaders/deform.frag");
	m_shNormal = new ShaderProg("shaders/calc_normal.vert", "", "shaders/calc_normal.frag");
	glBindAttribLocation(m_shDeform->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shDeform->m_programID, 0, "vert_in_texCoord");
	glBindAttribLocation(m_shNormal->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shNormal->m_programID, 0, "vert_in_texCoord");
	m_no_error &= m_shDeform->CompileAndLink() && m_shNormal->CompileAndLink();

	// Create VAO and VBO
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, renderQuad, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	/*
	// Copy original map into backup
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			m_heightmap.current, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.backup);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_R16, 0, 0, width, height, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	glGenerateMipmap(GL_TEXTURE_2D);
*/
}

//--------------------------------------------------------
Deform::~Deform(){
	//glDeleteTextures(1, &m_heightmap.backup);
	glDeleteFramebuffers(1, &m_fbo_heightmap);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	delete m_shDeform;
	delete m_shNormal;
}

//--------------------------------------------------------
void
Deform::displace_heightmap(TexData texdata, float2 tex_coord, float scale, bool isCoarse){
	int 	viewport[4];
	int		dim 			= isCoarse ? m_coarseDim : m_highDim;
	GLenum	bpp				= isCoarse ? GL_R16 	 : GL_R8;
	GLuint	backupTex;

	// Create the pingpong texture
	glGenTextures(1, &backupTex);
	glBindTexture(GL_TEXTURE_2D, backupTex);
	glTexImage2D(GL_TEXTURE_2D, 0, bpp, dim, dim, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Copy original map into backup
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.heightmap, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindTexture(GL_TEXTURE_2D, backupTex);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, bpp, 0, 0, dim, dim, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	glGenerateMipmap(GL_TEXTURE_2D);


	// Acquire current viewport origin and extent
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Prepare viewport for texture render
	glViewport(0, 0, dim, dim);
	// Enable the shader
	glUseProgram(m_shDeform->m_programID);
	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backupTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Set the Shader sampler
	vector2 dimScale(400.0f/dim);

	glUniform1i(glGetUniformLocation(m_shDeform->m_programID, "in_heightmap"), 0);
	glUniform1f(glGetUniformLocation(m_shDeform->m_programID, "tc_delta"), 1.0f/dim);
	glUniform2f(glGetUniformLocation(m_shDeform->m_programID, "thingy"), tex_coord.u, tex_coord.v);
	glUniform2f(glGetUniformLocation(m_shDeform->m_programID, "stamp_size_scale"), dimScale.x,
			dimScale.y);
	glUniform1f(glGetUniformLocation(m_shDeform->m_programID, "scale"), scale);
	// Bind the Framebuffer and set it's color attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.heightmap, 0);
	// Set the draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	// Bind the VAO
	glBindVertexArray(m_vao);

	// Render the new heightmap
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Reset viewport and framebuffer as it was
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	glGenerateMipmap(GL_TEXTURE_2D);

	// Regenerate normals and tangent
	calculate_normals(texdata, tex_coord, dimScale, isCoarse);

	glDeleteTextures(1, &backupTex);
}
//--------------------------------------------------------
void
Deform::calculate_normals(TexData texdata, float2 tex_coord, vector2 dimScale, bool isCoarse){
	int 	viewport[4];
	int		dim 			= isCoarse ? m_coarseDim : m_highDim;

	// Acquire current viewport origin and extent
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Prepare viewport for texture render
	glViewport(0, 0, dim, dim);
	// Enable the shader
	glUseProgram(m_shNormal->m_programID);
	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texdata.heightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Set the Shader sampler
	glUniform1i(glGetUniformLocation(m_shNormal->m_programID, "in_heightmap"), 0);
	glUniform1f(glGetUniformLocation(m_shNormal->m_programID, "tc_delta"), 1.0f/dim);
	glUniform2f(glGetUniformLocation(m_shNormal->m_programID, "thingy"), tex_coord.u, tex_coord.v);
	glUniform2f(glGetUniformLocation(m_shNormal->m_programID, "stamp_size_scale"), dimScale.x,
			dimScale.y);
	// Bind the Framebuffer and set it's color attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.normalmap, 0);
	if (isCoarse)
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
				texdata.tangentmap, 0);

	// Set the draw buffer
	if (isCoarse){
		GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, buffers);
	}else{
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}

	// Bind the VAO
	glBindVertexArray(m_vao);

	// Render the new heightmap
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Reset viewport and framebuffer as it was
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Generate Mipmaps
	glBindTexture(GL_TEXTURE_2D, texdata.normalmap);
	glGenerateMipmap(GL_TEXTURE_2D);
	if (isCoarse){
		glBindTexture(GL_TEXTURE_2D, texdata.tangentmap);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

