
#include <string>
#include <fstream>
#include <sstream>
#include "SDL/SDL.h"
#ifdef _WIN32
#	define GLEW_STATIC 1
#	include "GL/GLEW.H"
#else
#	define GL_GLEXT_PROTOTYPES
#	include "SDL/SDL_opengl.h"
#endif

using namespace std;
#include "re_shader.h"


//--------------------------------------------------------
ShaderProg::ShaderProg(string vertPath, string geomPath, string fragPath){
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
ShaderProg::~ShaderProg(){
	glDeleteProgram(m_programID);
}

//--------------------------------------------------------
int 
ShaderProg::CompileAndLink(){
	GLint res;

	if (!SetupShader(m_vertShID))
		return 0;
	if (!SetupShader(m_fragShID))
		return 0;
	if (!SetupShader(m_geomShID))
		return 0;
	
	// Link shaders together
	glLinkProgram(m_programID);

	glGetProgramiv(m_programID, GL_LINK_STATUS, &res);
	if (res==GL_FALSE){
		fprintf(stderr, "Could not link shaders.\n");
		PrintProgramLog();
		return 0;
	}
	return 1;
}

//--------------------------------------------------------
int
ShaderProg::SetupShader(GLuint id){
	//If shader is disabled then return
	if (id == -1)
		return 1;

	GLint res;

	// Compile
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE){
		fprintf(stderr, "Could not compile shader.\n");
		PrintShaderLog(id);
		return 0;
	}

	// Attach and set to delete on detachment
	glAttachShader(m_programID, id);
	glDeleteShader(id);

	return 1;
}


//--------------------------------------------------------
string
ShaderProg::LoadSource(string path){
	ifstream fin(path.c_str());             	

	if (!fin){
		fprintf(stderr,"ERROR: Could not find file %s containing shader source.\n",
			path.c_str());
		return ""; 
	}   

	stringstream ret;

	while(fin){
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
ShaderProg::PrintShaderLog(GLuint shaderID){
	char slog[1024];
	int len;
	printf("Shader errors: \n");
	glGetShaderInfoLog(shaderID,1024,&len,slog);
	printf("%s\n",slog);
}

//--------------------------------------------------------
void
ShaderProg::PrintProgramLog  (){
	char slog[1024];
	printf("Shader Program errors: \n");
	glGetProgramInfoLog(m_programID,1024,NULL,slog);
	printf("%s\n",slog);
}





