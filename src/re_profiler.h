

#ifndef _RE_PROFILER_H_
#define _RE_PROFILER_H_

#define RE_PROFILE 1
#if RE_PROFILE
#	include "re_timer.h"

// Create an array of timers
#	define N_PROFILERS			(20)
	extern reTimer __g_profilers__[N_PROFILERS];
	extern int __g_counters__[N_PROFILERS];
	extern float __g_elapsed__[N_PROFILERS];
#	define __RE_P_				__g_profilers__
#	define __RE_C_				__g_counters__
#	define __RE_E_				__g_elapsed__

// Regular timer
#	define PROF_BEGIN(id)		__RE_P_[id].start()
#	define PROF_CHECK(id)		printf("%d at %.3fms\n",id,__RE_P_[id].peekElapsed()*1000)
#	define PROF_END(id, name)	printf("%s : %.3fms\n",name,__RE_P_[id].getElapsed()*1000)

// OpenGL Timer (invokes glFinish)
#	define GLPROF_BEGIN(id)		\
		glFinish();				\
		__RE_P_[id].start();		
#	define GLPROF_END(id, name)	\
		glFinish();				\
		printf("%s : %.3fms\n",name,__RE_P_[id].getElapsed()*1000);

// Average timers.  Records many iterations of the elapsed time between BEGIN and END and shows the
// average with RESULT
#	define GLPROF2_BEGIN(id)	\
		glFinish();				\
		__RE_P_[id].start()
#	define GLPROF2_END(id)		\
		glFinish();				\
		__RE_C_[id]++;			\
		__RE_E_[id]+=__RE_P_[id].getElapsed()*1000
#	define GLPROF2_RESULT(id, name)	\
		glFinish();				\
		printf("%s : %.3fms\n",name,__RE_E_[id]/__RE_C_[id])
#	define GLPROF2_RESET(id)	\
		__RE_C_[id] = 0;		\
		__RE_E_[id] = .0f;

#else

#	define PROF_BEGIN(id)		{}
#	define PROF_CHECK(id)		{}
#	define PROF_END(id, msg)	{}
#	define GLPROF_BEGIN(id)		{}
#	define GLPROF_END(id, msg)	{}
#	define GLPROF2_BEGIN(id)	{}
#	define GLPROF2_END(id, msg)	{}
#	define GLPROF2_RESULT(id)	{}
#	define GLPROF2_RESET(id)	{}


#endif

#endif
