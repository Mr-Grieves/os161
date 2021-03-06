#ifndef COREMAP_H
#define COREMAP_H

#include <bitmap.h>
#include <addrspace.h>

struct invertedPage {
  struct addrspace *as;
  unsigned virtual_addr;
  struct invertedPage *next;
};

/*Coremap array element. Have to keep it small!*/
struct cm_entry {
 
  unsigned char isKernel;//Should never swap these.. I think
  unsigned char isAllocated;
  unsigned char lastInBlock;//Used for freeing
  struct invertedPage *list;
  
};

unsigned int cm_numEntries;
unsigned int cm_numKern;
unsigned int cm_numUser;
unsigned int cm_numFree;
unsigned int cm_base;

//False after bootstrap
int allocatingKernel;

//Inverted page table
struct cm_entry *coremap;

/*Steal memory for the coremap struct and the bitmap. Going to be tricky..*/
void cm_bootstrap(void);

//Give a page table address, find and allocate a frame for it
int cm_map(struct addrspace *as, unsigned page_addr, unsigned phy_addr); 

//Unmap a page
void cm_unmap(struct addrspace *as, unsigned phy_addr);

#endif

