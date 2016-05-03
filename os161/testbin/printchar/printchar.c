#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#define 	SIZE 	4096*17

int* dostuff(){
     int* ptr = malloc(SIZE*sizeof(int));
     int i;
     for(i = 0; i < SIZE; i++){
	ptr[i] = i;
     }
     return ptr;
}

int
main(int argc, char *argv[])
{
  /*  int status, options, i;
    char* filename = "/testbin/add";
    char* args[4];
    pid_t pid; 
    
    case '1':
	if(fork()==0)
	    //child!
	    printf("ima child!\n");
	else
	    printf("ima adult!\n");
	break;

	args[0] = "add";
	args[1] = "1";
	args[2] = "2";
	args[3] = NULL;
	
	pid = fork();
	if (pid == 0) execv(filename,args);
	else waitpid(pid,&status,options);
*/	
	int* yay;
	int i;
	yay = dostuff();
	for(i = 0; i < SIZE; i=i+100)
	  printf("%d\n", yay[i]);
    
    //return 0;
}