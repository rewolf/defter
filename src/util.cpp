
#include "regl3.h"
#include <SDL/SDL_image.h>
#include "util.h"


//--------------------------------------------------------
bool 
LoadTexturePNG(GLuint* tex, int* width, int* height, string filename){
	SDL_Surface*  surface;

	surface = IMG_Load(filename.c_str());
	if (surface == NULL){
		fprintf(stderr, "Could not load PNG %s: %s\n", filename.c_str(), IMG_GetError());
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
		printf("Could not load texture: %s\n", filename.c_str());
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
		fprintf(stderr, "OpenGL Error: ");
		switch(err){
			case GL_INVALID_ENUM:
				fprintf(stderr, "Invalid Enum  ");
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr, "Invalid Value  ");
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr, "Invalid Operation  ");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				fprintf(stderr, "Invalid FBO operation  ");
				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "Out of Memory  ");
				break;
		}
		printf ("[%s]\n", text.c_str());
		return false;
	}
	return true;
}
