/*
 * hog.c
 * 	Spawned by several other user programs to test time-slicing.
 *
 * This does not differ from guzzle in any important way.
 */
#include <unistd.h>
int
main(int argc, char *argv[])
{
	volatile int i;
	
	for (i = 0; i < argc; i++) {
	  printf("I'm a hog and arg %d is %s\n", i, argv[i]);
	}

	for (i=0; i<500000; i++)
		;
	
	printf("Done being a hog\n");

	return 0;
}
