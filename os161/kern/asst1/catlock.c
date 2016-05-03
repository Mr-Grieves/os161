/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
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
#include <synch.h>
#include <machine/spl.h>

struct lock* arrLocks[8] = {NULL};
struct lock* bowlOne;
struct lock* bowlTwo;

/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2


/*
 * 
 * Function Definitions
 * 
 */



/* who should be "cat" or "mouse" */
static void
eat(const char *who, int num, int iteration)
{
        kprintf("%s: %d starts eating: %d\n", who, num, iteration);
        clocksleep(1);
        kprintf("%s: %d ends eating: %d\n", who, num, iteration);
}


//Cycles through an array of locks, waking up every two threads
void governator() {
  while (1) {
    int i;
    for (i = 0; i < 8; i += 2) {
      //Release the bowls to begin
      lock_release(bowlOne);
      lock_release(bowlTwo);
      //Wake up two animals, either both cats or both mice
      int spl = splhigh();
      thread_wakeup(arrLocks[i]);
      thread_wakeup(arrLocks[i + 1]);
      splx(spl);
      //Reacquire the bowls
      lock_acquire(bowlOne);
      lock_acquire(bowlTwo);
    }
  }
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
	int iteration = 0;
        while (1) {
	  int i, spl = splhigh();
	  //Acquire free locks
	  for (i = 2; i < 8; i++) {
	    if (arrLocks[i]->isLocked == 0) {
	      lock_acquire(arrLocks[i]);
	      break;
	    }
	  }
	  for (i = 2; i < 8; i++) {
	    if (lock_do_i_hold(arrLocks[i])) {
	      thread_sleep(arrLocks[i]);
	      splx(spl);
	      while (1) {
		//Wait for a free bowl and grab it
		if (bowlOne->isLocked == 0) {
		  lock_acquire(bowlOne);
		  iteration++;
		  eat("Cat", catnumber, iteration);
		  lock_release(arrLocks[i]);
		  lock_release(bowlOne);
		  break;
		} else {
		  lock_acquire(bowlTwo);
		  iteration++;
		  eat("Cat", catnumber, iteration);
		  lock_release(arrLocks[i]);
		  lock_release(bowlTwo);
		  break;
		}
	      }
	      break;
	    }
	  }
	  /*if (catnumber % 2  == 0) 
	    clocksleep(5);*/
	}
	    
	   

        (void) unusedpointer;
        (void) catnumber;
}
	

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
	int iteration = 0;
	while (1) {
	  int spl = splhigh();
	  //Find a free lock in the array of locks
	  if (arrLocks[0]->isLocked == 0) {
	    lock_acquire(arrLocks[0]);
	  } else {
	    lock_acquire(arrLocks[1]);
	  }
	  
	  while (1) {
	    //Wait for a free bowl to eat from
	    if (lock_do_i_hold(arrLocks[0])) {
	      thread_sleep(arrLocks[0]);
	      lock_acquire(bowlOne);
	      splx(spl);
	      iteration++;
	      eat("Mouse", mousenumber, iteration);
	      lock_release(arrLocks[0]);
	      lock_release(bowlOne);
	      break;
	    } else {
	      thread_sleep(arrLocks[1]);
	      lock_acquire(bowlTwo);
	      splx(spl);
	      iteration++;
	      eat("Mouse", mousenumber, iteration);
	      lock_release(arrLocks[1]);
	      lock_release(bowlTwo);
	      break;
	    }
	  }
	  
	}
    
        (void) unusedpointer;
        (void) mousenumber;
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error, i;
	(void) nargs;
	(void) args;
   
        /*
         * Avoid unused variable warnings.
         */
	
	//Initialize and validate
	for (i = 0; i < 8; i++) {
	  arrLocks[i] = lock_create("A lock!");
	  assert((arrLocks[i]) != NULL);
	}
	bowlOne = lock_create("Bowl one");
	bowlTwo = lock_create("Bowl two");
	assert(bowlOne != NULL && bowlTwo != NULL);
	
	
        /*
         * Start NCATS catlock() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        governator();

        return 0;
}

/*
 * End of catlock.c
 */
