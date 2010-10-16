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


bool SavePNG	(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip = false);
bool LoadPNG	(GLuint *tex, string filename, bool flip=false, bool scale=false);
bool CheckError	(string);
void PrintFBOErr(GLenum);

#endif
