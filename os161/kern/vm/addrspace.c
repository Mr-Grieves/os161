#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <coremap.h>
#include <bitmap.h>
#include <coremap.h>
#include <vfs.h>
#include <vnode.h>
/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		kprintf("kmalloc fails\n");
		return NULL;
	}

	as->text_vbase = 0;
	as->text_npages = 0;
	as->text_offset = 0;
	as->text_filesize = 0;	
	as->heap_vbase = 0;
	as->heap_npages = 0;
	as->heap_vtop = 0;
	as->heap_offset = 0;
	as->heap_filesize = 0;
	as->data_vbase = 0;
	as->data_npages = 0;
	as->data_offset = 0;
	as->data_filesize = 0;
	as->as_stackpbase = USERTOP - 10*PAGE_SIZE;
	as->as_pagetable = pt_create();
	as->v = NULL;
	//kprintf("as: %x \n",as->as_pagetable);
	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	int spl = splhigh();
	struct addrspace *new;
	int i, j;
	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

	new->text_vbase = old->text_vbase;
	new->text_npages = old->text_npages;
	new->text_offset = old->text_offset;
	new->text_filesize = old->text_filesize;	
	new->heap_vbase = old->heap_vbase;
	new->heap_offset = old->heap_offset;
	new->heap_filesize = old->heap_filesize;
	new->heap_npages = old->heap_npages;
	new->heap_vtop = old->heap_vtop;
	new->data_vbase = old->data_vbase;
	new->data_npages = old->data_npages;
	new->data_offset = old->data_offset;
	new->data_filesize = old->data_filesize;
	new->v = old->v;
	VOP_INCREF(new->v);
	VOP_INCOPEN(new->v);
	
	if (old->as_pagetable->PT == NULL) {
	  new->as_pagetable->PT = NULL;
	} else {
	  new->as_pagetable->PT = kmalloc(1024*sizeof(unsigned int*));
	  
	  for (i = 0; i < 1024; i++) {
	    new->as_pagetable->PT[i] = NULL;
	  }
	  
	  //Copy every allocated frame
	  for (i = 0; i < 1024; i++) {    
	    if (old->as_pagetable->PT[i] != NULL) {
	      //Create second level array if needed
	      new->as_pagetable->PT[i] = kmalloc(1024*sizeof(unsigned int*));
	      for (j = 0; j < 1024; j++) {
		if (old->as_pagetable->PT[i][j] != 0) {
		  int temp = old->as_pagetable->PT[i][j]&0xfffff000;
		  temp -= cm_base;
		  temp /= PAGE_SIZE;
		  
		  //Sanity check
		  if (coremap[temp].isAllocated != 1) {
		    panic("Failure in as_copy. I DUN GOOFED.");
		  }
		  
	  
		  coremap[temp].isKernel = 1;
		  (old->as_pagetable->PT)[i][j] = ((old->as_pagetable->PT)[i][j]&0xFFFFF000)|TLBLO_VALID;
		  (new->as_pagetable->PT)[i][j] = (old->as_pagetable->PT)[i][j];
		  cm_map(new, i<<22|j<<12,old->as_pagetable->PT[i][j]&0xfffff000);

		 
		/*   kprintf("deleting some shit\n");
		      count++;
		      temp = as->as_pagetable->PT[i][j];
		      temp -= cm_base;
		      temp /= PAGE_SIZE;
		      coremap[temp].isAllocated = 0;
		      kfree(coremap[temp].list);
		      coremap[temp].isKernel = 0;
		      cm_numFree++;		      

		  }
	      }	*/
		
		 
	/*	  
  if (coremap[phys].list->as == as) {
    temp = (coremap[phys].list->next)->as;
  } else {
    temp = coremap[phys].list->as;
  }
  
  (temp->as_pagetable->PT)[PT1ind][PT2ind] = (temp->as_pagetable->PT)[PT1ind][PT2ind]|TLBLO_DIRTY|TLBLO_VALID;
  
  
  int newFrame = getppages(1);
  int phyAddr = newFrame;
  assert(coremap[(newFrame - cm_base)/PAGE_SIZE].isAllocated == 1);
  coremap[(newFrame - cm_base)/PAGE_SIZE].lastInBlock = 1;
  coremap[(newFrame - cm_base)/PAGE_SIZE].isKernel = 0;
  (as->as_pagetable->PT)[PT1ind][PT2ind] = newFrame|TLBLO_DIRTY|TLBLO_VALID;
  memcpy(PADDR_TO_KVADDR(phyAddr), PADDR_TO_KVADDR((temp->as_pagetable->PT)[PT1ind][PT2ind]&0xfffff000),PAGE_SIZE);
  cm_unmap(as, temptempthing); 
  
  if (coremap[phys].list->next = NULL)
    coremap[phys].isKernel = 0;

	
	//DEBUG(DB_VM, "Flushing TLB\n");
  int i;*/
		/*  int newFrame = getppages(1);
		  int phyAddr = newFrame;
		  //Copy coremap bits
		  assert(coremap[(newFrame - cm_base)/PAGE_SIZE].isAllocated == 1);
		  coremap[(newFrame - cm_base)/PAGE_SIZE].lastInBlock = coremap[temp].lastInBlock;
		  coremap[(newFrame - cm_base)/PAGE_SIZE].isKernel = coremap[temp].isKernel;
		  coremap[temp].isKernel = 0;
		  (old->as_pagetable->PT)[i][j] = ((old->as_pagetable->PT)[i][j]&0xFFFFF000)|TLBLO_VALID|TLBLO_DIRTY;
		  (new->as_pagetable->PT)[i][j] = (old->as_pagetable->PT)[i][j];
		  temp = (old->as_pagetable->PT)[i][j]&0xfff;*/
		  
/*	new->text_vbase = old->text_vbase;
	new->text_npages = old->text_npages;
	new->text_offset = old->text_offset;
	new->text_filesize = old->text_filesize;	
	new->heap_vbase = old->heap_vbase;
	new->heap_offset = old->heap_offset;
	new->heap_filesize = old->heap_filesize;
	new->heap_npages = old->heap_npages;*/
		 /* newFrame = newFrame|temp;
		  (new->as_pagetable->PT)[i][j] = newFrame;
		  memcpy(PADDR_TO_KVADDR(phyAddr), PADDR_TO_KVADDR((old->as_pagetable->PT)[i][j]&0xfffff000),
			 PAGE_SIZE);
		  //kprintf("Moving %d bytes from %x to %x\n", PAGE_SIZE,
			//  PADDR_TO_KVADDR((old->as_pagetable->PT)[i][j]&0xfffff000),PADDR_TO_KVADDR(phyAddr));*/
		} else {
		  (new->as_pagetable->PT)[i][j] = 0;
		}
	      }
	    }  
	  }  
	}
	
	splx(spl);
	*ret = new;
	return 0;
}

void
as_destroy(struct addrspace *as)
{

	//Didn't even manage to initialize the address space fully
	if (as->as_pagetable == NULL) {
	  kfree(as);
	  return;
	}
	
	int i, j, swap_index, count = 0;
	unsigned temp;
	
	
	
	
	
	for (i = 0; i < 1024; i++) {
	  
	  if (as->as_pagetable->PT[i] != NULL) {
	    
	    for (j = 0; j < 1024; j++) {
	      if (as->as_pagetable->PT[i][j] != 0) {
		  if ((as->as_pagetable->PT[i][j])&TLBLO_SWAPPED){//swapped
		      swap_index = as->as_pagetable->PT[i][j] >> 12;
		      //kprintf("deleting some swapped shit\n");
		      //kprintf("entry: %x swap index: %d\n",as->as_pagetable->PT[i][j],swap_index); 
		      //kprintf("is set?: %d\n",bitmap_Davids_better_isset(swap_map, swap_index));
		      //bitmap_unmark(swap_map, swap_index);	
		  } else {
		    temp = as->as_pagetable->PT[i][j];
		      temp -= cm_base;
		      temp /= PAGE_SIZE;
		    
		    if (coremap[temp].isKernel == 1) {
		      cm_unmap(as, as->as_pagetable->PT[i][j]);
		    } else {
		      //kprintf("deleting some shit\n");
		      count++;
		      coremap[temp].isAllocated = 0;
		      coremap[temp].isKernel = 0;
		      cm_numFree++;	
		    } 
		  }
		  //cm_unmap(as, as->as_pagetable->PT[i][j]);
	      }	
	    }
	    kfree(as->as_pagetable->PT[i]);
	  }  
	}
	vfs_close(as->v);
	DEBUG(DB_VM, "Destroyed %d page entries\n", count);
	kfree(as->as_pagetable);
	kfree(as);
}

void
as_activate(struct addrspace *as)
{
	int i, spl;

	(void)as;

	spl = splhigh();
	
	//DEBUG(DB_VM, "Flushing TLB\n");

	for (i=0; i<NUM_TLB; i++) {
		TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable,
		 int offset, int filesize)
{
	size_t npages; 
	
	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = sz / PAGE_SIZE;

	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;

	if (as->text_vbase == 0) {
		as->text_vbase = vaddr;
		as->text_npages = npages;
		as->text_offset = offset;
		as->text_filesize = filesize;
		kprintf("defining text, base: %x  pages: %d\n",vaddr, npages);
		return 0;
	}
	if (as->data_vbase == 0) {
		as->data_vbase = vaddr;
		as->data_npages = npages;
		as->data_offset = offset;
		as->data_filesize = filesize;
		
		//prepare heap here
		as->heap_vbase = as->data_vbase + as->data_npages*PAGE_SIZE;
		as->heap_vtop = as->heap_vbase;
		kprintf("defining data, base: %x  pages: %d\n",vaddr, npages);
		kprintf("defining heap, base: %x  top: %x\n",as->heap_vbase, as->heap_vtop);
		return 0;
	}
	
	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	//assert(as->as_stackpbase != 0);

	*stackptr = USERSTACK;
	//as->as_stackpbase = USERSTACK;
	return 0;
}

