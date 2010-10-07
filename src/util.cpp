
#include "regl3.h"
#include <SDL/SDL_image.h>
#include "util.h"
#include <png.h>

//--------------------------------------------------------
bool
SavePNG(char* filename, GLubyte* data, int bitdepth, int components, int w, int h, bool flip){
	int color_type;
	png_byte** row_pointers = (png_byte**)malloc(sizeof(png_byte*) * h);

	// Setup rows
	if (!flip)
		for (int i = 0; i < h; i++)
			row_pointers[i] = (png_byte*) (data + w * i * bitdepth * components);
	else
		for (int i = 0; i < h; i++)
			row_pointers[h-1-i] = (png_byte*) (data + w * i * bitdepth * components);

	// open file
	FILE* fp = fopen(filename, "wb");
	if (!fp){
		fprintf(stderr, "Could not open PNG file %s for saving.\n", filename);
		return false;
	}

	// create structures for hold png data and info
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr){
		fprintf(stderr,"Could not create png_ptr when writing %s\n", filename);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fprintf(stderr, "Could not create info_ptr when writing %s\n", filename);
		return false;
	}

	// Set strange jump location
	if (setjmp(png_jmpbuf(png_ptr))){
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		fprintf(stderr, "Could not set jump when writing %s\n", filename);
		return false;
	}

	png_init_io(png_ptr, fp);

	switch(components){
		case 1:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
		case 2:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;
		case 3:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case 4:
			color_type = PNG_COLOR_TYPE_RGBA;
			break;
		default:
			fprintf(stderr, "Unsupported number of components %d to write file %s\n", components,
				filename);
			return false;
	}

	if (bitdepth != 1 && bitdepth != 2){
		fprintf(stderr, "Unsupported bitdepth, should be 1 or 2, when writing %s\n", filename);
		return false;
	}
	
	// Set attributes of info struct
	png_set_IHDR(png_ptr, info_ptr, w, h,
		     bitdepth*8, color_type, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Write info to file
	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr))){
		fprintf(stderr, "failed to write info for %s\n", filename);
		return false;
	}
	
	png_write_image(png_ptr, row_pointers);
	if (setjmp(png_jmpbuf(png_ptr))){
		fprintf(stderr, "failed to write data for %s\n", filename);
		return false;
	}

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	//for (int i = 0; i < height; i++)
	//	free(row_pointers[i]);
	free(row_pointers);

	fclose(fp);


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
				GLenum FBOErr;

				FBOErr = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
				PrintFBOErr(FBOErr);

				FBOErr = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
				PrintFBOErr(FBOErr);

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

void
PrintFBOErr(GLenum err){
	printf("\n");
	switch (err){
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