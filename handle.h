#ifndef HANDLE_H
#define HANDLE_H

#include <time.h>

#define exit_if_false(value,message) \
	do { \
		if(!(value)) \
		{ \
			printf("\n[ERROR %s:%i] %s\n\n",__FILE__,__LINE__,message); \
			exit(EXIT_FAILURE); \
		} \
	} while(0)

#define warn_if_false(value,message) \
	do { \
		if(!(value)) \
		{ \
			printf("\n[WARNING %s:%i] %s\n",__FILE__,__LINE__,message); \
		} \
	} while(0)

#define print_time(format,function) \
	do { \
		struct timespec time[2]; \
		clock_gettime(CLOCK_MONOTONIC, &time[0]); \
		function; \
		clock_gettime(CLOCK_MONOTONIC, &time[1]); \
		double seconds = difftime(time[1].tv_sec, time[0].tv_sec); \
		long nano_seconds = time[1].tv_nsec - time[0].tv_nsec; \
		printf(format, seconds + nano_seconds / 1.0e9); \
		fflush(stdout); \
	} while(0)

#define print_end() printf("\n\n")

#endif
