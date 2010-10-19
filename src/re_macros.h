

#ifndef _RE_MACROS_H_
#define _RE_MACROS_H_

#include "re_profiler.h"

#include "re_math.h"
#define PRINTINT(x)				\
 	printf("%s:\t%d\n", #x, x)
#define PRINTFLOAT(x)			\
 	printf("%s:\t%.3f\n", #x, x)
#define PRINTVEC2(v)			\
 	printf("%s:\t<%5.3f, %5.3f>", #v, v.x, v.y)
#define PRINTVEC3(v)			\
	printf("%s:\t<%5.3f, %5.3f, %5.3f>", #v, v.x, v.y, v.z)


#if RE_DEBUG
#	define DEBUG(...) 			printf(__VA_ARGS__)
#else
#	define DEBUG(...)			{}
#endif

#endif
