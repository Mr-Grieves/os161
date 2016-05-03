#include <types.h>
#include <coremap.h>
#include <lib.h>
#include <kern/unistd.h>
#include <machine/spl.h>
#include <machine/pcb.h>
#include <curthread.h>
#include <kern/errno.h>
#include <thread.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <vm.h>
#include <addrspace.h>

#define ASSUME_512 1

void cm_bootstrap(void) {
  
  allocatingKernel = 1;
  
  int i, bitsNeeded, firstAddress, lastAddress;
  
  firstAddress = cm_firstAddr;
  lastAddress = cm_lastAddr;
  cm_base = firstAddress;
  
  /*Everything was tested assuming 512K memory*/
  #if ASSUME_512
  if (lastAddress != 524288) {
    kprintf("\nRam size is %d. I can't currently handle this.\n", lastAddress);
    kprintf("Everything else might not work.. fix it!\n\n");
  }
  #endif

  /*Find out how many bits are needed. Should probably be 128. Round up!*/
  bitsNeeded = DIVROUNDUP(lastAddress - firstAddress, PAGE_SIZE);

  //bitmap length should be the number of elements required in the coremap
  int coreSize = bitsNeeded * sizeof(struct cm_entry);
  coremap = PADDR_TO_KVADDR(firstAddress); //Need to translate it to kernel
  firstAddress += coreSize;
  
  //Make sure our assumption was correct
  if (firstAddress >= cm_firstAddr + 4096) {
    panic("Stole more than a page of memory? Waaah");
  }
  
  //Initialze a bunch of fields
  cm_numEntries = bitsNeeded;
  cm_numKern = 1;
  cm_numUser = 0;
  cm_numFree = cm_numEntries - 1; //Stole one for the coremap and bitmap
  
  /*ASSUMPTION! IF CRAZY SHIT HAPPENS THIS WAS WRONG!*/
  /*SERIOUSLY, THIS IS PROBABLY A BAD IDEA*/
  ram_stealmem(1);//Actually steal that page
  
  coremap[0].isKernel = 1;
  coremap[0].isAllocated = 1;
  coremap[0].lastInBlock = 1;
  coremap[0].list = NULL;
  
  //Initialize all of the coremap entires
  for (i=1; (unsigned)i < cm_numEntries; i++) {
    coremap[i].isKernel = 0;
    coremap[i].isAllocated = 0;
    coremap[i].lastInBlock = 0;
    coremap[i].list = NULL;
  }
 
  
}

int cm_map(struct addrspace *as, unsigned page_addr, unsigned phy_addr) {
 
  int frame_number = (phy_addr-cm_base)/PAGE_SIZE;
  if (frame_number < 0 || frame_number > cm_numEntries) {
    panic("Impossibly large frame number in cm_map");
  }
  
  //kprintf("Mapping to frame %d\n", frame_number);
  
  if (coremap[frame_number].list == NULL) {
    coremap[frame_number].list = kmalloc(sizeof(struct invertedPage));
    coremap[frame_number].list->next = NULL;
    coremap[frame_number].list->as = as;
    coremap[frame_number].list->virtual_addr = page_addr;
  } else {
    struct invertedPage *temp = kmalloc(sizeof(struct invertedPage));
    temp->next = coremap[frame_number].list;
    coremap[frame_number].list = temp;
    coremap[frame_number].list->as = as;
    coremap[frame_number].list->virtual_addr = page_addr;
  }  
  
  return 1;
}


void cm_unmap(struct addrspace *as, unsigned phy_addr) {
  int frame_number = (phy_addr-cm_base)/PAGE_SIZE;
  if (frame_number < 0 || frame_number >= cm_numEntries) {
    kprintf("I think the fame is %d\n", frame_number);
    panic("Impossibly large frame number in cm_unmap");
  }
  
  //kprintf("Called unmap on %d\n", frame_number);
  
  struct invertedPage *tempPtr;
  struct invertedPage *temp;
  
  //Find us, and remove us from the list. If only one is left unmark as kernel
  if (coremap[frame_number].list == NULL) {
    panic("Called unmap on a kernel frame. This may or may not be a problem.");
  } else {
    tempPtr = coremap[frame_number].list;
    if (tempPtr->as == as) {
      kprintf("Removing shared page\n");
      coremap[frame_number].list = tempPtr->next;
      kfree(tempPtr);
    } else {
      while (tempPtr->next != NULL) {
	if ((tempPtr->next)->as == as) {
	  kprintf("Removing shared page\n");
	  temp = tempPtr->next;
	  tempPtr->next = tempPtr->next->next;
	  kfree(temp);
	}
	tempPtr = tempPtr->next;
      }
    }
  }
  
  if (coremap[frame_number].list->next == NULL) {
    kprintf("Frame no longer shared\n");
    coremap[frame_number].isKernel = 0;
    unsigned addr = coremap[frame_number].list->virtual_addr;
    (coremap[frame_number].list->as->as_pagetable->PT)[addr>>22][(addr&(0x3ff000))>>12] |= TLBLO_DIRTY;
  }
  
}

void cm_unmapall(unsigned phy_addr) {
  int frame_number = (phy_addr-cm_base)/PAGE_SIZE;
  if (frame_number < 0 || frame_number >= cm_numEntries) {
    kprintf("I think the fame is %d\n", frame_number);
    panic("Impossibly large frame number in cm_unmapall");
  }
  
  if (coremap[frame_number].list == NULL) {
    return;
  } else if (coremap[frame_number].list->next != NULL) {
    kfree(coremap[frame_number].list->next);
    kfree(coremap[frame_number].list);
  } else {
    kfree(coremap[frame_number].list);
  }
 
}



