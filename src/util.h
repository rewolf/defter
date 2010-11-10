/*****************************************************************************
 * Header: util
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

class Splash;
class ShaderManager;
class Stamp;
class StampManager;

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

StampManager* GetStampMan	(void);



class Splash
{
public:
	Splash					(void);
	~Splash					(void);

	bool HasError			(void);
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

	bool AddShader			(string vert, string geom, string frag, int *index);
	void BindAttrib			(char *name, int val);
	bool CompileAndLink		(void);
	void SetActiveShader	(int shader);

	//Update methods
	void UpdateUni1i		(char *name, int val);
	void UpdateUni2i		(char *name, int val1, int val2);
	void UpdateUni1f		(char *name, float val);
	void UpdateUni2f		(char *name, float val1, float val2);
	void UpdateUni4f		(char *name, float val1, float val2, float val3, float val4);
	void UpdateUni2fv		(char *name, float val[2]);
	void UpdateUni3fv		(char *name, float val[3]);
	void UpdateUniMat3fv	(char *name, float val[9]);
	void UpdateUniMat4fv	(char *name, float val[16]);
	
public:
	ShaderProg*			shaders[SHADERNUM];
	int					curIndex;
};



class Stamp
{
public:
	Stamp					(void);
	~Stamp					(void);

	bool CreateFuncStamp	(string stampName, string vertPath, string fragPath);
	bool CreateTexStamp		(ShaderProg *shader, string stampName, string textureName);

	string GetStampName		(void);
	bool IsTexStamp			(void);
	void BindTexture		(void);
	GLuint GetShaderID		(void);

	void(*initShader)		(Stamp* stamp, vector2 clickPos, float scale, float intensity);

private:
	string				m_stampName;
	bool				m_isTexStamp;
	GLuint				m_texture;
	ShaderProg*			m_shader;
};



class StampManager
{
public:
	StampManager			(void);
	~StampManager			(void);

	bool HasError			(void);
	Stamp* GetStamp			(int stampIndex);
	string GetStampName		(int stampIndex);
	int GetStampIndex		(string stampName);

private:
	bool AddTexStamp		(string stampName, string textureName);
	bool AddFuncStamp		(string stampName, string vertPath, string fragPath);
	bool FinaliseStamp		(Stamp* newStamp);

	bool				m_error;
	ShaderProg*			m_shTexStamp;
	vector<Stamp*>		stampCollection;
	map<string, int>	stampIndexMap;
};

// Functional Stamp setup callbacks
void setupGaussian(Stamp* stamp, vector2 clickPos, float scale, float intensity);

#endif
