/*****************************************************************************
 * util: Provides utility methods - image handling and error checking
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "constants.h"

//--------------------------------------------------------
bool
SavePNG(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip)
{

	FIBITMAP* 			image;
	FREE_IMAGE_TYPE 	type;	// needed in case of 16-bit components
	BYTE*				bits;

	// 8-bits per component
	if (bitdepth == 8)
	{
		image = FreeImage_Allocate(w, h, components * bitdepth);
	} 
	else if (bitdepth == 16)
	{
		switch(components)
		{
			case 1:
				type = FIT_UINT16;
				break;
			case 3:
				type = FIT_RGB16;		// FreeImage expects  data in BGR format
				break;
			case 4:
				type = FIT_RGBA16;		// FreeImage expects  data in BGRA format
				break;
			default:
				fprintf(stderr, "No support for 2-component textures when saving %s\n", filename);
				return false;
		}
		image = FreeImage_AllocateT(type, w, h);
	}
	else
	{
		fprintf(stderr, "Invalid bitdepth for %s.  Must be 16 or 32.\n",filename);
		return false;
	}

	if (!image)
	{
		fprintf(stderr, "Failed to allocate bitmap space for %s\n", filename);
		return false;
	}

	bits = (BYTE*) FreeImage_GetBits(image);
	memcpy(bits, data, bitdepth/8 * components * w * h);

	if (flip)
		FreeImage_FlipVertical(image);
	
	FreeImage_Save(FIF_PNG, image, filename, PNG_Z_BEST_SPEED);

	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
bool 
LoadPNG(GLuint* tex, string filename, bool flip, bool scale)
{
	FIBITMAP*		image;
	BYTE*			bits;
	int				width;
	int				height;
	int				bitdepth;

	image = FreeImage_Load(FIF_PNG, filename.c_str(), 0);

	if (!image)
	{
		fprintf(stderr, "Error\n\tCould not load PNG: %s\n", filename.c_str());
		return false;
	}

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Loading PNG texture, setting parameters"))
	{
		fprintf(stderr, "\tFile: %s\n", filename.c_str());
		return false;
	}

	if (flip)
		FreeImage_FlipVertical(image);

	// Get the width & height of the image
	width	= FreeImage_GetWidth(image);
	height	= FreeImage_GetHeight(image);

	// Scale the image if told to do so
	if (scale)
	{
		// Scaling variables
		float targetScale;
		int newW, newH;
		int offsetw, offseth;

		targetScale = max(float(SCREEN_W) / width, float(SCREEN_H) / height);

		// Calculate the new width & height based on the scale
		float newWf	= float(width) * targetScale;
		float newHf	= float(height) * targetScale;

		newW		= (int)newWf;
		newH		= (int)newHf;
		
		// Rescale the image to the new size
		image = FreeImage_Rescale(image, newW, newH, FILTER_CATMULLROM);

		// Calculate the offsets and set the new width & height
		offsetw		= (newW - SCREEN_W) / 2;
		offseth		= (newH - SCREEN_H) / 2;
		newW		= SCREEN_W;
		newH		= SCREEN_H;

		// Copy the subimage to crop the image
		image = FreeImage_Copy(image, offsetw, offseth, offsetw + newW, offseth + newH);

		// Retrieve the actual image width & height to prevent any errors
		width		= FreeImage_GetWidth(image);
		height		= FreeImage_GetHeight(image);
	}

	// Get the bit depth and retrieve the pixel data
	bits 	= (BYTE*) FreeImage_GetBits(image);
	bitdepth= FreeImage_GetBPP(image);

	// Choose based on bit depth
	switch(bitdepth)
	{
		case 32:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);
			break;
		case 24:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, bits);
			break;
		case 8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bits);
			break;
		default:
			fprintf(stderr, "Failed to load image %s of bitdepth %d\n", filename.c_str(), bitdepth);
			return false;
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	
	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
bool 
CheckError(string text)
{
	GLuint err = glGetError();

	if (err!=GL_NO_ERROR){
		fprintf(stderr, "Error\n\tOpenGL Error: ");
		switch(err)
		{
			case GL_INVALID_ENUM:
				fprintf(stderr, "Invalid Enum");
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr, "Invalid Value");
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr, "Invalid Operation");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				fprintf(stderr, "Invalid FBO operation");
				GLenum FBOErr;

				FBOErr = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
				PrintFBOErr(FBOErr);
				
				FBOErr = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
				PrintFBOErr(FBOErr);

				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "Out of Memory");
				break;
		}

		// Print out messages only if have one set
		printf("\n");
		if (text.length() != 0)
			printf ("\t[%s]\n", text.c_str());

		return false;
	}
	return true;
}
void
PrintFBOErr(GLenum err)
{
	printf("\n");
	switch (err)
	{
		case GL_FRAMEBUFFER_UNDEFINED:
			fprintf(stderr, "FB Undefined");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			fprintf(stderr, "FB Incomplete Attachment");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			fprintf(stderr, "FB Incomplete Missing Attachment");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			fprintf(stderr, "FB Incomplete Draw Buffer");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			fprintf(stderr, "FB Incomplete Read Buffer");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			fprintf(stderr, "FB Unsupported");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			fprintf(stderr, "FB Incomplete Multisample");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			fprintf(stderr, "FB Incomplete Layer Targets");
			break;
	}
}


// Util VBO Stuff
GLfloat	square[]	= { -1.0f, -1.0f,
						 1.0f, -1.0f,
						 1.0f,  1.0f,
						-1.0f,  1.0f };
GLfloat	texcoords[]	= { 0.0f, 0.0f,
						1.0f, 0.0f,
						1.0f, 1.0f,
						0.0f, 1.0f };
GLuint	indices[]	= { 3, 0, 2, 1 };
GLuint	util_vbo[3]	= {0};
GLuint	util_vao	= 0;

// Hidden Util methods
bool isInit						= false;
StampManager* m_stampManager	= NULL;
int STAMPCOUNT					= 0;

void
InitUtil(void)
{
	// Do not re-initialise
	if (isInit)
		return;

	// Create the vertex array
	glGenVertexArrays(1, &util_vao);
	glBindVertexArray(util_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(3, util_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, util_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, square, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, util_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, util_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);

	isInit = true;
}

//--------------------------------------------------------
// Util Desctuctor
//--------------------------------------------------------
void
KillUtil(void)
{
	glDeleteBuffers(3, util_vbo);
	glDeleteVertexArrays(1, &util_vao);

	RE_DELETE(m_stampManager);
}

//--------------------------------------------------------
// Return the GLuint for the standard VAO
//--------------------------------------------------------
GLuint
GetStandardVAO(void)
{
	if (!isInit)
		InitUtil();

	return (util_vao);
}

//--------------------------------------------------------
// Return the Stamp Manager object
//--------------------------------------------------------
StampManager*
GetStampMan(void)
{
	// Create the stamp manager if not already created
	if (m_stampManager == NULL)
		m_stampManager = new StampManager();

	return (m_stampManager);
}





//--------------------------------------------------------
//--------------------------------------------------------
// Splash Screen Class Def
//--------------------------------------------------------
//--------------------------------------------------------
Splash::Splash(void)
{
	m_error = true;

	// Setup shader
	m_shSplash = new ShaderProg("shaders/splash.vert", "", "shaders/splash.frag");
	glBindAttribLocation(m_shSplash->m_programID, 0, "vert_Position");
	glBindAttribLocation(m_shSplash->m_programID, 1, "vert_texCoord");
	if (!m_shSplash->CompileAndLink())
		return;

	// Load the splash map
	if (!LoadPNG(&m_splashmap, SPLASHMAP_TEXTURE, false, true))
		return;

	// Set uniforms
	glUseProgram(m_shSplash->m_programID);
	glUniform1i(glGetUniformLocation(m_shSplash->m_programID, "splashmap"), 0);

	// set that no error has occured
	m_error = false;
}

//--------------------------------------------------------
Splash::~Splash(void)
{
	RE_DELETE(m_shSplash);
	glDeleteTextures(1, &m_splashmap);
}

//--------------------------------------------------------
// Return the error status
bool
Splash::HasError(void)
{
	return m_error;
}

//--------------------------------------------------------
// Render the splash screen
void
Splash::Render(SDL_Window* window)
{
	// Clear the screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Bind the vertex array
	glBindVertexArray(GetStandardVAO());

	// Set the textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_splashmap);

	// Draw the screen
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);

	// Swap windows to show splash
	SDL_GL_SwapWindow(window);
}
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------





//--------------------------------------------------------
//--------------------------------------------------------
// Shader Manager Class Def
//--------------------------------------------------------
//--------------------------------------------------------
ShaderManager::ShaderManager(void)
{
	curIndex = 0;
}

//--------------------------------------------------------
ShaderManager::~ShaderManager(void)
{
	//Delete each shader object
	for (int i = 0; i < (int)shaders.size(); i++)
		RE_DELETE(shaders.at(i));
}

//--------------------------------------------------------
// Add a new set of shaders
bool
ShaderManager::AddShader(string vert, string geom, string frag, int *index)
{
	*index = shaders.size();

	//Create the new shader and increment the current index
	shaders.push_back(new ShaderProg(vert, geom, frag));

	//Return successful
	return (true);
}

//--------------------------------------------------------
// Bind the specific variable to a location in the shaders
void
ShaderManager::BindAttrib(char *name, int val)
{
	for (int i = 0; i < (int)shaders.size(); i++)
		glBindAttribLocation(shaders.at(i)->m_programID, val, name);
}

//--------------------------------------------------------
// Compile and link the shaders
bool
ShaderManager::CompileAndLink(void)
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		//Check for errors in compiling any shader
		if (!shaders.at(i)->CompileAndLink())
			return false;
	}

	//Success
	return true;
}

//--------------------------------------------------------
// Set the active shader to be used
void
ShaderManager::SetActiveShader(int shader)
{
	if (shader >= (int)shaders.size())
		return;
	glUseProgram(shaders.at(shader)->m_programID);
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni1i(char *name, int val)
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform1i(glGetUniformLocation(shaders.at(i)->m_programID, name), val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni2i(char *name, int val1, int val2)
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform2i(glGetUniformLocation(shaders.at(i)->m_programID, name), val1, val2);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni1f(char *name, float val)
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform1f(glGetUniformLocation(shaders.at(i)->m_programID, name), val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni2f(char *name, float val1, float val2)
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform2f(glGetUniformLocation(shaders.at(i)->m_programID, name), val1, val2);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni4f(char *name, float val1, float val2, float val3, float val4)
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform4f(glGetUniformLocation(shaders.at(i)->m_programID, name), val1, val2, val3, val4);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni2fv(char *name, float val[2])
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform2fv(glGetUniformLocation(shaders.at(i)->m_programID, name), 1, val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni3fv(char *name, float val[3])
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniform3fv(glGetUniformLocation(shaders.at(i)->m_programID, name), 1, val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUniMat3fv(char *name, float val[9])
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniformMatrix3fv(glGetUniformLocation(shaders.at(i)->m_programID, name), 1, GL_FALSE, val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUniMat4fv(char *name, float val[16])
{
	for (int i = 0; i < (int)shaders.size(); i++)
	{
		glUseProgram(shaders.at(i)->m_programID);
		glUniformMatrix4fv(glGetUniformLocation(shaders.at(i)->m_programID, name), 1, GL_FALSE, val);

	}
}
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------





//--------------------------------------------------------
//--------------------------------------------------------
// Stamp Class Def
//--------------------------------------------------------
//--------------------------------------------------------
Stamp::Stamp(void)
{
	m_stampName		= "";
	m_isTexStamp	= false;
	m_texture		= 0;
	m_shader		= NULL;

	initShader		= NULL;
}

//--------------------------------------------------------
Stamp::~Stamp(void)
{
	if (!m_isTexStamp)
		RE_DELETE(m_shader);
}

//--------------------------------------------------------
bool
Stamp::CreateFuncStamp(string stampName, string vertPath, string fragPath)
{
	m_stampName = stampName;

	m_isTexStamp = false;

	m_shader = new ShaderProg(vertPath, "", fragPath);
	glBindAttribLocation(m_shader->m_programID, 0, "vert_Position");

	if (!m_shader->CompileAndLink())
		return false;

	// Set constant uniforms
	glUseProgram(m_shader->m_programID);
	glUniform1i(glGetUniformLocation(m_shader->m_programID, "in_heightmap"), 0);

	return true;
}

//--------------------------------------------------------
bool
Stamp::CreateTexStamp(ShaderProg *shader, string stampName, string textureName)
{
	m_shader = shader;
	m_stampName = stampName;
	m_isTexStamp = true;
	return (LoadPNG(&m_texture, textureName, true));
}

//--------------------------------------------------------
string
Stamp::GetStampName(void)
{
	return (m_stampName);
}

//--------------------------------------------------------
bool
Stamp::IsTexStamp(void)
{
	return (m_isTexStamp);
}

//--------------------------------------------------------
void
Stamp::BindTexture(void)
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

//--------------------------------------------------------
GLuint
Stamp::GetShaderID(void)
{
	return (m_shader->m_programID);
}
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------





//--------------------------------------------------------
//--------------------------------------------------------
// Stamp Manager Class Def
//--------------------------------------------------------
//--------------------------------------------------------
StampManager::StampManager(void)
{
	m_error = true;

	// Create the texture stamp shader
	m_shTexStamp = new ShaderProg("shaders/tex_stamps.vert", "", "shaders/tex_stamps.frag");
	
	glBindAttribLocation(m_shTexStamp->m_programID, 0, "vert_Position");

	// Compile and check for errors
	m_error &= !m_shTexStamp->CompileAndLink();

	// Set uniform values
	glUseProgram(m_shTexStamp->m_programID);
	glUniform1i(glGetUniformLocation(m_shTexStamp->m_programID, "in_heightmap"), 0);
	glUniform1i(glGetUniformLocation(m_shTexStamp->m_programID, "in_stampmap"), 1);

	// Gaussian function stamp
	if (!AddFuncStamp("Gaussian", "shaders/gaussian.vert", "shaders/gaussian.frag"))
		return;

	// Testing image stamp
	if (!AddTexStamp("%", "images/texstamps/%.png"))
		return;

	// Footprint stamp
	if (!AddTexStamp("Footprint", "images/texstamps/Footprint.png"))
		return;

	// Smiley stamp 1
	if (!AddTexStamp("Smiley", "images/texstamps/Smiley.png"))
		return;
	
	// Smiley stamp 2
	if (!AddTexStamp("Smiley2", "images/texstamps/Smiley2.png"))
		return;

	// Pedobear
	if (!AddTexStamp("Pedobear", "images/texstamps/Pedobear.png"))
		return;

	// Mess
	if (!AddTexStamp("Mess", "images/texstamps/Mess.png"))
		return;

	m_error = false;
}

//--------------------------------------------------------
StampManager::~StampManager(void)
{
	RE_DELETE(m_shTexStamp);
	for (int i = 0; i < (int)stampCollection.size(); i++)
		delete stampCollection.at(i);
}

//--------------------------------------------------------
bool
StampManager::AddTexStamp(string stampName, string textureName)
{
	Stamp* newStamp = new Stamp();
	if (!newStamp->CreateTexStamp(m_shTexStamp, stampName, textureName))
		return false;

	return (FinaliseStamp(newStamp));
}

//--------------------------------------------------------
bool
StampManager::AddFuncStamp(string stampName, string vertPath, string fragPath)
{
	Stamp* newStamp = new Stamp();
	if (!newStamp->CreateFuncStamp(stampName, vertPath, fragPath))
		return false;

	newStamp->initShader = &setupGaussian;

	return (FinaliseStamp(newStamp));
}

//--------------------------------------------------------
bool
StampManager::FinaliseStamp(Stamp* newStamp)
{
	stampCollection.push_back(newStamp);
	stampIndexMap[newStamp->GetStampName()] = STAMPCOUNT++;

	return true;
}

//--------------------------------------------------------
Stamp*
StampManager::GetStamp(int stampIndex)
{
	return (stampCollection.at(stampIndex));
}

//--------------------------------------------------------
string
StampManager::GetStampName(int stampIndex)
{
	return (stampCollection.at(stampIndex)->GetStampName());
}

//--------------------------------------------------------
int
StampManager::GetStampIndex(string stampName)
{
	return (stampIndexMap[stampName]);
}

//--------------------------------------------------------
void
setupGaussian(Stamp* stamp, vector2 clickPos, float scale, float intensity)
{
	const float epsilon = 0.000001f;

	float falloff = - log(epsilon / fabsf(intensity)) / (scale * scale);
	glUniform1f(glGetUniformLocation(stamp->GetShaderID(), "falloff"), falloff);
}
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------
