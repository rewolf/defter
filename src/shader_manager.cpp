/*****************************************************************************
 * shader_manager: Utility class for managing multiple shaders
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_shader.h"
#include "shader_manager.h"
#include "util.h"

extern const int SHADERNUM;

//--------------------------------------------------------
ShaderManager::ShaderManager(void)
{
	curIndex = 0;
}

//--------------------------------------------------------
ShaderManager::~ShaderManager(void)
{
	//Delete each shader object
	for (int i = 0; i < curIndex; i++)
		RE_DELETE(shaders[i]);

	//Delete shader array
	RE_DELETE_ARR(*shaders);
}

//--------------------------------------------------------
// Add a new set of shaders
//--------------------------------------------------------
bool
ShaderManager::AddShader(string vert, string geom, string frag)
{
	//If out of available shader spots then return an error
	if ((curIndex + 1) > SHADERNUM)
		return (false);

	//Create the new shader and increment the current index
	shaders[curIndex++] = new ShaderProg(vert, geom, frag);

	//Return successful
	return (true);
}

//--------------------------------------------------------
// Bind the specific variable to a location in the shaders
//--------------------------------------------------------
void
ShaderManager::BindAttrib(char *name, int val)
{
	for (int i = 0; i < curIndex; i++)
		glBindAttribLocation(shaders[i]->m_programID, val, name);
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni1i(char *name, int val)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform1i(glGetUniformLocation(shaders[i]->m_programID, name), val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni2i(char *name, int val1, int val2)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform2i(glGetUniformLocation(shaders[i]->m_programID, name), val1, val2);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni1f(char *name, float val)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform1f(glGetUniformLocation(shaders[i]->m_programID, name), val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni2f(char *name, float val1, float val2)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform2f(glGetUniformLocation(shaders[i]->m_programID, name), val1, val2);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni4f(char *name, float val1, float val2, float val3, float val4)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform4f(glGetUniformLocation(shaders[i]->m_programID, name), val1, val2, val3, val4);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni2fv(char *name, float val[2])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform2fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUni3fv(char *name, float val[3])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform3fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUniMat3fv(char *name, float val[9])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniformMatrix3fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, GL_FALSE, val);
	}
}

//--------------------------------------------------------
void
ShaderManager::UpdateUniMat4fv(char *name, float val[16])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniformMatrix4fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, GL_FALSE, val);

	}
}

//--------------------------------------------------------
// Compile and link the shaders
//--------------------------------------------------------
int
ShaderManager::CompileAndLink(void)
{
	for (int i = 0; i < curIndex; i++)
	{
		//Check for errors in compiling any shader
		if (!shaders[i]->CompileAndLink())
			return (0);
	}

	//Success
	return (1);
}

//--------------------------------------------------------
// Set the active shader to be used
//--------------------------------------------------------
void
ShaderManager::SetActiveShader(int shader)
{
	if (shader >= SHADERNUM)
		return;
	glUseProgram(shaders[shader]->m_programID);
}
