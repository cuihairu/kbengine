#ifndef KBE_TESTS_PLATFORM_H
#define KBE_TESTS_PLATFORM_H

// Platform-specific includes and function wrappers for test code

#ifdef _WIN32
	#include <process.h>
	#define KBE_GETPID() _getpid()
#else
	#include <unistd.h>
	#define KBE_GETPID() getpid()
#endif

#endif // KBE_TESTS_PLATFORM_H
