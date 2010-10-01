
#include "regl3.h"
#include "re_math.h"
#include "re_shader.h"
#include "deform.h"


//--------------------------------------------------------
Deform::Deform(texId* pHeightmap, texId* pNormalmap, texId* pTangentmap, int width, int height){
	const float renderQuad[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
		{1.0f, 1.0f}};


	m_no_error = true;
	m_pHeightmap = pHeightmap;
	m_heightmap.normalmap = *pNormalmap;
	m_heightmap.tangentmap = *pTangentmap;
	m_heightmap.current = *pHeightmap;
	m_heightmap_width = width;
	m_heightmap_height = height;


	// Create the pingpong texture
	glGenTextures(1, &m_heightmap.backup);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.backup);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	

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
	
	
	// Copy original map into backup
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			m_heightmap.current, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.backup);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_R16, 0, 0, width, height, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	glGenerateMipmap(GL_TEXTURE_2D);

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
Deform::displace_heightmap(float2 tex_coord, float scale){
	int viewport[4];

	// Acquire current viewport origin and extent
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Prepare viewport for texture render
	glViewport(0, 0, m_heightmap_width, m_heightmap_height);
	// Enable the shader
	glUseProgram(m_shDeform->m_programID);
	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.current);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Set the Shader sampler
	float2 dimScale = {400.0f/m_heightmap_width, 400.0f/m_heightmap_height};

	glUniform1i(glGetUniformLocation(m_shDeform->m_programID, "in_heightmap"), 0);
	glUniform1f(glGetUniformLocation(m_shDeform->m_programID, "tc_delta"), 1.0f/m_heightmap_width);
	glUniform2f(glGetUniformLocation(m_shDeform->m_programID, "thingy"), tex_coord.u, tex_coord.v);
	glUniform2f(glGetUniformLocation(m_shDeform->m_programID, "stamp_size_scale"), dimScale.u,
			dimScale.v);
	glUniform1f(glGetUniformLocation(m_shDeform->m_programID, "scale"), scale);
	// Bind the Framebuffer and set it's color attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_heightmap.backup, 0);
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


	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			m_heightmap.backup, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.current);

	int copyW = (int)ceil(dimScale.u * m_heightmap_width) ;
	int copyH = (int)ceil(dimScale.v * m_heightmap_height);
	int copyX = (int)(m_heightmap_width * tex_coord.u) - copyW/2;
	int copyY = (int)(m_heightmap_height * tex_coord.v) - copyH/2;
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, copyX, copyY, copyX, copyY, copyW, copyH);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	glGenerateMipmap(GL_TEXTURE_2D);

	// Regenerate normals and tangent
	calculate_normals(tex_coord, dimScale);
}
//--------------------------------------------------------

void
Deform::calculate_normals(float2 tex_coord, float2 dimScale){
	int viewport[4];

	// Acquire current viewport origin and extent
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Prepare viewport for texture render
	glViewport(0, 0, m_heightmap_width, m_heightmap_height);
	// Enable the shader
	glUseProgram(m_shNormal->m_programID);
	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.current);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Set the Shader sampler
	glUniform1i(glGetUniformLocation(m_shNormal->m_programID, "in_heightmap"), 0);
	glUniform1f(glGetUniformLocation(m_shNormal->m_programID, "tc_delta"), 1.0f/m_heightmap_width);
	glUniform2f(glGetUniformLocation(m_shNormal->m_programID, "thingy"), tex_coord.u, tex_coord.v);
	glUniform2f(glGetUniformLocation(m_shNormal->m_programID, "stamp_size_scale"), dimScale.u,
			dimScale.v);
	// Bind the Framebuffer and set it's color attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			m_heightmap.normalmap, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
			m_heightmap.tangentmap, 0);
	// Set the draw buffer
	GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, buffers);
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

	// Ping pong the textures
	glBindTexture(GL_TEXTURE_2D, m_heightmap.tangentmap);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_heightmap.normalmap);
	glGenerateMipmap(GL_TEXTURE_2D);
}

//--------------------------------------------------------
void
Deform::bind_displacers(){
	//glBindTexture(GL_TEXTURE_2D, m_heightmap.current);
}
