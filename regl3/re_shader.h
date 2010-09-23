
#ifndef _RE_SHADER_PROG_
#define _RE_SHADER_PROG_

class ShaderProg{
public:

	ShaderProg(string vertPath, string geomPath, string fragPath);
	~ShaderProg();

	int 	CompileAndLink	(void);

protected:
	int		SetupShader		(GLuint id);
	string 	LoadSource		(string path);
	void	PrintShaderLog	(GLuint shaderID);
	void	PrintProgramLog	(void);

public:
	GLuint 		m_programID;
	GLuint		m_vertShID;
	GLuint		m_geomShID;
	GLuint		m_fragShID;
};

#endif
