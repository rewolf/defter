/*****************************************************************************
 * util: Provides utility methods - image handling and error checking
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"
#include "util.h"
#include "FreeImage.h"

extern const int SCREEN_W;
extern const int SCREEN_H;
extern const float ASPRAT;

//--------------------------------------------------------
bool
SavePNG(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip)
{

	FIBITMAP* 			image;
	FREE_IMAGE_TYPE 	type;	// needed in case of 16-bit components
	BYTE*				bits;

	// 8-bits per component
	if (bitdepth == 8)
	{
		image = FreeImage_Allocate(w, h, components * bitdepth);
	} 
	else if (bitdepth == 16)
	{
		switch(components)
		{
			case 1:
				type = FIT_UINT16;
				break;
			case 3:
				type = FIT_RGB16;		// FreeImage expects  data in BGR format
				break;
			case 4:
				type = FIT_RGBA16;		// FreeImage expects  data in BGRA format
				break;
			default:
				fprintf(stderr, "No support for 2-component textures when saving %s\n", filename);
				return false;
		}
		image = FreeImage_AllocateT(type, w, h);
	}
	else
	{
		fprintf(stderr, "Invalid bitdepth for %s.  Must be 16 or 32.\n",filename);
		return false;
	}

	if (!image)
	{
		fprintf(stderr, "Failed to allocate bitmap space for %s\n", filename);
		return false;
	}

	bits = (BYTE*) FreeImage_GetBits(image);
	memcpy(bits, data, bitdepth/8 * components * w * h);

	if (flip)
		FreeImage_FlipVertical(image);
	
	FreeImage_Save(FIF_PNG, image, filename, PNG_Z_BEST_SPEED);

	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
bool 
LoadPNG(GLuint* tex, string filename, bool flip, bool scale)
{
	FIBITMAP*		image;
	BYTE*			bits;
	int				width;
	int				height;
	int				bitdepth;

	image = FreeImage_Load(FIF_PNG, filename.c_str(), 0);

	if (!image)
	{
		fprintf(stderr, "Error\n\tCould not load PNG: %s\n", filename.c_str());
		return false;
	}

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Loading PNG texture, setting parameters"))
	{
		fprintf(stderr, "\tFile: %s\n", filename.c_str());
		return false;
	}

	if (flip)
		FreeImage_FlipVertical(image);

	// Get the width & height of the image
	width	= FreeImage_GetWidth(image);
	height	= FreeImage_GetHeight(image);

	// Scale the image if told to do so
	if (scale)
	{
		// Scaling variables
		float targetScale, newW, newH;
		int offsetw, offseth;

		targetScale = max(float(SCREEN_W) / width, float(SCREEN_H) / height);

		// Calculate the new width & height based on the scale
		newW		= float(width) * targetScale;
		newH		= float(height) * targetScale;
		
		// Rescale the image to the new size
		image = FreeImage_Rescale(image, newW, newH, FILTER_CATMULLROM);

		// Calculate the offsets and set the new width & height
		offsetw		= (newW - SCREEN_W) / 2;
		offseth		= (newH - SCREEN_H) / 2;
		newW		= SCREEN_W;
		newH		= SCREEN_H;

		// Copy the subimage to crop the image
		image = FreeImage_Copy(image, offsetw, offseth, offsetw + newW, offseth + newH);

		// Retrieve the actual image width & height to prevent any errors
		width		= FreeImage_GetWidth(image);
		height		= FreeImage_GetHeight(image);
	}

	// Get the bit depth and retrieve the pixel data
	bits 	= (BYTE*) FreeImage_GetBits(image);
	bitdepth= FreeImage_GetBPP(image);

	// Choose based on bit depth
	switch(bitdepth)
	{
		case 32:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);
			break;
		case 24:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, bits);
			break;
		case 8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bits);
			break;
		default:
			fprintf(stderr, "Failed to load image %s of bitdepth %d\n", filename.c_str(), bitdepth);
			return false;
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	
	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
bool 
CheckError(string text)
{
	GLuint err = glGetError();

	if (err!=GL_NO_ERROR){
		fprintf(stderr, "Error\n\tOpenGL Error: ");
		switch(err)
		{
			case GL_INVALID_ENUM:
				fprintf(stderr, "Invalid Enum");
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr, "Invalid Value");
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr, "Invalid Operation");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				fprintf(stderr, "Invalid FBO operation");
				GLenum FBOErr;

				FBOErr = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
				PrintFBOErr(FBOErr);
				
				FBOErr = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
				PrintFBOErr(FBOErr);

				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "Out of Memory");
				break;
		}

		// Print out messages only if have one set
		printf("\n");
		if (text.length() != 0)
			printf ("\t[%s]\n", text.c_str());

		return false;
	}
	return true;
}

void
PrintFBOErr(GLenum err)
{
	printf("\n");
	switch (err)
	{
		case GL_FRAMEBUFFER_UNDEFINED:
			fprintf(stderr, "FB Undefined");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			fprintf(stderr, "FB Incomplete Attachment");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			fprintf(stderr, "FB Incomplete Missing Attachment");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			fprintf(stderr, "FB Incomplete Draw Buffer");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			fprintf(stderr, "FB Incomplete Read Buffer");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			fprintf(stderr, "FB Unsupported");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			fprintf(stderr, "FB Incomplete Multisample");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			fprintf(stderr, "FB Incomplete Layer Targets");
			break;
	}
}
