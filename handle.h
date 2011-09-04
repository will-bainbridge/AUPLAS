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
		struct timespec handle_time[2]; \
		clock_gettime(CLOCK_MONOTONIC, &handle_time[0]); \
		function; \
		clock_gettime(CLOCK_MONOTONIC, &handle_time[1]); \
		double handle_seconds = difftime(handle_time[1].tv_sec, handle_time[0].tv_sec); \
		long handle_nano_seconds = handle_time[1].tv_nsec - handle_time[0].tv_nsec; \
		printf(format, handle_seconds + handle_nano_seconds / 1.0e9); \
		fflush(stdout); \
	} while(0)

#define print_end() printf("\n\n")

#endif
