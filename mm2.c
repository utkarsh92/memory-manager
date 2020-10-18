/*
 * mm-2.c - The fastest, most memory-efficient malloc package.
 *
 * In this approach, we are maintaining an explicit free list instead of implicit
 * which saves us a lot of time wasted during free block searching. Also the header
 * packaging has also been optimized so that it can hold more information without
 * increasing it's space.
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
  size_t size; //holds size and free status in lsb
  struct header *prev_real;
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
header_t *head, *tail, *last_block;

int mm_init(void)
{
  //This function is called every time before each test run of the trace.
  //It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.

  head = NULL;
  tail = NULL;
  last_block = NULL;

  /*
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 *
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */

  return 0; //Returns 0 on successfull initialization.
}

//get free bit value
size_t get_free(size_t size)
{
  return (size & 1);
}

//set free bit to 1
size_t set_free(size_t size)
{
  return (size | 1);
}

//get block size OR set free bit to 0
size_t get_size(size_t size)
{
  return (size & -2);
}

//removes block from current explicit list
//updates head and tail as well
void delete_free(header_t *block)
{
  if (!get_free(block->size))
    return;

  if (block->prev)
    block->prev->next = block->next;
  else
    head = block->next;

  if (block->next)
    block->next->prev = block->prev;
  else
    tail = block->prev;

  block->size = get_size(block->size);
  block->next = NULL;
  block->prev = NULL;
}

//adds block to explicit free list
void add_free_block(header_t *block)
{
  //setting current blk free
  block->size = set_free(block->size);
  block->next = NULL;
  block->prev = NULL;

  if (head == NULL)
  {
    head = block;
    tail = block;
    return;
  }

  //REAL before and after
  header_t *before = block->prev_real;
  header_t *after = (header_t *)((char *)block + get_size(block->size));

  if (before && get_free(before->size) && (after <= last_block) && get_free(after->size))
  {
    //both sides free
    before->size += get_size(block->size) + get_size(after->size);

    delete_free(after);

    //within real block list
    header_t *after_after = (header_t *)((char *)after + get_size(after->size));
    if (after_after <= last_block)
      after_after->prev_real = before;
    else
      last_block = before;
  }
  else if (before && get_free(before->size))
  {
    //only before free
    before->size += get_size(block->size);

    //within real block list
    if (after <= last_block)
      after->prev_real = before;
    else
      last_block = before;
  }
  else if ((after <= last_block) && get_free(after->size))
  {
    //only after
    block->size += get_size(after->size);
    block->next = after->next;
    block->prev = after->prev;

    //within free list
    if (after->prev)
      after->prev->next = block;
    else
      head = block;

    if (after->next)
      after->next->prev = block;
    else
      tail = block;

    //within real block list
    header_t *after_after = (header_t *)((char *)after + get_size(after->size));
    if (after_after <= last_block)
      after_after->prev_real = block;
    else
      last_block = block;
  }
  else
  {
    //simply add to the explicit free list
    tail->next = block;
    block->prev = tail;
    tail = block;
  }
}

//attempts to break/split/fragment block
//and add the newly created free block to list, if any
int break_block(header_t *block, size_t new_size)
{
  ssize_t rem_space = get_size(block->size) - new_size - sizeof(header_t);

  if ((rem_space % ALIGNMENT == 0) && (rem_space >= ALIGNMENT))
  {
    header_t *after = (header_t *)((char *)block + get_size(block->size));

    if (get_free(block->size))
      block->size = set_free(new_size);
    else
      block->size = new_size;

    header_t *free_blk = (void *)((char *)block + new_size);
    free_blk->size = sizeof(header_t) + rem_space;
    free_blk->prev_real = block;

    if (last_block == block)
      last_block = free_blk;
    else
      after->prev_real = free_blk;

    add_free_block(free_blk);
    return 1;
  }

  return 0;
}

//searches for free block using bestfit
//breaks block into 2, if free space is available after satisfying
//user size request
header_t *free_bestfit_block(size_t size)
{
  if (!head)
    return NULL;

  header_t *curr = head, *block = NULL;
  size_t min_size = ULONG_MAX;
  size_t curr_size;

  while (curr)
  {
    curr_size = get_size(curr->size); //get size w/o free bit

    if (get_free(curr->size) && (size <= curr_size))
    {
      if (curr_size <= min_size)
      {
        min_size = curr_size;
        block = curr;
      }
    }
    curr = curr->next;
  }

  if (block)
  {
    delete_free(block);
    break_block(block, size);
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
  if (size <= 0)
  { // Invalid request size
    return NULL;
  }

  size = ((size + 7) / 8) * 8; //size alligned to 8 bytes'
  size_t blk_size = size + sizeof(header_t);

  header_t *block = free_bestfit_block(blk_size);
  if (block)
  {
    return (void *)(block + 1);
  }

  //mem_sbrk() is wrapper function for the sbrk() system call.
  //Please use mem_sbrk() instead of sbrk() otherwise the evaluation results
  //may give wrong results

  void *temp = mem_sbrk(blk_size);
  if (temp == (void *)-1)
  {
    return NULL;
  }

  block = (header_t *)temp;
  block->size = get_size(blk_size);
  block->next = NULL;
  block->prev = NULL;

  block->prev_real = last_block;
  last_block = block;

  return (void *)(block + 1);
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

  header_t *block = (header_t *)ptr - 1;
  add_free_block(block);
}

void *create_new_copy(void *ptr, size_t prev_size, void *new_ptr, size_t size)
{
  size_t cpy_size = (prev_size < size) ? prev_size : size;

  if (new_ptr)
  {
    memcpy(new_ptr, ptr, cpy_size);
    mm_free(ptr);
    return new_ptr;
  }
  else
  {
    return NULL;
  }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
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
  /*
	 * This function should also copy the content of the previous memory block into the new block.
	 * You can use 'memcpy()' for this purpose.
	 *
	 * The data structures corresponding to free memory blocks and allocated memory
	 * blocks should also be updated.
	*/

  header_t *block;
  block = (header_t *)ptr - 1;
  size_t prev_size = block->size - sizeof(header_t);

  if (size <= prev_size)
  {
    return ptr;
  }

  header_t *after = (header_t *)((char *)block + get_size(block->size));
  if ((after <= last_block) && get_free(after->size) && (size <= (prev_size + get_size(after->size))))
  {
    delete_free(after);
    block->size += get_size(after->size);

    header_t *after_after = (header_t *)((char *)after + get_size(after->size));
    if (after_after <= last_block)
      after_after->prev_real = block;
    else
      last_block = block;

    return ptr;
  }

  //nothing worked, create new blk
  void *new_ptr = mm_malloc(size);
  size_t cpy_size = (prev_size < size) ? prev_size : size;

  if (new_ptr)
  {
    memcpy(new_ptr, ptr, cpy_size);
    mm_free(ptr);
    return new_ptr;
  }
  else
  {
    return NULL;
  }
}
