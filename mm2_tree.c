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

struct Node
{
  size_t size;
  int height;
  // int bf;
  struct Node *left;
  struct Node *right;
};

typedef struct Node node_t;
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;
node_t *root, *best_node, *best_node_parent;

int mm_init(void)
{

  //This function is called every time before each test run of the trace.
  //It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.

  root = NULL;

  /*
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 *
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */

  return 0; //Returns 0 on successfull initialization.
}

/*
void coalesce(node_t *header)
{
  if (header->free_flag == 0)
    return; //block is not free

  node_t *after = header->next;
  node_t *before = header->prev;

  if (after && before && after->free_flag && before->free_flag)
  {
    //before and after blocks are free
    size_t new_size = header->size + before->size + after->size + 2 * sizeof(node_t);
    before->size = new_size;

    before->next = after->next;
    if (after->next)
      after->next->prev = before;

    if (tail == after)
      tail = before;
  }
  else if (after && after->free_flag)
  {
    //only after block is free
    size_t new_size = header->size + after->size + sizeof(node_t);
    header->size = new_size;

    header->next = after->next;
    if (after->next)
      after->next->prev = header;

    if (tail == after)
      tail = header;
  }
  else if (before && before->free_flag)
  {
    //only before block is free
    size_t new_size = header->size + before->size + sizeof(node_t);
    before->size = new_size;

    before->next = header->next;
    if (header->next)
      header->next->prev = before;

    if (tail == header)
      tail = before;
  }
}
*/

node_t *find_bestfit(size_t size)
{
  best_node = NULL;
  best_node_parent = NULL;
  search_tree(root, NULL, size);

  if (best_node_parent != NULL)
  {
  }

  return best_node;
}

void insert_tree(node_t *node, node_t *new_node)
{
  if(new_node->size <= node->size)
  {
    if(node->left)
      insert_tree(node->left, new_node);
    else
      node->left = new_node;
  }
  else
  {
    /* code */
  }
}

void search_tree(node_t *node, node_t *parent, size_t size)
{
  if (node == NULL)
    return;

  if (node->size == size)
  {
    best_node = node;
    best_node_parent = parent;
    return;
  }

  if (node->size > size)
  {
    best_node = node;
    best_node_parent = parent;
    search_tree(node->left, node, size);
  }
  else
  {
    search_tree(node->right, node, size);
  }
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
  node_t *header = find_bestfit(size);
  if (header)
  {
    // header->free_flag = 0;
    return (void *)(header + 1);
  }

  //mem_sbrk() is wrapper function for the sbrk() system call.
  //Please use mem_sbrk() instead of sbrk() otherwise the evaluation results
  //may give wrong results
  size_t chunk_size = size + sizeof(node_t);
  void *block = mem_sbrk(chunk_size);
  if (block == (void *)-1)
  {
    return NULL;
  }

  header = block;
  header->size = size;
  header->height = 0;
  header->left = NULL;
  header->right = NULL;

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

  node_t *header;
  header = (node_t *)ptr - 1;
  header->height = 1;

  if (root)
    insert_tree(root, header);
  else
    root = header;

  // coalesce(header);
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

  node_t *header;
  header = (node_t *)ptr - 1;
  size_t prev_size = header->size;

  if (size == prev_size)
  {
    return ptr;
  }

  void *new_ptr = mm_malloc(size);

  size_t cpy_size = (prev_size < size) ? prev_size : size;

  if (new_ptr != NULL)
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
