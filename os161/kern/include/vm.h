#ifndef _VM_H_
#define _VM_H_

#include <machine/vm.h>
#include <bitmap.h>
#include <vfs.h>
#include <synch.h>
#include <vnode.h>
#include <uio.h>

/*
 * VM system-related definitions.
 *
 * You'll probably want to add stuff here.
 */


/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/


/* Initialization function */
void vm_bootstrap(void);

/* find an unallocated page and write frame to it */
void writeout(int entry);

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);

void swap_replacement(void);

/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(int npages);
void free_kpages(vaddr_t addr);
paddr_t getppages(unsigned long npages);

struct bitmap *swap_map;
struct vnode *swap_vnode;
struct lock *swap_lock;

int swap_map_size;


#endif /* _VM_H_ */
