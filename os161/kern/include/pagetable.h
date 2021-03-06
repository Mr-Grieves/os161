#ifndef _PAGETABLE_H_
#define _PAGETABLE_H_

#include <vm.h>
#include "opt-dumbvm.h"
struct vnode;

struct pagetable {
	/* Two level Page Table */
	// array of size 2^10, of pointers to 2nd level page tables
	unsigned int** PT;
	
};


struct pagetable   *pt_create(void);
unsigned int	   *pt_read(struct pagetable* pt, vaddr_t vaddr);
void pt_protection(struct addrspace* as, vaddr_t vaddr) ;
void 		   pt_write(struct pagetable* pt, vaddr_t vaddr);
#endif /* _ADDRSPACE_H_ */
