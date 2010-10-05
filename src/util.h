
#ifndef _UTIL_H_
#define _UTIL_H_

struct TexData{
	GLuint 	heightmap;
	GLuint	normalmap;
	GLuint	tangentmap;
};


bool LoadTexturePNG	(GLuint *tex, int* width, int* height, string filename);
bool CheckError(string);



#endif
