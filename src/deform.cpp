/*****************************************************************************
 * deform: Creates deformations on a heightmap and calculates partial
 *		   derivative maps
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include <map>
using namespace std;
#include "re_shader.h"
#include "util.h"
#include "deform.h"

//--------------------------------------------------------
Deform::Deform(int coarseDim, int highDim, float metre_to_tex, float metre_to_detail_tex)
{
	const float renderQuad[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
		{1.0f, 1.0f}};

	m_no_error				= true;
	m_coarseDim				= coarseDim;
	m_highDim				= highDim;
	m_metre_to_tex			= metre_to_tex;
	m_metre_to_detail_tex	= metre_to_detail_tex;

	// Setup heightmap framebuffer
	glGenFramebuffers(1, &m_fbo_heightmap);

	// Setup shader
	m_shTexStamp = new ShaderProg("shaders/tex_stamps.vert", "", "shaders/tex_stamps.frag");
	m_shPDMapper = new ShaderProg("shaders/calc_pdmap.vert", "", "shaders/calc_pdmap.frag");
	glBindAttribLocation(m_shTexStamp->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shPDMapper->m_programID, 0, "vert_Position");
	m_no_error &= (m_shTexStamp->CompileAndLink() == 1 && m_shPDMapper->CompileAndLink() == 1);

	// Create VAO and VBO
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, renderQuad, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	m_initialised = false;

	// Add in the stamps
	Stamp newStamp;

	// Gaussian function stamp
	newStamp.initShader = &setupGaussian;
	m_no_error &= newStamp.SetupShader("shaders/gaussian.vert", "shaders/gaussian.frag");
	stampCollection["Gaussian"] = newStamp;

	// Testing image stamp
	newStamp = Stamp();
	m_no_error &= newStamp.LoadTexture("images/stamps/percent.png");
	stampCollection["%"] = newStamp;

	// Footprint stamp
	newStamp = Stamp();
	m_no_error &= newStamp.LoadTexture("images/stamps/leftfoot.png");
	stampCollection["leftfoot"] = newStamp;
	// Add other stamps...
}

//--------------------------------------------------------
Deform::~Deform()
{
	glDeleteFramebuffers(1, &m_fbo_heightmap);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	RE_DELETE(m_shTexStamp);
	RE_DELETE(m_shPDMapper);
}

//--------------------------------------------------------
void
Deform::displace_heightmap(TexData texdata, vector2 clickPos, vector2 clickOffset, string stampName, vector4 SIRM,
	bool isCoarse, GLuint copySrcTex)
{
	int 	viewport[4];
	int		dim 		= isCoarse ? m_coarseDim : m_highDim;
	float	metre_scale	= isCoarse ? m_metre_to_tex : m_metre_to_detail_tex;
	clickPos			= isCoarse ? (clickPos * metre_scale) + vector2(0.5f) : (clickPos * metre_scale);
	clickPos		   += clickOffset;
	SIRM.x				= 0.5f * SIRM.x * metre_scale;
	matrix2 stampRot	= rotate_tr2(SIRM.z);
	GLenum	bpp			= isCoarse ? GL_R16 	 : GL_R8;

	GLuint	backupTex;
	// Stamp controls
	Stamp stamp			= stampCollection[stampName];
	GLuint shaderID		= stamp.m_isTexStamp ? m_shTexStamp->m_programID : stamp.m_shader->m_programID;

	static reTimer timer;
	glFinish();
	timer.start();
	// check if doing a high detail deform
	if (!isCoarse || !m_initialised)
	{
		// Copy original map into backup
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		// if we're setting the tex to have a base of copySrcTex before deforming..
		if (copySrcTex!=0)
		{
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					copySrcTex, 0);
			glBindTexture(GL_TEXTURE_2D, texdata.heightmap);
			backupTex = copySrcTex;
		}
		else
		{
			// Create the pingpong texture
			glGenTextures(1, &backupTex);
			glBindTexture(GL_TEXTURE_2D, backupTex);
			glTexImage2D(GL_TEXTURE_2D, 0, bpp, dim, dim, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					texdata.heightmap, 0);
			glBindTexture(GL_TEXTURE_2D, backupTex);
		}

		glCopyTexImage2D(GL_TEXTURE_2D, 0, bpp, 0, 0, dim, dim, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		// set that has been initialised
		if (!m_initialised && isCoarse)
		{
			m_initialised = true;
			m_coarseBackup = backupTex;
		}
	}
	else
		backupTex = m_coarseBackup;
	glFinish();
	printf("\t\tCopy: %.3fms\n", timer.getElapsed()*1000);

	timer.start();
	// Acquire current viewport origin and extent
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Prepare viewport for texture render
	glViewport(0, 0, dim, dim);

	// Enable the shader
	glUseProgram(shaderID);
	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backupTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	//Bind the stamp texture if it uses one
	if (stamp.m_isTexStamp)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, stamp.m_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	
	// Set the Shader parameters
	glUniform1i(glGetUniformLocation(shaderID, "in_heightmap"), 0);

	glUniform2fv(glGetUniformLocation(shaderID, "clickPos"), 1, clickPos.v);
	glUniform2f(glGetUniformLocation(shaderID, "stamp_scale"), SIRM.x, SIRM.x);
	glUniformMatrix2fv(glGetUniformLocation(shaderID, "stamp_rotation"), 1, GL_FALSE, stampRot.m);
	if (stamp.m_isTexStamp)
	{
		glUniform1i(glGetUniformLocation(shaderID, "in_stampmap"), 1);

		// Controls for mirroring the texture
		vector2 mirror(0.0f, -1.0f);
		if (SIRM.w != 0)
			mirror = vector2(1.0f);
		glUniform2fv(glGetUniformLocation(shaderID, "stamp_mirror"), 1, mirror.v);
	}

	glUniform1f(glGetUniformLocation(shaderID, "intensity"), SIRM.y);

	// Call a predefined function if need be
	if (stamp.initShader)
		stamp.initShader(stamp, clickPos, SIRM.x, SIRM.y);

	// Bind the Framebuffer and set it's color attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.heightmap, 0);

	// Set the draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	// Bind the VAO
	glBindVertexArray(m_vao);
	glFinish();
	printf("\t\tSetup: %.3fms\n", timer.getElapsed()*1000);
	timer.start();
	// Render the new heightmap
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glFinish();
	printf("\t\tRender: %.3fms\n", timer.getElapsed()*1000);
	timer.start();

	// Reset viewport and framebuffer as it was
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	glBindTexture(GL_TEXTURE_2D, texdata.heightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);
	if (isCoarse)
	{
		// Setup textures to copy heightmap changes to the other map
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				texdata.heightmap, 0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindTexture(GL_TEXTURE_2D, backupTex);

		// Setup the regions for copying
		// Do a slightly larger area than required to allow for rotation
		int copyW = (int)ceil(2.9f * SIRM.x * dim) ;
		int copyH = (int)ceil(2.9f * SIRM.x * dim);
		int copyX = max(0, (int)(dim * clickPos.x) - copyW / 2);
		int copyY = max(0, (int)(dim * clickPos.y) - copyH / 2);

		// Make sure it's not out of bounds
		copyW = copyX + copyW > dim-1 ? dim - copyX : copyW;
		copyH = copyY + copyH > dim-1 ? dim - copyY : copyH;
		// Copy the changed subimage
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, copyX, copyY, copyX, copyY, copyW, copyH);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	}

	//Delete the created memory if need be
	if (!isCoarse && copySrcTex == 0)
		glDeleteTextures(1, &backupTex);
	glFinish();
	printf("\t\tEnd: %.3fms\n", timer.getElapsed()*1000);
}

//--------------------------------------------------------
void
Deform::calculate_pdmap(TexData texdata, vector2 clickPos, vector2 clickOffset, float scale, bool isCoarse, bool init){
	int 	viewport[4];
	int		dim 		= isCoarse ? m_coarseDim : m_highDim;
	float	metre_scale	= isCoarse ? m_metre_to_tex : m_metre_to_detail_tex;

	if (!init)
	{
		clickPos			= isCoarse ? (clickPos * metre_scale) + vector2(0.5f) : (clickPos * metre_scale);
		scale				= 0.75f * scale * metre_scale;
		clickPos += clickOffset;
	}
	else
	{
		clickPos		= vector2(0.5f);
		scale			= 1.0f;
	}

	// Acquire current viewport origin and extent
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Prepare viewport for texture render
	glViewport(0, 0, dim, dim);
	// Enable the shader
	glUseProgram(m_shPDMapper->m_programID);

	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texdata.heightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set the Shader sampler
	glUniform1i(glGetUniformLocation(m_shPDMapper->m_programID, "in_heightmap"), 0);
	glUniform1f(glGetUniformLocation(m_shPDMapper->m_programID, "tc_delta"), 1.0f / dim);
	glUniform2f(glGetUniformLocation(m_shPDMapper->m_programID, "clickPos"), clickPos.x, clickPos.y);
	glUniform2f(glGetUniformLocation(m_shPDMapper->m_programID, "stamp_scale"), scale, scale);

	// Bind the Framebuffer and set it's color attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.pdmap, 0);

	// Set the draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	// Bind the VAO
	glBindVertexArray(m_vao);

	// Render the new heightmap
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Reset viewport and framebuffer as it was
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Generate Mipmaps
	glBindTexture(GL_TEXTURE_2D, texdata.pdmap);
	glGenerateMipmap(GL_TEXTURE_2D);
}

//--------------------------------------------------------
void
Deform::create_pdmap(TexData texdata, bool isCoarse)
{
	calculate_pdmap(texdata, vector2(0.0f), vector2(0.0f), 0.0f, isCoarse, true);
}

//--------------------------------------------------------
void
setupGaussian(Stamp stamp, vector2 clickPos, float scale, float intensity)
{
	const float epsilon = 0.000001f;

	float falloff = - log(epsilon / fabsf(intensity)) / (scale * scale);
	glUniform1f(glGetUniformLocation(stamp.m_shader->m_programID, "falloff"), falloff);
}
