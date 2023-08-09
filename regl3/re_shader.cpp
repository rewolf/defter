/*****************************************************************************
 * re_shader: Creates shader programs and handles GLSL shaders
 *
 * Copyright � 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include <string>
#include <fstream>
#include <sstream>
#include "SDL2/SDL.h"
#ifdef _WIN32
#	define GLEW_STATIC 1
#	include "GL/GLEW.H"
#else
#	define GL_GLEXT_PROTOTYPES
#	include "SDL2/SDL_opengl.h"
#endif

using namespace std;
#include "re_shader.h"


//--------------------------------------------------------
ShaderProg::ShaderProg(string vertPath, string geomPath, string fragPath)
{
	string vertSource, geomSource, fragSource;
	const char *vsrc, *gsrc, *fsrc;

	m_programID = glCreateProgram();

	m_vertShID = glCreateShader(GL_VERTEX_SHADER);
	m_fragShID = glCreateShader(GL_FRAGMENT_SHADER);
	if (geomPath != "")
		m_geomShID = glCreateShader(GL_GEOMETRY_SHADER);
	else
		m_geomShID = -1;

	vertSource = LoadSource(vertPath);
	fragSource = LoadSource(fragPath);
	if (geomPath != "")
		geomSource = LoadSource(geomPath);

	vsrc = vertSource.c_str();
	gsrc = geomSource.c_str();
	fsrc = fragSource.c_str();

	glShaderSource(m_vertShID, 1, (const char**)&vsrc, NULL);
	glShaderSource(m_fragShID, 1, &fsrc, NULL);
	if (geomPath != "")
		glShaderSource(m_geomShID, 1, &gsrc, NULL);
}

//--------------------------------------------------------
ShaderProg::~ShaderProg()
{
	glDeleteProgram(m_programID);
}

//--------------------------------------------------------
bool 
ShaderProg::CompileAndLink()
{
	GLint res;

	if (!SetupShader(m_vertShID))
		return false;
	if (!SetupShader(m_fragShID))
		return false;
	if (!SetupShader(m_geomShID))
		return false;
	
	// Link shaders together
	glLinkProgram(m_programID);

	glGetProgramiv(m_programID, GL_LINK_STATUS, &res);
	if (res == GL_FALSE)
	{
		fprintf(stderr, "Could not link shaders.\n");
		PrintProgramLog();
		return false;
	}
	return true;
}

//--------------------------------------------------------
bool
ShaderProg::SetupShader(GLuint id)
{
	//If shader is disabled then return
	if (id == -1)
		return true;

	GLint res;

	// Compile
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE)
	{
		fprintf(stderr, "Could not compile shader.\n");
		PrintShaderLog(id);
		return false;
	}

	// Attach and set to delete on detachment
	glAttachShader(m_programID, id);
	glDeleteShader(id);

	return true;
}


//--------------------------------------------------------
string
ShaderProg::LoadSource(string path)
{
	ifstream fin(path.c_str());             	

	if (!fin)
	{
		fprintf(stderr,"ERROR: Could not find file %s containing shader source.\n",
			path.c_str());
		return ""; 
	}   

	stringstream ret;

	while(fin)
	{
		string tmp;
		getline(fin, tmp);
		
		//If an include file is specified then load that file
		if (tmp.compare(0, 13, "#IncludeFile:") == 0)
			ret << LoadSource(path.substr(0, path.find_last_of("/\\")) + "/" + tmp.substr(13)) << endl;
		else
			ret << tmp << endl;
	}   

	fin.close();
	return ret.str();
}

//-------------------------------------------------------- 
void
ShaderProg::PrintShaderLog(GLuint shaderID)
{
	char slog[1024];
	int len;
	printf("Shader errors: \n");
	glGetShaderInfoLog(shaderID, 1024, &len, slog);
	printf("%s\n", slog);
}

//--------------------------------------------------------
void
ShaderProg::PrintProgramLog()
{
	char slog[1024];
	printf("Shader Program errors: \n");
	glGetProgramInfoLog(m_programID, 1024, NULL, slog);
	printf("%s\n", slog);
}
