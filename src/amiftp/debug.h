#include <interfaces/exec.h>

#if defined(_DEBUG_)
	#define DBUG(x,...) DebugPrintF("[%s:%-4ld] "x ,__FILE__,__LINE__, ##__VA_ARGS__)
#else
	#define DBUG(x,...)
#endif
