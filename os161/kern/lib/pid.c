#include <types.h>
#include <pid.h>
#include <lib.h>
#include <kern/unistd.h>
#include <machine/spl.h>
#include <machine/pcb.h>
#include <curthread.h>
#include <kern/errno.h>
#include <thread.h>

//Only used in bootstrap
void pid_initialize(void) {
  
  int i;
  pidCount = 0;
  if (PROCESS_MAX <= 1) {
    panic("Need to allow more than one process");
  }
  
  //Initialize hash table
  for (i = 0; i < PROCESS_MAX; i++) {
    pid_hash[i] = NULL;
  }
   
  //Create a lock. Better have worked.
  pid_lock = lock_create("PID LOCK");
  if (pid_lock == NULL) {
    panic("Couldn't create pid lock");
  }
  
  //Create boot thread PID
  pid_hash[0] = pid_makeStruct(0, -1);
  if (pid_hash[0] == NULL) {
    panic("Couldn't create main pid struct");
  }
    
  numPID = 1;
  nextPID = 1;
 
}

//Makes a new pid_data struct. Null if anything goes wrong.
pid_data *pid_makeStruct(int pid, int ppid) {
  
  //Make sure all malloc'ing works
  pid_data *temp = kmalloc(sizeof(pid_data));
  if (temp == NULL) {
    return NULL;
  }
 
  temp->waitLock = lock_create("Wait Lock");
  if (temp->waitLock == NULL) {
    kfree(temp);
    return NULL;
  }
  
  lock_acquire(temp->waitLock);
  temp->pid = pid;
  temp->ppid = ppid;
  temp->pidExited = 0;
  temp->exitStatus = 0xf005ba11; //Awesome game, and unlikely exit status
  temp->numInterested = 1; //Parent cares
  
  return temp;
  
}

//Allocates a new pid
int pid_allocate(int *value) {
 
  
  //Only one person can be doing this at once
  lock_acquire(pid_lock);
  
  //Oh noes
  if (numPID >= PROCESS_MAX) {
    lock_release(pid_lock);
    return ENOMEM;
  }
  
  //Since numPID is less than PROCESS_MAX, this shouldn't infinite loop
  while(1) {
    if (pid_hash[nextPID % PROCESS_MAX] == NULL) {
      break;
    } else {
      nextPID++;
      if (nextPID == 0) {
	nextPID++;
      }
    }
  }
 
  
  //nextPID is now the new PID
  pid_hash[nextPID % PROCESS_MAX] = pid_makeStruct(nextPID, curthread->pid_ID);
  if (pid_hash[nextPID % PROCESS_MAX] == NULL) {
    lock_release(pid_lock);
    return EAGAIN;
  }  
 
  *value = pid_hash[nextPID % PROCESS_MAX]->pid;
  numPID++;
  nextPID++;
  
  DEBUG(DB_THREADS, "New PID: %d, total PIDs: %d\n", *value, numPID);
  
  lock_release(pid_lock);
  return 0; //Success
    
}

//Recycles pid
void pid_purge(int pid) {
  
  if (pid_hash[pid % PROCESS_MAX] == NULL) {
    panic("PID doesn't exist and I'm trying to recycle it");
  }
 
  //Kill CV
  lock_destroy(pid_hash[pid % PROCESS_MAX]->waitLock);
  //Frees structure
  kfree(pid_hash[pid % PROCESS_MAX]);
  //Reassigns NULL
  pid_hash[pid % PROCESS_MAX] = NULL;
  //One less PID
  numPID--;
 
}

//Set exit
void pid_setExit(int pidID, int exitValue) {
  
  lock_acquire(pid_lock);
  
  if (pid_hash[pidID % PROCESS_MAX] == NULL) {
    panic("PID doesn't exist and I'm trying to exit it");
  }
  
  pid_hash[pidID % PROCESS_MAX]->pidExited = 1;
  pid_hash[pidID % PROCESS_MAX]->exitStatus = exitValue;
  lock_release(pid_hash[pidID % PROCESS_MAX]->waitLock);
  lock_release(pid_lock);
}

int pid_wait(int pid, int *status, int options) {
  
  (void)options;
  
  //lock_acquire(pid_lock);
  
  if (pid_hash[pid % PROCESS_MAX]->numInterested >= 1) {
    if (pid_hash[pid % PROCESS_MAX]->ppid == curthread->pid_ID) {
      //I'm the parent and I care and it has exited
      if (pid_hash[pid % PROCESS_MAX]->pidExited == 1) {
	*status = pid_hash[pid % PROCESS_MAX]->exitStatus;
	pid_hash[pid % PROCESS_MAX]->numInterested--;
	//lock_release(pid_lock);
	return pid;
      } else { //I'm the parent and it hasn't exited
	//lock_release(pid_lock);
	lock_acquire(pid_hash[pid % PROCESS_MAX]->waitLock);
//	lock_acquire(pid_lock);
	lock_release(pid_hash[pid % PROCESS_MAX]->waitLock);
	*status = pid_hash[pid % PROCESS_MAX]->exitStatus;
	pid_hash[pid % PROCESS_MAX]->numInterested--;
//	lock_release(pid_lock);
	return pid;
      }
    }
  } else {
    panic("Non parent attempting to wait on child. This can happen?");
    return pid; //Shouldn't get here
  }

}

int pid_didExit(int pid) {
  if (pid_hash[pid % PROCESS_MAX]->pidExited == 1) {
    return 1;
  } else {
    return 0;
  }
}

int pid_numInterest(int pid) {
  return pid_hash[pid % PROCESS_MAX]->numInterested;
}














