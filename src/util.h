/*****************************************************************************
 * Header: util
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

struct TexData
{
	GLuint 	heightmap;
	GLuint	pdmap;
};

bool SavePNG				(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip = false);
bool LoadPNG				(GLuint *tex, string filename, bool flip=false, bool scale=false);
bool CheckError				(string);
void PrintFBOErr			(GLenum);

void KillUtil				(void);
GLuint GetStandardVAO		(void);



class Splash
{
public:
	Splash					(void);
	~Splash					(void);

	bool GetErrorStatues	(void);
	void Render				(SDL_Window* window);

private:
	GLuint				m_splashmap;
	ShaderProg*			m_shSplash;
	bool				m_error;
};


class ShaderManager
{
public:
	ShaderManager			(void);
	~ShaderManager			(void);
	bool AddShader			(string vert, string geom, string frag);

	//Update methods
	void BindAttrib			(char *name, int val);
	void UpdateUni1i		(char *name, int val);
	void UpdateUni2i		(char *name, int val1, int val2);
	void UpdateUni1f		(char *name, float val);
	void UpdateUni2f		(char *name, float val1, float val2);
	void UpdateUni4f		(char *name, float val1, float val2, float val3, float val4);
	void UpdateUni2fv		(char *name, float val[2]);
	void UpdateUni3fv		(char *name, float val[3]);
	void UpdateUniMat3fv	(char *name, float val[9]);
	void UpdateUniMat4fv	(char *name, float val[16]);
	
	int CompileAndLink		(void);
	void SetActiveShader	(int shader);

public:
	ShaderProg*			shaders[2];
	int					curIndex;
};

#endif
