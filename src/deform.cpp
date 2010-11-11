/*****************************************************************************
 * deform: Creates deformations on a heightmap and calculates partial
 *		   derivative maps
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "constants.h"
using namespace std;
#include "deform.h"

//--------------------------------------------------------
Deform::Deform(int coarseDim, int highDim, float metre_to_tex, float metre_to_detail_tex)
{
	const float renderQuad[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
		{1.0f, 1.0f}};

	m_error					= false;
	m_coarseDim				= coarseDim;
	m_highDim				= highDim;
	m_metre_to_tex			= metre_to_tex;
	m_metre_to_detail_tex	= metre_to_detail_tex;

	// Setup heightmap framebuffer
	glGenFramebuffers(1, &m_fbo_heightmap);

	// Setup shader
	m_shPDMapper = new ShaderProg("shaders/calc_pdmap.vert", "", "shaders/calc_pdmap.frag");
	glBindAttribLocation(m_shPDMapper->m_programID, 0, "vert_Position");
	m_error &= !m_shPDMapper->CompileAndLink();

	// Set uniform values
	glUseProgram(m_shPDMapper->m_programID);
	glUniform1i(glGetUniformLocation(m_shPDMapper->m_programID, "in_heightmap"), 0);

	// Create VAO and VBO
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, renderQuad, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	m_initialised = false;
	init_backups();
}

//--------------------------------------------------------
Deform::~Deform()
{
	glDeleteFramebuffers(1, &m_fbo_heightmap);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	RE_DELETE(m_shPDMapper);
	glDeleteTextures(1, &m_coarseBackup);
	glDeleteTextures(1, &m_highBackup);
}

//--------------------------------------------------------
bool
Deform::HasError(void)
{
	return (m_error);
}

//--------------------------------------------------------
void
Deform::init_backups()
{
	glGenTextures(1, &m_coarseBackup);
	glBindTexture(GL_TEXTURE_2D, m_coarseBackup);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_coarseDim, m_coarseDim, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glGenTextures(1, &m_highBackup);
	glBindTexture(GL_TEXTURE_2D, m_highBackup);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_highDim, m_highDim, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);
}

// Perform the coarsemap deformation which handles edge deformations as well
void
Deform::EdgeDeform(TexData texdata, vector2 clickPos, vector4 SIRM, string stampName)
{
	vector2 areaMin(clickPos - vector2(SIRM.x / 2.0f));
	vector2 areaMax(areaMin	+ SIRM.x);

	areaMin *= m_metre_to_tex;
	areaMin += vector2(0.5f);
	areaMax *= m_metre_to_tex;

	areaMax += vector2(0.5f);

	list<vector2> fuck;

	// Left-Col
	if (areaMin.x < 0.0)
	{
		// Left-Top
		if (areaMin.y < 0.0)
			fuck.push_back(vector2(1.0f, 1.0f));

		// Left-Centre
		if (areaMax.y > 0.0 && areaMin.y < 1.0)
			fuck.push_back(vector2(1.0f, 0.0f));

		// Left-Bottom
		if (areaMax.y > 1.0)
			fuck.push_back(vector2(1.0f, -1.0f));
	}

	// Centre-Col
	if (areaMin.x < 1.0 && areaMax.x > 0.0)
	{
		// Centre-Top
		if (areaMin.y < 0.0)
			fuck.push_back(vector2(0.0f, 1.0f));

		// Centre-Centre
		if (areaMax.y > 0.0 && areaMin.y < 1.0)
			fuck.push_back(vector2(0.0f));
				
		// Centre-Bottom
		if (areaMax.y > 1.0)
			fuck.push_back(vector2(0.0f, -1.0f));
	}

	// Right-Col
	if (areaMax.x > 1.0)
	{
		// Right-Top
		if (areaMin.y < 0.0)
			fuck.push_back(vector2(-1.0f, 1.0f));

		// Right-Centre
		if (areaMax.y > 0.0 && areaMin.y < 1.0)
			fuck.push_back(vector2(-1.0f, 0.0f));

		// Right-Bottom
		if (areaMax.y > 1.0)
			fuck.push_back(vector2(-1.0f, -1.0f));
	}
			
	// Displace the heightmap
	for (list<vector2>::iterator shit = fuck.begin(); shit != fuck.end(); shit++)
		displace_heightmap(texdata, clickPos, *shit, SIRM, true, stampName);

	// Calculate the normals
	for (list<vector2>::iterator shit = fuck.begin(); shit != fuck.end(); shit++)
		calculate_pdmap(texdata, clickPos, *shit, SIRM.x, true);
}

//--------------------------------------------------------
// Used to apply deformations to the given heightmap
void
Deform::displace_heightmap(TexData texdata, vector2 clickPos, vector2 clickOffset, vector4 SIRM, bool isCoarse, string stampName, GLuint copySrcTex)
{
	GLuint	backupTex;
	int 	currentViewport[4];
	int		dim;
	int		copyX, copyY;
	int		copyW, copyH;
	float	metre_scale;
	matrix2 stampRot;
	Stamp	*stamp;
	GLuint	shaderID;

	// Setup transforms
	SIRM.x			= 0.5f * SIRM.x;				// scale in metres
	stampRot		= rotate_tr2(SIRM.z);			// rotation

	// Stamp controls
	if (stampName.compare("") == 0)
		stamp		= GetStampMan()->GetCurrentStamp();
	else
		stamp		= GetStampMan()->GetStamp(stampName);
	shaderID		= stamp->GetShaderID();

	// Setup variables dependent on whether it is Coarse or High detail deformation
	if (isCoarse)
	{
		dim			= m_coarseDim;
		metre_scale = m_metre_to_tex;
		SIRM.x		= SIRM.x * metre_scale;
		clickPos	= (clickPos * metre_scale) + vector2(0.5f) + clickOffset;
		backupTex	= m_coarseBackup;
	}
	else
	{
		dim			= m_highDim;
		metre_scale	= m_metre_to_detail_tex;
		SIRM.x		= SIRM.x * metre_scale;
		clickPos	= (clickPos * metre_scale) + clickOffset;
		backupTex	= m_highBackup;
	}

	// Calculate bounds of the render area as texel coordinates where X,Y in [0, dim-1]
	copyW 		= (int)ceil(2.9f * SIRM.x * dim) ;
	copyH 		= (int)ceil(2.9f * SIRM.x * dim);
	copyX 		= max(0, (int)(dim * clickPos.x) - copyW / 2);
	copyY 		= max(0, (int)(dim * clickPos.y) - copyH / 2);

	// Make sure it's not out of bounds
	copyW 		= copyX + copyW > dim-1 ? dim - copyX : copyW;
	copyH 		= copyY + copyH > dim-1 ? dim - copyY : copyH;

	////// PHASE 1: Copy To Backup
	//////////////////////////////
	// For HD textures, we need to copy the heightmap into a double buffer for it to read from
	// For Coarsemap, we only need to do a copy the first time as it does not share the texture
	if (!isCoarse || !m_initialised) {
		// Setup the Read framebuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		// Set up textures and attachments for the copy
		if (copySrcTex != 0){
			// If we're writing to an HD for the first time, we need to fill it with the Zero tex
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					copySrcTex, 0);
			glBindTexture(GL_TEXTURE_2D, texdata.heightmap);
			// Use the Zero texture to read the current state from
			backupTex = copySrcTex;
		} 
		else {
			// Otherwise copy the current state of the render region into the backup
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
					texdata.heightmap, 0);
			glBindTexture(GL_TEXTURE_2D, backupTex);
		}

		// Perform copy
		if (isCoarse || copySrcTex != 0)
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, dim, dim);
		else
			//glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, dim, dim);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, copyX, copyY, copyX, copyY, copyW, copyH);

		// Unbind FBO, texture and regenerate mipmap
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		// set that has been initialised
		m_initialised |= isCoarse;
	}

	////// PHASE 2: Render to Texture
	/////////////////////////////////
	// First we set up the Framebuffer and it's viewport and bind our target attachment
	glGetIntegerv(GL_VIEWPORT, currentViewport);
	glViewport(0, 0, dim, dim);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.heightmap, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// Bind the source texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backupTex);

	//Bind the stamp texture if it uses one
	if (stamp->IsTexStamp())
	{
		glActiveTexture(GL_TEXTURE1);
		stamp->BindTexture();
	}
	
	// Bind the shader and set the uniform values
	glUseProgram(shaderID);
	glUniform2fv(glGetUniformLocation(shaderID, "clickPos"), 1, clickPos.v);
	glUniform2f (glGetUniformLocation(shaderID, "stamp_scale"), SIRM.x, SIRM.x);
	glUniformMatrix2fv(glGetUniformLocation(shaderID, "stamp_rotation"), 1, GL_FALSE, stampRot.m);
	if (stamp->IsTexStamp())
	{
		// Controls for mirroring the texture
		vector2 mirror(0.0f, -1.0f);
		if (SIRM.w != 0)
			mirror = vector2(1.0f);
		glUniform2fv(glGetUniformLocation(shaderID, "stamp_mirror"), 1, mirror.v);
	}

	glUniform1f(glGetUniformLocation(shaderID, "intensity"), SIRM.y);

	// Call a predefined function if need be
	if (stamp->initShader)
		stamp->initShader(stamp, clickPos, SIRM.x, SIRM.y);

	// Bind the Vertex Array Object containing the Render Quad and its texture coordinates
	glBindVertexArray(m_vao);

	// RENDER to (DEFORM) the heightmap
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Reset viewport and framebuffer as it was
	glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	////// PHASE 3: Regenerate Mipmap and Copy Backup
	/////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, texdata.heightmap);
	glGenerateMipmap(GL_TEXTURE_2D);

	// If it's the Coarsemap, we must copy these changes into its double buffer (backup texture)
	if (isCoarse)
	{
		// Setup textures to copy heightmap changes to the other map
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_heightmap);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				texdata.heightmap, 0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindTexture(GL_TEXTURE_2D, m_coarseBackup);

		// Copy the changed subimage
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, copyX, copyY, copyX, copyY, copyW, copyH);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);	
	}
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
		scale				= 0.5f * scale * metre_scale;
		clickPos += clickOffset;
	}
	else
	{
		clickPos		= vector2(0.5f);
		scale			= 1.0f;
	}

	// First we set up the Framebuffer and it's viewport and bind our target attachment
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, dim, dim);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_heightmap);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			texdata.pdmap, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// Enable the shader and set uniforms
	glUseProgram(m_shPDMapper->m_programID);
	glUniform1f(glGetUniformLocation(m_shPDMapper->m_programID, "tc_delta"), 1.0f / dim);
	glUniform2f(glGetUniformLocation(m_shPDMapper->m_programID, "clickPos"), clickPos.x, clickPos.y);
	glUniform2f(glGetUniformLocation(m_shPDMapper->m_programID, "stamp_scale"), scale, scale);

	// Bind the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texdata.heightmap);

	// Bind the Vertex Array Object containing the Render Quad and its texture coordinates
	glBindVertexArray(m_vao);

	// Render the new heightmap
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
