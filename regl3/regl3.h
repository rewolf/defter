/*
    Copyright (C) 2010 Andrew Flower <andrew.flower@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _REGL3_H
#define _REGL3_H

// SDL and OpenGL
#include "SDL/SDL.h"
#ifdef _WIN32
#	define GLEW_STATIC 1
#	include "GL/GLEW.H"
#else
#	define GL_GLEXT_PROTOTYPES
#	include "SDL/SDL_opengl.h"
#endif

// Standard library headers
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;

// Headers
#include "re_input.h"
#include "re_timer.h"


/******************************************************************************
 * MACROS & DEFINES
 ******************************************************************************/
#define	RE_DELETE(x)			{		if (x!=NULL) delete x; x=NULL;			}
#define RE_DELETE_ARR(x)		{		if (x!=NULL) delete[] x; x=NULL;		}

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/
typedef unsigned int u_int;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

/*
 * AppConfig
 * Contains settings to pass to AppMain for SDL, OpenGL setup.
 */
struct AppConfig{
	AppConfig(){
		fullscreen 	= false;
		VSync		= false;
		resizable	= false;
		lockLogicTimeStep = false;
		winWidth 	= 800;
		winHeight	= 600;
		fsaa		= 0; 		// Fullscreen Antialias (# of samples per pixel)
		sleepTime 	= .01f;		// Delay in seconds
		timeStep	= .02f;		// This applies if the logic time-step is locked.
		gl_major	= 2;
		gl_minor	= 1;
		title		= "SDL1.3 & OpenGL";
	}
	void Print(){
		printf("fullscreen: %d\n", fullscreen);
		printf("VSync: %d\n", VSync);
		printf("resizable: %d\n", resizable);
		printf("winWidth: %d\n", winWidth);
		printf("winHeight: %d\n", winHeight);
		printf("sleepTime: %.3f\n", sleepTime);
		printf("gl_version: %d.%d\n", gl_major, gl_minor);
		printf("title: %s\n", title.c_str());
	}
	bool 	fullscreen;
	bool 	VSync;
	bool	resizable;
	bool	lockLogicTimeStep;
	u_int 	winWidth;
	u_int 	winHeight;
	u_int	fsaa;
	u_int	gl_major;
	u_int	gl_minor;
	float	sleepTime;
	float	timeStep;
	string	title;
};

/******************************************************************************
 * reGL3App - The Main app class, that can be instantiated
 * or derived to create an application using SDL and GL3
 ******************************************************************************/
class reGL3App{
public:
	reGL3App(AppConfig conf=AppConfig());
	virtual ~reGL3App();

	/* Start method calls InitGL, InitSDL and if they succeed, it then calls Run */
	bool		Start();
	void		Quit();

	virtual void	ProcessInput	(float dt);
	virtual	void	Render			(float dt);
	virtual	void	Logic			(float dt);

protected:
	/* Override InitGL with your GL setup and projection setups etc. */
	virtual bool		InitGL();
private:
	bool				InitSDL();
	void				Run();
	void				WinProc();

public:
	AppConfig		m_config;
	SDL_GLContext	m_context;
	SDL_Window*		m_pWindow;
	Input			m_input;

	bool 			m_isRunning;

	float			m_currentFPS;
};


#endif
