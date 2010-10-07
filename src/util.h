
#ifndef _UTIL_H_
#define _UTIL_H_

struct TexData{
	GLuint 	heightmap;
	GLuint	normalmap;
	GLuint	tangentmap;
};


bool SavePNG	(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip=false);
bool LoadPNG	(GLuint *tex, string filename, bool flip=false);
bool CheckError	(string);
void PrintFBOErr(GLenum);

#endif
