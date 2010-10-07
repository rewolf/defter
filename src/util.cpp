
#include "regl3.h"
#include <SDL/SDL_image.h>
#include "util.h"
#include "FreeImage.h"

//--------------------------------------------------------
bool
SavePNG(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip){

	FIBITMAP* 			image;
	FREE_IMAGE_TYPE 	type;	// needed in case of 16-bit components

	if (bitdepth == 8) {  // 8-bits per component
		image = FreeImage_Allocate(w, h, components * bitdepth);
	} 
	else if (bitdepth == 16){
		switch(components){
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
	else{
		fprintf(stderr, "Invalid bitdepth for %s.  Must be 16 or 32.\n",filename);
		return false;
	}

	if (!image){
		fprintf(stderr, "Failed to allocate bitmap space for %s\n", filename);
		return false;
	}

	BYTE* bits = (BYTE*) FreeImage_GetBits(image);
	memcpy(bits, data, bitdepth/8 * components * w * h);

	if (flip)
		FreeImage_FlipVertical(image);
	
	FreeImage_Save(FIF_PNG, image, filename, PNG_Z_BEST_SPEED);

	FreeImage_Unload(image);
	return true;
}

//--------------------------------------------------------
bool 
LoadTexturePNG(GLuint* tex, int* width, int* height, string filename){
	SDL_Surface*  surface;

	surface = IMG_Load(filename.c_str());
	if (surface == NULL){
		fprintf(stderr, "\t\tError\n\tCould not load PNG: %s\n\t%s\n", filename.c_str(), IMG_GetError());
		return false;
	}

	*width = surface->w;
	*height= surface->h;

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Loading PNG texture, setting parameters")){
		printf("\tFile: %s\n", filename.c_str());
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	SDL_FreeSurface(surface);
	return true;
}

//--------------------------------------------------------
bool 
CheckError(string text){
	GLuint err = glGetError();

	if (err!=GL_NO_ERROR){
		fprintf(stderr, "\t\tError\n\tOpenGL Error: ");
		switch(err){
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
				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "Out of Memory");
				break;
			case 0:
				fprintf(stderr, "Unknown");
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
