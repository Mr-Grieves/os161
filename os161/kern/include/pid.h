#ifndef PID_H

#define PID_H
#define PROCESS_MAX 64

#include <array.h>
#include <synch.h>

/*PID data struct. Holds all important PID information*/
typedef struct {
	int pid; //Process ID. Every thread is going to get one
	int ppid; //Parent process ID
	volatile int pidExited; //True if exited
	int exitStatus;	//Only makes sense if pidExited is true
	int numInterested;
	struct lock *waitLock; //People wait here
	
} pid_data;

//Hash table of pid_data. Hash using nextPID % PROCESS_MAX
pid_data *pid_hash[PROCESS_MAX];
//Overflow is inconsequential 
unsigned int nextPID;
//How many pids are allocated
unsigned int numPID;
//Lock for pid stuff
struct lock* pid_lock;




//Should only be called once in bootstrap (otherwise ERRORS!)
void pid_initialize(void);
//Returns 0 if successful, and an error code elsewise. New PID in value
int pid_allocate(int *value);
//The pid MUST exist if you're calling this
void pid_purge(int pid);
//Sets pidExited to true, and sets an exit value
void pid_setExit(int pidID, int exitValue);



//Helpfully creates a new pid_data struct
pid_data *pid_makeStruct(int pid, int ppid);
//Helpfully does stuff for waitpid (AKA: Everything)
int pid_wait(int pid, int *status, int options);
//Has exited
int pid_didExit(int pid);
//Wake
void pid_interestWake(int pid);
//Find interested
int pid_numInterest(int pid);
int pidCount;


#endif
