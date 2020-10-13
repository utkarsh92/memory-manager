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

struct tag
{
  unsigned long size; //size of payload
  int free_flag;
  struct tag *next;
  char waste[4]; //To pad to 16 bytes
};

typedef struct tag tag_t;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;
tag_t *head_header, *head_footer, *tail_header, *tail_footer;

int mm_init(void)
{

  //This function is called every time before each test run of the trace.
  //It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.

  head_header = NULL;
  head_footer = NULL;
  tail_header = NULL;
  tail_footer = NULL;

  /*
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 *
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */

  return 0; //Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------

tag_t *free_bestfit_block(size_t size)
{
  tag_t *curr = head_header, *block = NULL;
  unsigned long min_size = ULONG_MAX;
  while (curr)
  {
    //printf("Going on forever\n");
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
  //Fragmentation to save memory
  if(block!=NULL){
    long rem_space = block->size - size - 2*sizeof(tag_t);
    //rem_space = ((rem_space + 7) / 8) * 8; //size alligned to 8 bytes'
    if(rem_space >= 8){
      printf("Demand size:%lu\n",size);
      printf("Before breaking, block size: %lu\nblock header start: %x, block start:%x, block end: %x, block footer end: %x\n", block->size, (char *)block, (char *)block + sizeof(tag_t), (char *)block + sizeof(tag_t) + block->size, (char *)block + 2*sizeof(tag_t) + block->size);
      tag_t *break_footer, *break_header, *block_footer;
      break_footer = (void *)((char *)block + sizeof(tag_t) + size);
      break_footer->size = size;
      break_header = (void *)((char *)block + 2*sizeof(tag_t) + size);
      break_header->size = rem_space;
      block_footer = (void *)((char *)block + sizeof(tag_t) + block->size);
      break_footer->next = block_footer->next;
      break_header->next = block->next;
      block->size = size;
      block_footer->size = rem_space;
      if(block->next != NULL){
        printf("Chutiya bnaya bda mja aaya\n");
        block->next = break_header;
      }
      if(block_footer->next != NULL){
        printf("Chutiya bnaya bda mja aaya part 2\n");
        block_footer->next = break_footer;
      }

      printf("After breaking, left part block size: %lu\nblock header start: %x, block start:%x, block end: %x, block footer end: %x\n", block->size, (char *)block, (char *)block + sizeof(tag_t), (char *)block + sizeof(tag_t) + block->size, (char *)block + 2*sizeof(tag_t) + block->size);
      printf("After breaking, right part block size: %lu\nblock header start: %x, block start:%x, block end: %x, block footer end: %x\n", break_header->size, (char *)break_header, (char *)break_header + sizeof(tag_t), (char *)break_header + sizeof(tag_t) + break_header->size, (char *)break_header + 2*sizeof(tag_t) + break_header->size);
      printf("\n\n");
    }
  }
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

   printf("Allocating %lu\n",size);

  if (size <= 0)
  { // Invalid request size
    return NULL;
  }

  size = ((size + 7) / 8) * 8; //size alligned to 8 bytes'

  tag_t *header, *footer;
  printf("Enter here\n");
  header = free_bestfit_block(size);
  if (header)
  {
    header->free_flag = 0;

    footer = (void *)((char *)header + sizeof(tag_t) + header->size);
    footer->free_flag = 0;

    return (void *)(header + 1);
  }
  printf("Exit here\n");
  //mem_sbrk() is wrapper function for the sbrk() system call.
  //Please use mem_sbrk() instead of sbrk() otherwise the evaluation results
  //may give wrong results
  size_t chunk_size = size + 2 * sizeof(tag_t);
  void *block = mem_sbrk(chunk_size);

  if (block == (void *)-1)
  {
    return NULL;
  }

  header = block;
  footer = (void *)((char *)block + sizeof(tag_t) + size); //(char*) so that we can increment byte by byte
  header->size = size;
  footer->size = size;
  header->free_flag = 0;
  footer->free_flag = 0;
  header->next = NULL;
  footer->next = NULL;

  if (head_header == NULL)
  {
    //only gets updated once after init
    head_header = header;
    head_footer = footer;
  }

  if (tail_header != NULL)
  {
    tail_header->next = header;
    footer->next = tail_footer;
  }

  tail_header = header;
  tail_footer = footer;

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

  tag_t *header, *footer;
  header = (tag_t *)ptr - 1;
  footer = (void *)((char *)ptr + header->size);
  header->free_flag = 1;
  footer->free_flag = 1;

  printf("Freeing from location: %x, size: %lu\n", ptr, header->size);
  tag_t *before = footer->next; //points to prev block footer
  tag_t *after = header->next;  //points to next block header
  printf("before: %x, after: %x\n", before, after);
  if (before && after && (before->free_flag == 1) && (after->free_flag == 1))
  {
    //printf("free block on both ends\n");
    //free blocks on both ends
    tag_t *before_header = (void *)((char *)before - before->size - sizeof(tag_t));
    tag_t *after_footer = (void *)((char *)after + sizeof(tag_t) + after->size);
    unsigned long new_size = before->size + after->size + header->size + 4 * sizeof(tag_t);

    before_header->next = after->next;
    after_footer->next = before->next;
    before_header->size = after_footer->size = new_size;

    //new current header and footer
    header = before_header;
    footer = after_footer;
  }
  else
  {
    if (before && (before->free_flag == 1))
    {
      //printf("free block only before\n");
      //free block only before
      unsigned long new_size = footer->size + before->size + 2 * sizeof(tag_t);
      footer->next = before->next;
      footer->size = new_size;

      tag_t *before_header = (void *)((char *)before - before->size - sizeof(tag_t));
      before_header->next = header->next;
      before_header->size = new_size;

      //new current header
      header = before_header;
    }
    else if (after && (after->free_flag == 1))
    {
      //printf("free block only after\n");
      //free block only after
      unsigned long new_size = header->size + after->size + 2 * sizeof(tag_t);
      header->next = after->next;
      header->size = new_size;

      tag_t *after_footer = (void *)((char *)after + sizeof(tag_t) + after->size);
      after_footer->next = footer->next;
      after_footer->size = new_size;

      //new current footer
      footer = after_footer;
    }
  }
  printf("Exiting free\n");
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  printf("Reallocating size: %lu\n",size);
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

  printf("Reallocating address: %x\n", ptr);

  /*
	 * This function should also copy the content of the previous memory block into the new block.
	 * You can use 'memcpy()' for this purpose.
	 *
	 * The data structures corresponding to free memory blocks and allocated memory
	 * blocks should also be updated.
	*/
  tag_t *header;
  header = (tag_t *)ptr - 1;
  unsigned long prev_size = header->size;

  if (size == prev_size)
  {
    return ptr;
  }

  void *new_chunk = mm_malloc(size);
  unsigned long cpy_size = (prev_size < size) ? prev_size : size;
  if (new_chunk != NULL)
  {
    memcpy(new_chunk, ptr, cpy_size);
    mm_free(ptr);
    return new_chunk;
  }
  else
  {
    return NULL;
  }
}
