/*****************************************************************************
 * regl3: Main program to create and handle a OpenGL 3.x context
 *
 * Copyright � 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"

/******************************************************************************
 * reGL3App Constructor
 * Just stores the config struct
 ******************************************************************************/
reGL3App::reGL3App(AppConfig conf)
{
	m_config  = conf;
	m_isRunning = true;
}

/******************************************************************************
 * reGL3App Destructor
 * Basically just kills SDL at the moment.
 ******************************************************************************/
reGL3App::~reGL3App()
{
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
reGL3App::Start()
{
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
reGL3App::Quit()
{
	m_isRunning = false;
}


/******************************************************************************
 * Initializes all settings for SDL, using those specified from m_config
 ******************************************************************************/
bool
reGL3App::InitSDL()
{
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
		flags |= SDL_WINDOW_FULLSCREEN;

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
	else
	{
		// HW acceleration
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	}

	// Create the window
	m_pWindow = SDL_CreateWindow(m_config.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		m_config.winWidth, m_config.winHeight, flags);
	if (!m_pWindow)
	{
		fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
		return false;
	}

	// Create the GL context for the window
	m_context = SDL_GL_CreateContext(m_pWindow);
	if (!m_context)
	{
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
reGL3App::InitGL()
{
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
reGL3App::Logic(float dt)
{

}

/******************************************************************************
 * ProcessInput
 * A function whose purpose is to handle any input updates
 ******************************************************************************/
void
reGL3App::ProcessInput(float dt)
{
	if (m_input.WasKeyPressed(SDLK_ESCAPE))
		Quit();
}


/******************************************************************************
 * Render
 * Performs the drawing to GL context. The default is a blank screen
 ******************************************************************************/
void
reGL3App::Render(float dt)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(m_pWindow);
}

/******************************************************************************
 * Run
 * The function that iterates the main game-loop
 * It will exit only when m_isRunning is false, which can be set via Quit().
 ******************************************************************************/
void
reGL3App::Run()
{
	reTimer timer;
	timer.start();

	// If playing a demo, load the file here
	if (m_config.demo == RE_DEMO_PLAY && !LoadDemo()){
		fprintf(stderr, "Coult not load demo file!\n");
		return;
	}

	// Start the main loop
	while(m_isRunning){
		float elapsed = timer.getElapsed();

		if (m_config.demo == RE_DEMO_RECORD){
			m_curFrame 			= DemoFrame();
			m_curFrame.dt 		= elapsed;
		}
		else if (m_config.demo == RE_DEMO_PLAY){
			if (m_demoFrames.size() > 0){
				m_curFrame			= m_demoFrames.front();
				elapsed				= m_curFrame.dt;
				m_demoFrames.pop_front();
			}
			else{
				// demo is finished, resume normal play
				m_config.demo = RE_DEMO_NONE;
				printf("demo over\n");
			}
		}
		// Poll the SDL input buffer
		WinProc();
		if (m_config.demo == RE_DEMO_RECORD){
			m_demoFrames.push_back(m_curFrame);
		}

		// Process the input state
		ProcessInput(elapsed);
		// Perform Game Logic
		Logic(elapsed);
		// Render the scene
		Render(elapsed);

		if (m_config.demo != RE_DEMO_PLAY)
			SDL_Delay(int(m_config.sleepTime*1000));
	}
	printf("Average FPS: %.2f\n", timer.getFPS());

	// If recording a demo, now output the frame data
	if (m_config.demo == RE_DEMO_RECORD){
		SaveDemo();
	}
}

/******************************************************************************
 * WinProc
 * This is the function that polls SDL for input events
 * and then passes them to the Input handler
 ******************************************************************************/
void
reGL3App::WinProc()
{
	SDL_Event evt;
	
	if (m_config.demo == RE_DEMO_PLAY){
		// Still handle essential commands, like exit commands
		while (SDL_PollEvent(&evt))
		{
			switch(evt.type)
			{
				case SDL_KEYDOWN:
					if (evt.key.keysym.sym == SDLK_ESCAPE)
						m_input.PressKey(evt.key.keysym.sym);
					break;
				case SDL_KEYUP:
					if (evt.key.keysym.sym == SDLK_ESCAPE)
						m_input.ReleaseKey(evt.key.keysym.sym);
					break;
				case SDL_QUIT:
					Quit();
					break;
			}
		}
		// Go through this frame's events
		for (list<SDL_Event>::iterator e=m_curFrame.events.begin(); e!=m_curFrame.events.end(); e++){
			SDL_Event evt = *e;
			printf("Event %d\n", evt.type);
			switch(evt.type)
			{
				case SDL_KEYDOWN:
					m_input.PressKey(evt.key.keysym.sym);
					printf("pressed key\n");
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
					if (evt.wheel.y > 0)
						m_input.WheelUp();
					else
						m_input.WheelDown();
					break;
				case SDL_MOUSEBUTTONDOWN:
					m_input.PressButton(evt.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					m_input.ReleaseButton(evt.button.button);
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					return;
			}

		}
	}
	else
	{
		while (SDL_PollEvent(&evt))
		{
			switch(evt.type)
			{
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
					m_input.PressButton(evt.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					m_input.ReleaseButton(evt.button.button);
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					return;
			}
			if (m_config.demo == RE_DEMO_RECORD){
				m_curFrame.events.push_back(evt);
			}
		}
	}
}

/******************************************************************************
 * LoadDemo
 * Reads the per-frame event details from a file called "demo"
 ******************************************************************************/
bool
reGL3App::LoadDemo(){
	printf("Loading demo frames...\n");
	FILE* fp = fopen("demo","r");
	if (!fp){
		fprintf(stderr, "Could not open demo file\n");
		return false;
	}

	long int nFrames;
	fscanf(fp, "%ld", &nFrames);
	printf("\t%ld demo frames\n", nFrames);
	
	for (int i = 0; i < nFrames; i++){
		DemoFrame frame;

		fscanf(fp, "%f", &frame.dt);

		while(true){
			SDL_Event e;
			int t;

			// type
			fscanf(fp, "%d", &e.type);
			// If its the end-marker, break
			if (e.type==0)
				break;
			printf("Loaded %d %t\n", e.type);

			// details
			switch(e.type)
			{
				case SDL_KEYDOWN:
					fscanf(fp, "%d", &e.key.keysym.sym);
					break;
				case SDL_KEYUP:
					fscanf(fp, "%d", &e.key.keysym.sym);
					break;
				case SDL_MOUSEMOTION:
					fscanf(fp, "%d %d %d %d", &e.motion.x, &e.motion.y, &e.motion.xrel,
							&e.motion.yrel);
					break;
				case SDL_MOUSEWHEEL:
					fscanf(fp, "%d", &e.wheel.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
					fscanf(fp, "%d", &t);
					e.button.button = Uint8(t);
					break;
				case SDL_MOUSEBUTTONUP:
					fscanf(fp, "%d", &t);
					e.button.button = Uint8(t);
					break;
			}
			frame.events.push_back(e);
		}
		m_demoFrames.push_back(frame);
	}

	fclose(fp);
	printf("done\n");
	return true;
}

/******************************************************************************
 * SaveDemo
 * Saves the per-frame event details to a file called "demo"
 ******************************************************************************/
void
reGL3App::SaveDemo(){
	printf("Creating demo file...\n");
	FILE* fp = fopen("demo", "w");
	if (!fp){
		fprintf(stderr, "Failed to open demo file\n");
		return;
	}

	// Print the number of frames
	fprintf(fp, "%ld\n", m_demoFrames.size());

	for (list<DemoFrame>::iterator i = m_demoFrames.begin(); i!=m_demoFrames.end(); i++){
		DemoFrame frame = *i;
		// Print frame time and number of events
		fprintf(fp, "%.6f\n", frame.dt);

		for (list<SDL_Event>::iterator e = frame.events.begin(); e!=frame.events.end(); e++){
			switch(e->type)
			{
				case SDL_KEYDOWN:
					fprintf(fp, "  %d ", int(e->type));
					fprintf(fp, "%d\n", e->key.keysym.sym);
					printf("%d\n",e->type);
					break;
				case SDL_KEYUP:
					fprintf(fp, "  %d ", int(e->type));
					fprintf(fp, "%d\n", e->key.keysym.sym);
					break;
				case SDL_MOUSEMOTION:
					fprintf(fp, "  %d ", int(e->type));
					fprintf(fp, "%d %d %d %d\n", e->motion.x, e->motion.y, e->motion.xrel,
							e->motion.yrel);
					break;
				case SDL_MOUSEWHEEL:
					fprintf(fp, "  %d ", int(e->type));
					fprintf(fp, "%d\n", e->wheel.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
					fprintf(fp, "  %d ", int(e->type));
					fprintf(fp, "%d\n", int(e->button.button));
					break;
				case SDL_MOUSEBUTTONUP:
					fprintf(fp, "  %d ", int(e->type));
					fprintf(fp, "%d\n", int(e->button.button));
					break;
			}
		}
		fprintf(fp, "  0\n");
	}


	fclose(fp);
	printf("Done. Goodbye :D\n");
}
