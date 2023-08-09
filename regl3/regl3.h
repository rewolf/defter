/*****************************************************************************
 * Header: regl3
 *
 * Copyright � 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _REGL3_H
#define _REGL3_H

// SDL and OpenGL
#include "SDL2/SDL.h"
#ifdef _WIN32
#	define GLEW_STATIC 1
#	include "GL/GLEW.H"
#else
#	define GL_GLEXT_PROTOTYPES
#	include "SDL2/SDL_opengl.h"
#endif

// Standard library headers
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <list>
using namespace std;

// Headers
#include "re_input.h"
#include "re_timer.h"

enum reDemo {RE_DEMO_NONE, RE_DEMO_PLAY, RE_DEMO_RECORD};

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
struct AppConfig
{
	AppConfig()
	{
		fullscreen 	= false;
		VSync		= false;
		resizable	= false;
		winWidth 	= 800;
		winHeight	= 600;
		fsaa		= 0; 		// Fullscreen Antialias (# of samples per pixel)
		sleepTime 	= .01f;		// Delay in seconds
		gl_major	= 2;
		gl_minor	= 1;
		title		= "SDL1.3 & OpenGL";
		demo		= RE_DEMO_NONE;
	}
	void Print()
	{
		printf("fullscreen: %d\n", fullscreen);
		printf("VSync: %d\n", VSync);
		printf("resizable: %d\n", resizable);
		printf("winWidth: %d\n", winWidth);
		printf("winHeight: %d\n", winHeight);
		printf("sleepTime: %.3f\n", sleepTime);
		printf("gl_version: %d.%d\n", gl_major, gl_minor);
		printf("title: %s\n", title.c_str());
		if (demo!=RE_DEMO_NONE)
			printf("demo: %s\n", demo==RE_DEMO_RECORD ? "record" : "play");

	}
	bool 	fullscreen;
	bool 	VSync;
	bool	resizable;
	u_int 	winWidth;
	u_int 	winHeight;
	u_int	fsaa;
	u_int	gl_major;
	u_int	gl_minor;
	float	sleepTime;
	string	title;
	reDemo	demo;
};


/*
 * DemoFrame
 * Contains all events that occurred this frame
 */
struct DemoFrame{
	list<SDL_Event> 	events;
	float				dt;
};


/******************************************************************************
 * reGL3App - The Main app class, that can be instantiated
 * or derived to create an application using SDL and GL3
 ******************************************************************************/
class reGL3App
{
public:
	reGL3App(AppConfig conf=AppConfig());
	virtual ~reGL3App();

	/* Start method calls InitGL, InitSDL and if they succeed, it then calls Run */
	bool		Start();
	void		Quit();

	virtual void	ProcessInput	(float dt);
	virtual	void	Render			(float dt);
	virtual	void	Logic			(float dt);

	list<DemoFrame>	GetDemo			();

protected:
	/* Override InitGL with your GL setup and projection setups etc. */
	virtual bool		InitGL();
private:
	bool				InitSDL();
	void				Run();
	void				WinProc();
	void				SaveDemo();
	bool				LoadDemo();

public:
	AppConfig		m_config;
	SDL_GLContext	m_context;
	SDL_Window*		m_pWindow;
	Input			m_input;
	list<DemoFrame>	m_demoFrames;
	DemoFrame		m_curFrame;

	bool 			m_isRunning;

	float			m_currentFPS;
};

#endif
