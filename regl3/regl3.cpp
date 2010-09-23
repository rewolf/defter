
#include "regl3.h"

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
/******************************************************************************
 * reGL3App Constructor
 * Just stores the config struct
 ******************************************************************************/
reGL3App::reGL3App(AppConfig conf){
	m_config  = conf;
	m_isRunning = true;
}

/******************************************************************************
 * reGL3App Destructor
 * Basically just kills SDL at the moment.
 ******************************************************************************/
reGL3App::~reGL3App(){
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_pWindow);
	SDL_Quit();
}


/******************************************************************************
 * Start
 * Calls InitSDL and then InitGL. If both return successfully, then it calls Run.
 * This will block until the run loop is stopped by calling Quit.
 ******************************************************************************/
bool 
reGL3App::Start(){
	printf("-----------------------------------------\n");
	if (!InitSDL())
		return false;
	// Display Driver & OpenGL info
	printf("Vendor      : %s\n", glGetString(GL_VENDOR));
	printf("Renderer    : %s\n", glGetString(GL_RENDERER));
	printf("GL Driver   : %s\n", glGetString(GL_VERSION));
	printf("-----------------------------------------\n");
	if (!InitGL())
		return false;
	Run();
	return true;
}

/******************************************************************************
 * Quit
 * Can be called at any time to stop the run loop.
 ******************************************************************************/
void
reGL3App::Quit(){
	m_isRunning = false;
}


/******************************************************************************
 * Initializes all settings for SDL, using those specified from m_config
 ******************************************************************************/
bool
reGL3App::InitSDL(){
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	// Flags for SDL initialization
	u_int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	if (m_config.resizable)
		flags |= SDL_WINDOW_RESIZABLE;

	// Enable fullscreen
	if (m_config.fullscreen)
		flags |= SDL_FULLSCREEN;

	// Set attributes for GL video buffers
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// Desired OpenGL context version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, m_config.gl_major);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, m_config.gl_minor);

	// Multisampling
	if (m_config.fsaa > 0){
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, m_config.fsaa);
	}
	else{
		// HW acceleration
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	}

	// Create the window
	m_pWindow = SDL_CreateWindow(m_config.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		m_config.winWidth, m_config.winHeight, flags);
	if (!m_pWindow){
		fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
		return false;
	}

	// Create the GL context for the window
	m_context = SDL_GL_CreateContext(m_pWindow);
	if (!m_context){
		fprintf(stderr, "Could not create the OpenGL context for the window: %s\n", SDL_GetError());
		return false;
	}

	int val;
	SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &val);
	if (!val)
		printf("No hardware support for SDL\n");


	// Enable VSync
	SDL_GL_SetSwapInterval(m_config.VSync);
	return true;
}

/******************************************************************************
 * InitGL
 * Initializes OpenGL stuff
 ******************************************************************************/
bool
reGL3App::InitGL(){
	glClearColor(.0f, .0f, .0f, .0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, m_config.winWidth, m_config.winHeight);

	return true;
}

/******************************************************************************
 * Logic
 * Performs the game logics, which may include movement of objects, AI, sound 
 * management etc.
 ******************************************************************************/
void
reGL3App::Logic(float dt){
}

/******************************************************************************
 * ProcessInput
 * A function whose purpose is to handle any input updates
 ******************************************************************************/
void
reGL3App::ProcessInput(float dt){
	if (m_input.WasKeyPressed(SDLK_ESCAPE))
		Quit();
}


/******************************************************************************
 * Render
 * Performs the drawing to GL context. The default is a blank screen
 ******************************************************************************/
void
reGL3App::Render(float dt){
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(m_pWindow);
}

/******************************************************************************
 * Run
 * The function that iterates the main game-loop
 * It will exit only when m_isRunning is false, which can be set via Quit().
 ******************************************************************************/
void
reGL3App::Run(){
	reTimer timer;
	timer.start();
	// Start the main loop
	// If we're locking the logic timestep, run a different loop, rather than checking every iteration.
	if (m_config.lockLogicTimeStep)
	{
		while(m_isRunning){
			static float rem = .0f;
			float elapsed = timer.getElapsed();

			// Poll the SDL input buffer
			WinProc();

			// Process input state
			ProcessInput(m_config.timeStep);


			// Calculate the number of iterations of the fixed timestep
			rem += elapsed;
			int nIters = int(rem/m_config.timeStep);
			rem -= nIters * m_config.timeStep;

			// Iterate the logic loop with the fixed timestep
			for (int i = 0; i < nIters; i++){
				Logic(m_config.timeStep);
			}

			// Call the render method
			Render(elapsed);

			SDL_Delay(int(m_config.sleepTime*1000));
		}
	}
	// ELSE, a normal game loop
	else
	{
		while(m_isRunning){
			float elapsed = timer.getElapsed();

			// Poll the SDL input buffer
			WinProc();

			// Process the input state
			ProcessInput(elapsed);
			// Perform Game Logic
			Logic(elapsed);
			// Render the scene
			Render(elapsed);

			SDL_Delay(int(m_config.sleepTime*1000));
		}
	}
	printf("Average fps: %.2f\n", timer.getFPS());
}

/******************************************************************************
 * WinProc
 * This is the function that polls SDL for input events
 * and then passes them to the Input handler
 ******************************************************************************/
void
reGL3App::WinProc(){
	SDL_Event evt;
	while (SDL_PollEvent(&evt)){
		switch(evt.type){
			case SDL_KEYDOWN:
				// Don't really understand what SDL guys are doing with their key
				// codes, but ill just do this hack to remove the bitflag so it
				// can fit in the array
				m_input.PressKey(evt.key.keysym.sym);
				break;
			case SDL_KEYUP:
				m_input.ReleaseKey(evt.key.keysym.sym);
				break;
			case SDL_QUIT:
				Quit();
				break;
			case SDL_MOUSEMOTION:
				m_input.MoveMouse(evt.motion);
				break;
			case SDL_MOUSEWHEEL:
				// The below things seemed to have stopped working, moved
				// wheel handling to BUTTONDOWN event.
				if (evt.wheel.y > 0)
					m_input.WheelUp();
				else
					m_input.WheelDown();
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (evt.button.button == SDL_BUTTON_WHEELUP)
					m_input.WheelUp();
				else if (evt.button.button == SDL_BUTTON_WHEELDOWN)
					m_input.WheelDown();
				else
					m_input.PressButton(evt.button.button);
				break;
			case SDL_MOUSEBUTTONUP:
				m_input.ReleaseButton(evt.button.button);
				break;
			case SDL_VIDEORESIZE:
				return;
		}
	}
}




