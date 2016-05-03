/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <array.h>
#include <curthread.h>
#include <synch.h>
#include <machine/spl.h>


/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20


/*
 *
 * Function Definitions
 *
 */

static const char *directions[] = { "N", "E", "S", "W" };
unsigned order = 0;
struct array* north;
struct array* east;
struct array* south;
struct array* west;
struct sem* LRsem;
struct sem* UDsem;
struct sem* NWsem;
struct sem* NEsem;
struct sem* SWsem;
struct sem* SEsem;

/* str should be "approaching", "entering" or "leaving" */
void
message(const char *str, int carnumber, int cardirection, int destdirection)
{
	int spl = splhigh();
        kprintf("%s: car = %d, direction = %s, destination = %s\n", str, 
                carnumber, directions[cardirection], directions[destdirection]);
	splx(spl);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
  	int destdirection = (cardirection+2) % 4;
	message("approaching", carnumber, cardirection, destdirection);
	
	switch(cardirection){
	  case 0:
	      P((struct sem*)NWsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)SWsem);
	      V((struct sem*)NWsem);
	      V((struct sem*)SWsem);
	      break;
	  case 1:
	      P((struct sem*)NEsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)NWsem);
	      V((struct sem*)NEsem);
	      V((struct sem*)NWsem);
	      break;
	  case 2:
	      P((struct sem*)SEsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)NEsem);
	      V((struct sem*)SEsem);
	      V((struct sem*)NEsem);
	      break;
	  case 3:
	      P((struct sem*)SWsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)SEsem);
	      V((struct sem*)SWsem);
	      V((struct sem*)SEsem);
	      break;
	}
	      	      
	message("leaving", carnumber, cardirection, destdirection);	
        (void) cardirection;
        (void) carnumber;
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
	int destdirection = (cardirection+1) % 4;
  	message("approaching", carnumber, cardirection, destdirection);
	
	switch(cardirection){
	  case 0:
	      P((struct sem*)NWsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)SWsem);
	      V((struct sem*)NWsem);
	      P((struct sem*)SEsem);
	      V((struct sem*)SWsem);
	      V((struct sem*)SEsem);
	      break;
	  case 1:
	      P((struct sem*)NEsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)NWsem);
	      V((struct sem*)NEsem);
	      P((struct sem*)SWsem);
	      V((struct sem*)NWsem);
	      V((struct sem*)SWsem);
	      break;
	  case 2:
	      P((struct sem*)SEsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)NEsem);
	      V((struct sem*)SEsem);
	      P((struct sem*)NWsem);
	      V((struct sem*)NEsem);
	      V((struct sem*)NWsem);
	      break;
	  case 3:
	      P((struct sem*)SWsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      P((struct sem*)SEsem);
	      V((struct sem*)SWsem);
	      P((struct sem*)NEsem);
	      V((struct sem*)SEsem);
	      V((struct sem*)NEsem);
	      break;
	}
	
  	message("leaving", carnumber, cardirection, destdirection);
        (void) cardirection;
        (void) carnumber;
}



/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
	int destdirection = (cardirection+3) % 4;
  	message("approaching", carnumber, cardirection, destdirection);
	
	switch(cardirection){
	  case 0:
	      P((struct sem*)NWsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      V((struct sem*)NWsem);
	      break;
	  case 1:
	      P((struct sem*)NEsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      V((struct sem*)NEsem);
	      break;
	  case 2:
	      P((struct sem*)SEsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      V((struct sem*)SEsem);
	      break;
	  case 3:
	      P((struct sem*)SWsem);
	      message("entering", carnumber, cardirection, destdirection);	
	      V((struct sem*)SWsem);
	      break;
	}
	
  	message("leaving", carnumber, cardirection, destdirection);
        (void) cardirection;
        (void) carnumber;
}

void gothrough(int turn, int cardirection, int carnumber){
	// go through the intersection
	switch(turn){
	  case 0:
	     turnleft(cardirection,carnumber);
	     break;
	  case 1:
	     gostraight(cardirection,carnumber);
	     break;
	  case 2:
	     turnright(cardirection,carnumber);
	     break;
	}
	return;
}

// switches between the two queues
void governator2(){
    
    // while either q has cars in it...
    //while(q_empty(leftRightQ) != 1 || q_empty(upDownQ) != 1){
    while(1){
	// acquire left-right queue
	P(LRsem);
	int spl = splhigh();
	kprintf("Up Down Light\n");
	splx(spl);
	// wait for 5 seconds
	clocksleep(5);
	// release left-right queue
	V(LRsem);
	
	// acquire up-down queue
	P(UDsem);
	spl = splhigh();
	kprintf("Left Right Light\n");
	splx(spl);
	// wait for 5 seconds
	clocksleep(5);
	// release up-down queue
	V(UDsem);
    }

}

/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;
	int turn,spl;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
	(void) gostraight;
	(void) turnleft;
	(void) turnright;

        // cardirection is set randomly.
        cardirection = random() % 4;
	
	// select random turn direction
	turn = random() % 3;
	
	// wait for your turn to get in the queue
	while(carnumber > order){
	  thread_yield();
	}	
	order++;
	
	// add yourself to the right queue
	switch(cardirection){
	  case 0:
	    P(UDsem); //wait for the light
	    V(UDsem);
	    array_add(north,curthread);
	    while(array_getguy(north,0) != curthread){
		      thread_yield();}
	    gothrough(turn,cardirection,carnumber);
	    spl = splhigh();
	    array_remove(north,0);
	    splx(spl);
	    break;
	  case 1:
	    P(LRsem); //wait for the light
	    V(LRsem);
	    array_add(east,curthread);
	    while(array_getguy(east,0) != curthread){
	      	      thread_yield();}
	    gothrough(turn,cardirection,carnumber);
	    spl = splhigh();
	    array_remove(east,0);
	    splx(spl);
	    break;
	  case 2:
	    P(UDsem); //wait for the light
	    V(UDsem);
	    array_add(south,curthread);
	    while(array_getguy(south,0) != curthread){
	      	      thread_yield();}
	    gothrough(turn,cardirection,carnumber);
	    spl = splhigh();
	    array_remove(south,0);
	    splx(spl);
	    break;	  
	  case 3:
	    P(LRsem); //wait for the light
	    V(LRsem);
	    array_add(west,curthread);
	    while(array_getguy(west,0) != curthread){
	      	      thread_yield();}
	    gothrough(turn,cardirection,carnumber);
	    spl = splhigh();
	    array_remove(west,0);
	    splx(spl);
	    break;
	}
	
    /*
	// wait for the light!
	if(cardirection == 1 || cardirection == 3){
	  P(LRsem);
	  V(LRsem);
	} else {
	  P(UDsem);
	  V(UDsem);
	}*/	  
	return;
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

       // (void) nargs;
       // (void) args;
	
	// initialize lists and their semaphores
        north = array_create();
        east = array_create();
        south = array_create();
        west = array_create();
	array_preallocate(north,NCARS);
	array_preallocate(east,NCARS);
	array_preallocate(south,NCARS);
	array_preallocate(west,NCARS);
	LRsem = sem_create("LRQ",1);
	UDsem = sem_create("UDQ",1);
	NWsem = sem_create("NW",1);
	NEsem = sem_create("NE",1);
	SWsem = sem_create("SW",1);
	SEsem = sem_create("SE",1);
	
	error = thread_fork("governator thread",
                                    NULL,
                                    0,
                                    governator2,
                                    NULL
                                    );
				    
        // Start NCARS approachintersection() threads.
        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );
                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );	   
		}	
        }
        
       
	while(1);
        return 0;
}
