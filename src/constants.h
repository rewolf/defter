// reGL Library includes
#include "regl3.h"
#include "re_input.h"
#include "re_math.h"
#include "re_shader.h"
#include "re_timer.h"

// Other common includes
#include <vector>
#include <list>
#include <queue>
#include <limits.h>
#include <assert.h>
#include <map>

// External library includes
#include "FreeImage.h"

// NOTE: util.h included at BOTTOM!!!!!!



#ifdef _WIN32
//#	pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#	pragma comment(linker, "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
#	define GLEW_STATIC 1
#	pragma comment(lib, "opengl32.lib")
#	pragma comment(lib, "sdl.lib")
#	pragma comment(lib, "sdlmain.lib")
#	pragma comment(lib, "freeimage.lib")

#	include <direct.h>
#	define mkdir(x) _mkdir(x)


#define log2(n)				(logf(n)/logf(2))
#else
#	include <sys/stat.h>
#	define mkdir(x) mkdir(x, S_IRWXU)
#endif



// Value of PI
#define PI					(3.14159265358979323846264338327950288f)



// Near and Far plane for view frustum
#define NEAR_PLANE			(0.5f)
#define FAR_PLANE			(1000.0f)



// Wrapping macro
#define WRAP_POS(p,b)		( p < -b * .5f ? p + b : \
							( p >  b * .5f ? p - b : p))


// App Settings
#define FSAA				(0)
#define SLEEP_TIME			(0.0f)
#define VSYNC				(false)



// Screen settings
#define SCREEN_W			(1024)
#define SCREEN_H			(768)
#define ASPRAT				(float(SCREEN_W) / SCREEN_H)
#define SCREENSHOT_W		(1024)
#define SCREENSHOT_H		(768)



// Texture locations
#define COARSEMAP_FILENAME	("images/coarsemap.png")
#define COARSEMAP_TEXTURE	("images/coarsemap_tex3.png")
#define	SPLASHMAP_TEXTURE	("images/splash.png")
#define SKYBOX_TEXTURE		("images/skybox001.png")



// Shader Manager settings
#define SHADERNUM			(2)



const vector3 	GRAVITY		= vector3(0.0f, -19.81f, 0.0f);
#define ACCELERATION		(3.5f)
#define AIR_DRAG			(0.6f)
#define FRICTION			(1.8f)
#define DT 					(0.005f)
#define invDT   			(1.0f / DT)


#define CLIPMAP_DIM			(255)
#define CLIPMAP_RES			(0.1f)
#define CLIPMAP_LEVELS		(5)
#define CACHING_LEVEL		(2)
#define CACHING_DIM			((CLIPMAP_DIM + 1) * CACHING_LEVEL)
#define HIGH_DIM			(1024 * CACHING_LEVEL)
#define HIGH_RES			(CLIPMAP_RES / 3.0f)
#define HD_AURA				((HIGH_DIM * HIGH_RES * .9f)/2.0f)
#define HD_AURA_SQ			(HD_AURA * HD_AURA)
#define COARSE_AURA			((CLIPMAP_DIM + 1) * 8 * CLIPMAP_RES)
#define VERT_SCALE			(40.0f)
#define EYE_HEIGHT			(2.0f)
#define STEP_TIME			(0.4f)

#define MAP_TRANSFER_WAIT	(.01f)	// N second gap after deform, before downloading it
#define MAP_BUFFER_CYCLES	(0)	// After commencing download, wait a few cycles before mapping



// Caching constants
#define WRAP(val, dim) 		((val < 0) ? (val + dim) : ((val > (dim - 1)) ? (val - dim) : val))
#define OFFSET(x, y, dim) 	((y) * dim + (x))
#define LOAD_CYCLES			(4)
#define	UNLOAD_CYCLES		(2)
#define READS_PER_FRAME		(3)
#define LOCK(m)				{ if (SDL_LockMutex(m) == -1) fprintf(stderr, "Mutex Lock Error\n"); }
#define UNLOCK(m)			{ if (SDL_UnlockMutex(m) == -1) fprintf(stderr, "Mutex Unlock Error\n"); }


// Radar settings
#define RADAR_OFFSET		(20.0f)
#define RADAR_SIZE			(200.0f)
#define RADAR2_SIZE			(RADAR_SIZE / 2.0f)
#define RADAR_LINE_W		(1.0f / RADAR_SIZE)
#define RADAR2_LINE_W		(1.0f / RADAR2_SIZE)
#define RADAR_DOT_R			(16.0f / (RADAR_SIZE * RADAR_SIZE))
#define RADAR2_DOT_R		(16.0f / (RADAR2_SIZE * RADAR2_SIZE))



// Debug and profiling controls
#define DEBUG_ON			(0)
#define PROFILE				(0)

#if DEBUG_ON
#	define DEBUG(...)		printf(__VA_ARGS__)
#else
#	define DEBUG(...)		{}
#endif

#if PROFILE
	reTimer g_profiler;
	float timeCount = 0;
	long frameCount = 0;
#	define BEGIN_PROF		{glFinish(); g_profiler.start();}
#	define END_PROF			{glFinish(); timeCount+=g_profiler.getElapsed(); frameCount++;}
#	define PRINT_PROF		{printf("Average render: %.3fms\n", timeCount*1000.0f / frameCount);}
#else
#	define BEGIN_PROF		{}
#	define END_PROF			{}
#	define PRINT_PROF		{}
#endif



// Inlude the Util last so that it can use these values
#include "util.h"