/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Hakaishins",
    /* First member's full name */
    "Utkarsh Indolia",
    /* First member's email address */
    "utkarshindolia@cse.iitb.ac.in",
    /* Second member's full name (leave blank if none) */
    "Prashant Ravi",
    /* Second member's email address (leave blank if none) */
    "prashantravi@cse.iitb.ac.in"};

struct header
{
  unsigned long int size;
  int free_flag;
  struct header *next;
  struct header *prev;
};

typedef struct header header_t;
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;
header_t *head, *tail;

int mm_init(void)
{

  //This function is called every time before each test run of the trace.
  //It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.

  head = NULL;
  tail = NULL;

  /*
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 *
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */

  return 0; //Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------
void coalesce(header_t *header){
  header_t *next_header, *prev_header;
  next_header = header->next;
  prev_header = header->prev;
  if(next_header && prev_header && next_header->free_flag && prev_header->free_flag)
  {
    unsigned long new_size = header->size + prev_header->size + next_header->size + 2*sizeof(header_t);
    prev_header->size = new_size;
    prev_header->next = next_header->next;
    header_t *next_next_header = next_header->next;
    if(next_next_header){
      next_next_header->prev = prev_header;
    }
  }
  else if(next_header && next_header->free_flag)
  {
      unsigned long new_size = header->size + next_header->size + sizeof(header_t);
      header->size = new_size;
      header->next = next_header->next;
      header_t *next_next_header = next_header->next;
      if(next_next_header){
        next_next_header->prev = header;
      }
  }
  else if(prev_header && prev_header->free_flag)
  {
    unsigned long new_size = header->size + prev_header->size + sizeof(header_t);
    prev_header->size = new_size;
    prev_header->next = header->next;
    header_t *next_header = header->next;
    if(next_header){
      next_header->prev = prev_header;
    }
  }
}

header_t *free_bestfit_block(size_t size)
{
  header_t *curr = head, *block = NULL;
  unsigned long int min_size = ULONG_MAX;
  while (curr)
  {
    if (curr->free_flag && curr->size >= size)
    {
      if (curr->size <= min_size)
      {
        min_size = curr->size;
        block = curr;
      }
    }
    curr = curr->next;
  }

 /*if(block!=NULL){
    long rem_space = block->size - size - sizeof(header_t);
    if(rem_space >= 8){
      //printf("Demand size:%lu\n",size);
      //printf("Before breaking, block size: %lu\nblock header start: %x, block start: %x, block end: %x\n", block->size, (char *)block, (char *)block + sizeof(header_t), (char *)block + sizeof(header_t) + block->size);
      header_t *break_header;
      break_header = (void *)((char *)block + sizeof(header_t) + size);
      break_header->size = rem_space;
      break_header->next = block->next;
      break_header->free_flag = 1;
      block->size = size;
      block->next = break_header;
      //printf("After breaking, left part block size: %lu\nblock header start: %x, block start:%x, block end: %x\n", block->size, (char *)block, (char *)block + sizeof(header_t), (char *)block + sizeof(header_t) + block->size);
      //printf("After breaking, right part block size: %lu\nblock header start: %x, block start:%x, block end: %x\n", break_header->size, (char *)break_header, (char *)break_header + sizeof(header_t), (char *)break_header + sizeof(header_t) + break_header->size);
      //printf("\n\n");
    }
  } */
  return block;
}

/*
  * mm_malloc - Allocate a block by incrementing the brk pointer.
  *     Always allocate a block whose size is a multiple of the alignment.
  */

void *mm_malloc(size_t size)
{
  /*
	 * This function should keep track of the allocated memory blocks.
	 * The block allocation should minimize the number of holes (chucks of unusable memory) in the heap memory.
	 * The previously freed memory blocks should be reused.
	 * If no appropriate free block is available then the increase the heap  size using 'mem_sbrk(size)'.
	 * Try to keep the heap size as small as possible.
	 */
  //printf("Allocating %lu\n",size);
  if (size <= 0)
  { // Invalid request size
    return NULL;
  }

  size = ((size + 7) / 8) * 8; //size alligned to 8 bytes'
  header_t *header;
  header = free_bestfit_block(size);
  if (header)
  {
    header->free_flag = 0;
    return (void *)(header + 1);
  }
  //if(header && header->next) coalesce(header->next);
  //mem_sbrk() is wrapper function for the sbrk() system call.
  //Please use mem_sbrk() instead of sbrk() otherwise the evaluation results
  //may give wrong results
  size_t chunk_size = size + sizeof(header_t);
  void *block = mem_sbrk(chunk_size);
  if (block == (void *)-1)
  {
    return NULL;
  }
  header = block;
  header->size = size;
  header->free_flag = 0;
  header->next = NULL;
  header->prev = NULL;
  if (head == NULL)
  {
    head = header;
  }
  if (tail != NULL)
  {
    tail->next = header;
    header->prev = tail;
  }
  tail = header;
  return (void *)(header + 1);
}

void mm_free(void *ptr)
{
  /*
	 * Searches the previously allocated node for memory block with base address ptr.
	 *
	 * It should also perform coalesceing on both ends i.e. if the consecutive memory blocks are
	 * free(not allocated) then they should be combined into a single block.
	 *
	 * It should also keep track of all the free memory blocks.
	 * If the freed block is at the end of the heap then you can also decrease the heap size
	 * using 'mem_sbrk(-size)'.
	 */

  if (ptr == NULL)
  {
    return;
  }

  header_t *header;
  header = (header_t *)ptr - 1;
  header->free_flag = 1;

  //printf("Freeing from location: %x, size: %lu\n", ptr, header->size);
  coalesce(header);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  //printf("Reallocating size: %lu\n",size);
  size = ((size + 7) / 8) * 8; //8-byte alignement

  if (ptr == NULL)
  { //memory was not previously allocated
    return mm_malloc(size);
  }

  if (size == 0)
  { //new size is zero
    mm_free(ptr);
    return NULL;
  }
  //printf("Reallocating address: %x\n", ptr);
  /*
	 * This function should also copy the content of the previous memory block into the new block.
	 * You can use 'memcpy()' for this purpose.
	 *
	 * The data structures corresponding to free memory blocks and allocated memory
	 * blocks should also be updated.
	*/



  header_t *header, *next_header;
  header = (header_t *)ptr - 1;
  unsigned long int prev_size = header->size;
  next_header = header->next;

  if (size == prev_size)
  {
    return ptr;
  }

  if(next_header && next_header->free_flag){
    long new_size = header->size + next_header->size + sizeof(header_t);
    if(new_size>=size){
      header->size = new_size;
      header->next = next_header->next;
      header_t *next_next_header = next_header->next;
      if(next_next_header){
        next_next_header->prev = header;
      }
      return ptr;
    }
  }

  void *new_chunk = mm_malloc(size);

  unsigned long int cpy_size = (prev_size < size) ? prev_size : size;

  if (new_chunk != NULL)
  {
    memcpy(new_chunk, ptr, cpy_size);
    mm_free(ptr);
    ptr = new_chunk;
    return ptr;
  }
  else
  {
    return NULL;
  }
}
