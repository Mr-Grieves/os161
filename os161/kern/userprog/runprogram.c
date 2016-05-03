/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, int argc, char **argv)
{
	vaddr_t entrypoint, stackptr;
	int result;
	
	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	
	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &(curthread->t_vmspace->v));
	if (result) {
		kprintf("vfs sucks\n");
		return result;
	}

	if (curthread->t_vmspace==NULL) {
		vfs_close(curthread->t_vmspace->v);
		kprintf("as_create sucks\n");
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(curthread->t_vmspace->v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		kprintf("loadelf sucks\n");
		vfs_close(curthread->t_vmspace->v);
		return result;
	}

	/* Done with the file now. */
	//vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}
	
	//Write to stack, stuff, stuff, and stuff
	int i;
	char **user_argv = kmalloc(sizeof(char*) * (argc + 1));
	user_argv[argc] = NULL;
	
	
	for (i = argc - 1; i >= 0; i--) {
	  int length = strlen(argv[i]) + 1;
	  int padding = 4 - length % 4; 
	  stackptr -= length + padding;
	  copyoutstr(argv[i], stackptr, length, NULL);
	  user_argv[i] = stackptr;
	}
	
	
	for (i = argc; i >= 0; i--) {
	  stackptr -= 4;
	  copyout(&(user_argv[i]), stackptr, sizeof(4));
	}
	  
	/* Warp to user mode. */
	md_usermode(argc, stackptr,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

