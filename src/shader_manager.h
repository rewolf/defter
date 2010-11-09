/*****************************************************************************
 * Header: shader_manager
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _SHADER_MANAGER_H_
#define _SHADER_MANAGER_H_

extern const int SHADERNUM;

class ShaderManager
{
public:
	ShaderManager(void);
	~ShaderManager(void);
	bool AddShader(string vert, string geom, string frag);

	//Update methods
	void BindAttrib(char *name, int val);
	void UpdateUni1i(char *name, int val);
	void UpdateUni2i(char *name, int val1, int val2);
	void UpdateUni1f(char *name, float val);
	void UpdateUni2f(char *name, float val1, float val2);
	void UpdateUni4f(char *name, float val1, float val2, float val3, float val4);
	void UpdateUni2fv(char *name, float val[2]);
	void UpdateUni3fv(char *name, float val[3]);
	void UpdateUniMat3fv(char *name, float val[9]);
	void UpdateUniMat4fv(char *name, float val[16]);
	
	int CompileAndLink(void);
	void SetActiveShader(int shader);

public:
	ShaderProg *shaders[2];
	int curIndex;
};

#endif
