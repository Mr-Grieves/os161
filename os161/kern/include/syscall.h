#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_printchar(int input);
int sys_readchar(void);
int sys_getpid(int* retval);
int sys_fork(struct trapframe* parentTF, int* retval);
void sys_exit(int value);
int sys_waitpid(int pid, int *status, int options,int *err);
int sys_execv(char* prog, char** args, int *retval);
int sys_sbrk(int amount, int *retval);

void md_modifyRegisters(struct trapframe* childTF);
void md_forkentry(void **ptr);

#endif /* _SYSCALL_H_ */
