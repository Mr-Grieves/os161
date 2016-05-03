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
#include <kern/stat.h>
#include <kern/unistd.h>

#define DUMBVM_STACKPAGES    12
#define SWAP_DEVICE "lhd0raw:"

void
vm_bootstrap(void){
	int retval;
	struct stat a;
	swap_lock = lock_create("swap_lock");
	swap_vnode = kmalloc(sizeof(struct vnode));
	//a = kmalloc(sizeof(struct stat));
	retval = vfs_open((char*)SWAP_DEVICE, O_RDWR, &swap_vnode);
	if (retval) {
	    panic("couldn't open swap device\n");
	}	
	
	VOP_STAT(swap_vnode, &a);
	swap_map = bitmap_create(a.st_size/PAGE_SIZE);
	swap_map_size = a.st_size/PAGE_SIZE;
	if (!swap_map)
	    panic("couldn't create swap map\n");
}

void writeout(int entry){
	struct uio u;
	struct addrspace *as;
	u_int32_t swap_index;
	int result, vaddr;
     
	//kprintf("allocating swap frame...\n");
	// find an unallocated page in the swap bitmap and set it	
	bitmap_alloc(swap_map, &swap_index);

	//writeout that shit
	mk_kuio(&u, PADDR_TO_KVADDR(entry*PAGE_SIZE+cm_base), PAGE_SIZE, swap_index * PAGE_SIZE, UIO_WRITE);
	result = VOP_WRITE(swap_vnode, &u);
	
	//change the pt entry to point to disk
	as = coremap[entry].list->as;
	vaddr = coremap[entry].list->virtual_addr;
	//do everything in ONE FREAKING LINE
	//kprintf("swapping out, this entry: %x to this disk offset: %x\n",entry,(swap_index * PAGE_SIZE));
	as->as_pagetable->PT[(vaddr&0xffc00000)>>22][(vaddr&0x3ff000)>>12] = (swap_index<<12)|TLBLO_SWAPPED;
}

paddr_t
getppages(unsigned long npages)
{
 
	int spl;

	spl = splhigh();

	unsigned i, firstIndex, count;
	
	//SWAPPIN'!
	if (npages >= cm_numFree) {
	  //kprintf("ATTEMPTING TO SWAP! If things go to shit after this, we know the reason\n");
	  if (npages != 1) {
	    panic("FFUIFFSDUFUSDFUFUUFUU");
	  }
	  swap_replacement();
	}
	
	//kprintf("Requesting %d page(s)\n", (int)npages);
	
	count = 0;
	firstIndex = 0;
	for (i = 0; i < cm_numEntries; i++) {
	  if (coremap[i].isAllocated == 0) {
	    count++;
	    if (count == 1) {
	      firstIndex = i;
	    }
	    if (count == npages) {
	      //Found enough consecutive frames. Sweet
	      break;
	    }    
	  } else {
	    count = 0;
	  } 
	}
	
	if (count != npages) {
	  kprintf("Couldn't allocate contiguous pages. Swapping should have fixed this\n");
	  return 0;
	}
	
	count = 0;
	for (i = firstIndex; i < firstIndex + npages; i++) {
	  coremap[i].isAllocated = 1;
	  coremap[i].lastInBlock = 0;
	  coremap[i].list = NULL;
	  if (allocatingKernel) {
	    coremap[i].isKernel = 1;
	  }
	  count++;
	  cm_numFree--;
	}
	
	coremap[i - 1].lastInBlock = 1;
	
	//for (i = firstIndex; i < firstIndex + npages; i++) {
	//  kprintf("%d%d  ", coremap[i].isAllocated, coremap[i].lastInBlock);
	//}
	
	//kprintf("\n");
	
	if (npages != count)
	kprintf("Wanted %d pages, allocated %d\n", (int)npages, (int)count);

	
	//int temp = ram_stealmem(npages);
	
	//if (temp != cm_base + firstIndex*PAGE_SIZE)
	//kprintf("ram steal thinks it is %x, I think it is %x\n", temp, cm_base + firstIndex*PAGE_SIZE);
	
	splx(spl);
	
	//kprintf("Coremap has %d free frames\n", cm_numFree);
	
	return cm_base + firstIndex*PAGE_SIZE;
	
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
	paddr_t pa;
	pa = getppages(npages);
	if (pa==0) {
		return 0;
	}
	/*Anything allocated through here is unswappable*/
	unsigned page = (pa - cm_base)/PAGE_SIZE;
	int i;
	for (i = page; i < (npages + page); i++) {
	  //kprintf("Marking %d pages as kernel\n", npages);
	  coremap[i].isKernel = 1;
	  
	}
	
	//kprintf("Allocated physical addr %x at kernel addr %x\n", pa, PADDR_TO_KVADDR(pa));
	
	return PADDR_TO_KVADDR(pa);
}

void 
free_kpages(vaddr_t addr)
{
	
	/*MAYBE DEALS WITH BLOCKS, DUNNO*/
	int page = addr - MIPS_KSEG0 - cm_base;
	page = page/PAGE_SIZE;
	
	//Currently a user program so this doesn't work
	if (addr < MIPS_KSEG0) {
	  kprintf("Can't free user processes currently");
	  return;
	}
	
	int spl = splhigh();
	
	int j = page;
	
	//kprintf("I think I'm freeing page %x\n", page);
	
	while (coremap[j].lastInBlock == 0) {
	  coremap[j].isAllocated = 0;
	  coremap[j].isKernel = 0;
	  coremap[j].lastInBlock = 0;
	 // /if (coremap[j].list != NULL) {
	  //  kfree(coremap[j].list);
	 // }
	  j++;
	  cm_numFree++;
	}
	
	coremap[j].isAllocated = 0;
	coremap[j].isKernel = 0;
	coremap[j].lastInBlock = 0;
	//if (coremap[j].list != NULL) {
	//    kfree(coremap[j].list);
	//}
	
	cm_numFree++;
	//kprintf("Coremap has %d free frames\n", cm_numFree);
	//kprintf("Freeing %d pages\n", j - page);
	
	splx(spl);

	(void)addr;
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	//vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	//paddr_t paddr;
	//int i;
	//u_int32_t ehi, elo;
	struct addrspace *as;
	int spl;

	spl = splhigh();

	faultaddress &= PAGE_FRAME;

	as = curthread->t_vmspace;
	if (as == NULL) {
		kprintf("as is null\n");
		return EFAULT;
	}
	
	switch (faulttype) {
	    case VM_FAULT_READONLY:
		pt_protection(curthread->t_vmspace, faultaddress);
		splx(spl);
		return 0;
		break;
	    case VM_FAULT_READ:
		//kprintf("VM_READ FAULT\n");
		pt_read(curthread->t_vmspace->as_pagetable,faultaddress);
		splx(spl);
		return 0;
		break;
	    case VM_FAULT_WRITE:
		//kprintf("VM_WRITE FAULT\n");
		pt_read(curthread->t_vmspace->as_pagetable,faultaddress);
		splx(spl);
		return 0;
		break;
	    default:
		splx(spl);
		return EINVAL;
	}


	/*
	assert(as->as_vbase1 != 0);
	assert(as->as_pbase1 != 0);
	assert(as->as_npages1 != 0);
	assert(as->as_vbase2 != 0);
	assert(as->as_pbase2 != 0);
	assert(as->as_npages2 != 0);
	assert(as->as_stackpbase != 0);
	assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
	assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
	assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;

	if (faultaddress >= vbase1 && faultaddress < vtop1) {
		paddr = (faultaddress - vbase1) + as->as_pbase1;
	}
	else if (faultaddress >= vbase2 && faultaddress < vtop2) {
		paddr = (faultaddress - vbase2) + as->as_pbase2;
	}
	else if (faultaddress >= stackbase && faultaddress < stacktop) {
		paddr = (faultaddress - stackbase) + as->as_stackpbase;
	}
	else {
		splx(spl);
		return EFAULT;
	}

	
	assert((paddr & PAGE_FRAME)==paddr);

	for (i=0; i<NUM_TLB; i++) {
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		TLB_Write(ehi, elo, i);
		splx(spl);
		return 0;
	}

	kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
	splx(spl);
	return EFAULT;*/
}

//Figure out what pages to swap
void swap_replacement(void) {
 
  
  //lock_acquire(swap_lock);
  
  int i;
  unsigned long result;
  
  //Try to find the end of the majority of the kernel frames
  for (i = 0; i < cm_numEntries; i++) {
    if (coremap[i].isKernel == 0 || coremap[i].isAllocated == 0) {
      break;  
    }
  }
  
  if (i == cm_numEntries - 1) {
    panic("Shit. Apparently every frame is a kernel frame. I DID NOT FORSEE THIS!");
  }
 
  //Keep randoming until shit works out
  while (1) {
    result = random();
    result = result % cm_numEntries;
    //% cm_numFree + i;
    if (result > cm_numEntries) {
      kprintf("math fail\n");
      //Try again..
    } else if (coremap[result].isKernel == 0 && result > 40) {
      //found our answer
      break;
    }
  }

  if (coremap[result].isAllocated == 1 && coremap[result].list == NULL) {
    panic("Apparently we managed to allocate a user frame and not put it in the inverted page table. Fail.");
  }

  if (coremap[result].isAllocated == 1) {
    writeout(result);
  }
  
  /*FLUSH TLB*/
  
  int spl = splhigh();
  
  coremap[result].isAllocated = 0;
  coremap[result].isKernel = 0;
  coremap[result].lastInBlock = 0;
 
	cm_numFree++;
	//DEBUG(DB_VM, "Flushing TLB\n");

  for (i=0; i<NUM_TLB; i++) {
	TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
  }

  splx(spl);
  
  //lock_release(swap_lock);
  
}
  
  
  
  
  