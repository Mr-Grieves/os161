/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>
#include <array.h>

#define BASE_SLEEPER_SIZE_LOCK 5
////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	//Not locked
	lock->isLocked = 0;
	//No owner
	lock->ownerAddr = NULL;
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);
	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	int spl;
	//Need to actually give me a lock to use
	assert(lock != NULL);
	//Make sure we're not blocking an interrupt handler
	assert(in_interrupt == 0);
	//Save old interrupt values, make us uninterruptable
	spl = splhigh();
	
	while (1) {
	  if (lock->isLocked == 0) {
	    //We're good! Onwards!
	    lock->isLocked = 1;
	    lock->ownerAddr = curthread;
	    //Fix interrupts you silly goose
	    splx(spl);
	    return;
	  } else {
	    //Sleep on the address of the lock. This might cause starvation.
	    thread_sleep(lock);
	  }
	}
}

void
lock_release(struct lock *lock)
{
	int spl;
	
	//We can't release one we don't own or one that doesn't exist
	assert(lock != NULL);
	//Don't be in an interrupt handler
	assert(in_interrupt == 0);
	
	//This should probably be atomic
	spl = splhigh();
	if (1) {
	  //No one owns this now
	  lock->ownerAddr = NULL;
	  //Unlocked
	  lock->isLocked = 0;
	  //Wake up threads if there are any
	  thread_wakeup(lock);
	  
	}
	
	//Reallow interrupts
	splx(spl);
}

int
lock_do_i_hold(struct lock *lock)
{
	assert(in_interrupt == 0);
	int spl = splhigh();
	//Did you pass me a real lock? If not I definitely don't own it
	if (lock == NULL) {
	  return 0;
	}
	
	if (lock->ownerAddr == curthread) {
	  //It's yours!
	  splx(spl);
	  return 1;
	} else {
	  //It's not youts =(
	  splx(spl);
	  return 0;
	}
}


////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	


	//create thread array
	cv->threads = array_create();
	//preallocate array space
	array_preallocate(cv->threads, 10);
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);
	array_destroy(cv->threads);
	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
        // Atomically, do the following:
        //   1. release the lock
        //   2. put the thread to sleep on the condition variable "cv"
        //   3. re-acquire the lock
        int spl;
        spl = splhigh();

	//Make sure the lock exists
	assert(lock != NULL);
	//Make sure we're not blocking an interrupt handler
	assert(in_interrupt == 0);

        lock_release(lock);
	
	// add current thread to threads array
	array_add(cv->threads, curthread);

	// sleep the thread
	thread_sleep(curthread);  	
	
	lock_acquire(lock);
	splx(spl);

	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// Write this
	int spl;
	spl = splhigh();
	
	if(array_getnum(cv->threads) > 0){
	    thread_wakeup(array_getguy(cv->threads,0));
	    array_remove(cv->threads,0);
	}	

	splx(spl);
     
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	int spl,i;
	spl = splhigh();
           
	// wake up all threads sleeping on cv
	for(i = array_getnum(cv->threads)-1; i >= 0; i--){
	    //thread_wakeup(cv->threads[i]);
	    //array_remove(cv->threads, i);
	    cv_signal(cv,lock);
	}

	splx(spl);

	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}
