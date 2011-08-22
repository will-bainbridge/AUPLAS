#ifndef HANDLE_H
#define HANDLE_H

#define handle(quit,value,message) \
	do{ \
		if(!(value)) \
		{ \
			if(quit) \
			{ \
				printf("\n[ERROR %s:%i] %s\n\n",__FILE__,__LINE__,message); \
				exit(EXIT_FAILURE); \
			} \
			else \
			{ \
				printf("\n[WARNING %s:%i] %s\n",__FILE__,__LINE__,message); \
			} \
		} \
	} while(0)

#endif
