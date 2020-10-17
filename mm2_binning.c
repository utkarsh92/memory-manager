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
    "Prashant Ravi",
    /* First member's email address */
    "prashantravi@cse.iitb.ac.in",
    /* Second member's full name (leave blank if none) */
    "Utkarsh Indolia",
    /* Second member's email address (leave blank if none) */
    "utkarshindolia@cse.iitb.ac.in"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define NUMBER_OF_BINS 11

typedef struct header
{
  unsigned long int size;
  int free;
  struct header *next;
  struct header *prev;
} header_t;

typedef struct footer{
	header_t* header;
	int free;
} footer_t;


/*
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;

header_t *bins[NUMBER_OF_BINS];
int overhead = sizeof(header_t) + sizeof(footer_t);
header_t *inital_pg_break, *curr_pg_break;

int mm_init(void)
{
	//printf("New trace starts\n");

	//This function is called every time before each test run of the trace.
	//It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.

	int i=0;
	for(i=0;i<NUMBER_OF_BINS;i++){
		bins[i] = NULL;
	}
	inital_pg_break = mem_sbrk(0);
	curr_pg_break = inital_pg_break;
	////printf("Initial program break: %u\n",(unsigned int)inital_pg_break);
	/*
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 *
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */

    return 0;		//Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------

int get_index(size_t size){
	if(size!=65) size--;
	int index=0;
	size = size < 64 ? 64 : size;
  if(size>64) index=1;
	while(size/2){
		index++;
		size = size/2;
	}
	return (index-6 < NUMBER_OF_BINS)? index-6 : NUMBER_OF_BINS-1;
}



header_t *get_chunk(header_t *binhead, size_t size){
	header_t *iter = binhead;
	while(iter){
		if(iter->size >= size){
			return iter;
		}
		iter = iter->next;
	}
	return iter;
}

footer_t *get_footer(header_t *header) {
    return (footer_t *)((char *)header + header->size + sizeof(header_t));
}

footer_t *create_footer(header_t *header, footer_t *footer) {
    //footer_t *footer = get_footer(header);
    footer->header = header;
		footer->free = header->free;
		return footer;
}

header_t *add_chunk(header_t *binhead, header_t *chunk){
	//printf("###add_chunk me aa rha h\n");
	if(binhead == NULL){
		//printf("###head pe add hua\n");
		binhead = chunk;
		return binhead;
	}
	header_t *curr = binhead;
	header_t *prev = NULL;

	while(curr && curr->size <= chunk->size){
		prev = curr;
		curr = curr->next;
	}

	if(curr == NULL){
		//printf("###curr == null me add hua\n");
		prev->next = chunk;
		chunk->prev = prev;
	}
	else if(prev){
		//printf("###prev me add hua\n");
		chunk->next = curr;
		chunk->prev = prev;
		curr->prev = chunk;
		prev->next = chunk;
	}
	else{
		//printf("###last else me add hua\n");
		chunk->next = binhead;
		binhead->prev = chunk;
		binhead = chunk;
	}
	return binhead;
}

header_t *remove_chunk(header_t *binhead, header_t *chunk){
	if(binhead == NULL)
		return NULL;
	if(binhead == chunk){
		binhead = binhead->next;
		return binhead;
	}

	header_t *iter = binhead->next;
	while(iter){
		if(iter == chunk){
			if(iter->next == NULL){
				iter->prev->next = NULL;
			}
			else{
				iter->prev->next = iter->next;
				iter->next->prev = iter->prev;
			}
			return binhead;
		}
		iter = iter->next;
	}
	return binhead;
}

header_t *divide_chunk(header_t *chunk, size_t size){
	long int rem_space = chunk->size - size;
	if(rem_space >= 32 + overhead){
		header_t *break_header = (header_t *)((char *)chunk + size + overhead);
		break_header->size = rem_space - overhead;
		break_header->free = 1;
		//printf("Selected chunk size: %lu, Allocating rem_space: %ld\n",chunk->size, break_header->size);
		chunk->size = size;
		footer_t *chunk_footer = NULL, *break_footer = NULL;
		chunk_footer = get_footer(chunk);
		break_footer = get_footer(break_header);
		chunk_footer = create_footer(chunk, chunk_footer);
		break_footer = create_footer(break_header, break_footer);
		//if(chunk_footer->header == chunk){
			//printf("...Chunk k liye footer kaam kr rha h\n");
	//	}
	//	if(break_footer->header == break_header){
			//printf("...Break_header k liye footer kaam kr rha h\n");
	//	}
		break_header->next = NULL;
		break_header->prev = NULL;
		bins[get_index(break_header->size)] = add_chunk(bins[get_index(break_header->size)],break_header);
	}
	return chunk;
}

void printbins(){
	//printf("New printbin call from here\n");
	int i=0;
	for(i=0;i<NUMBER_OF_BINS;i++){
		//printf("Bin %d from here\n", i);
		header_t *iter = bins[i];
		while(iter){
			//printf("Chunk start: %u, Chunk size: %lu, Chunk free: %d, Chunk next: %u, Chunk prev: %u, Chunk end: %u\n",(unsigned int)iter, iter->size, iter->free, (unsigned int)iter->next, (unsigned int)iter->prev,(unsigned int)get_footer(iter)+sizeof(footer_t));
			iter = iter->next;
		}
		//printf("Bin Change\n");
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

	if(size <= 0){		// Invalid request size
		return NULL;
	}
	////printf("size of header: %u\nsize of footer: %u\n",sizeof(header_t),sizeof(footer_t));
	//printf("Allocating %u\n",size);
	size = ((size+7)/8)*8;		//size alligned to 8 bytes
	size_t tot_size = size + overhead;
	int bin = get_index(size);
	////printf("size: %u\tindex: %d\n",size,bin);
	header_t *chunk = NULL;
	while(chunk == NULL){
		if(bin>NUMBER_OF_BINS-1)
			break;
		chunk = get_chunk(bins[bin],size);
		bin++;
	}
	bin--;
	if(chunk){
		chunk->free = 0;
		chunk = divide_chunk(chunk,size);
		bins[bin] = remove_chunk(bins[bin],chunk);
		chunk->next = NULL;
		chunk->prev	= NULL;
		//printbins();
		return (void *)(chunk + 1);
	}

	void *block = mem_sbrk(tot_size);
	if (block == (void *)-1)
	{
		return NULL;
	}
	chunk = block;
	chunk->size = size;
	chunk->free = 0;
	chunk->next = NULL;
	chunk->prev = NULL;
	footer_t *chunk_footer = NULL;
	chunk_footer = get_footer(chunk);
	chunk_footer = create_footer(chunk, chunk_footer);
	if(chunk_footer->header == chunk){
		//printf("...Chunk (niche waala) k liye footer kaam kr rha h\n");
	}
	////printf("Addresses are: %u\ttot_size: %u\n",(unsigned int)chunk,tot_size);
	curr_pg_break = (header_t *)((unsigned int)chunk + tot_size);
	////printf("Current Program Break: %u\n",(unsigned int)curr_pg_break);
	//printbins();
	return (void *)(chunk + 1);
	//mem_sbrk() is wrapper function for the sbrk() system call.
	//Please use mem_sbrk() instead of sbrk() otherwise the evaluation results
	//may give wrong results
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
	 if(ptr == NULL){
		 return;
	 }
	 header_t *header = (header_t *)ptr - 1;
	 //printf("Freeing from %u, size %lu\n",(unsigned int)header, header->size);
	 header_t *next_in_mem = (header_t *)((char *)get_footer(header) + sizeof(footer_t));
	 if((unsigned int)next_in_mem >= (unsigned int)curr_pg_break){
		 next_in_mem = NULL;
	 }
	 header_t *prev_in_mem = NULL;
	 footer_t *prev_f = (footer_t *)((char *)header - sizeof(footer_t));
	 if((unsigned int)prev_f <= (unsigned int)inital_pg_break){
		 prev_f = NULL;
	 }
	 if(prev_f){
		prev_in_mem = prev_f->header;
		if((unsigned int)prev_in_mem < (unsigned int)inital_pg_break){
 		 prev_in_mem = NULL;
 	 	}
	 }
	 footer_t *footer = NULL;
	 if(prev_in_mem && prev_in_mem->free){
		 bins[get_index(prev_in_mem->size)] = remove_chunk(bins[get_index(prev_in_mem->size)],prev_in_mem);
		 prev_in_mem->size += header->size + overhead;
		 footer = get_footer(header);
		 footer->header = prev_in_mem;
		 footer->free = 1;
		 prev_in_mem->free = 1;
		 header = prev_in_mem;
	 }

	 if(next_in_mem && next_in_mem->free){
		 bins[get_index(next_in_mem->size)] = remove_chunk(bins[get_index(next_in_mem->size)],next_in_mem);
		 header->size += next_in_mem->size + overhead;
		 footer = get_footer(next_in_mem);
		 footer->header = header;
		 footer->free = 1;
	 }
	 memset((void*)(header + 1), 0, header->size);
	 header->free = 1;
	 header->next = NULL;
	 header->prev = NULL;
	 bins[get_index(header->size)] = add_chunk(bins[get_index(header->size)],header);
	 //printbins();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	size = ((size+7)/8)*8; //8-byte alignement

	if(ptr == NULL){			//memory was not previously allocated
		return mm_malloc(size);
	}

	if(size == 0){				//new size is zero
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

	header_t *header;
  header = (header_t *)ptr - 1;
	//printf("Reallocating from %u, size %lu\n",(unsigned int)header, header->size);
  unsigned long int prev_size = header->size;

  if (size == prev_size)
  {
    return ptr;
  }

	if(size<prev_size){
		header = divide_chunk(header,size);
		return ptr;
	}

	if(size>prev_size){
	 header_t *next_in_mem = (header_t *)((char *)get_footer(header) + sizeof(footer_t));
 	 if((unsigned int)next_in_mem >= (unsigned int)curr_pg_break){
 		 next_in_mem = NULL;
 	 }

	 if(next_in_mem && next_in_mem->free){
		unsigned long int tot_size = header->size + next_in_mem->size + overhead;
		if(tot_size>size){
			bins[get_index(next_in_mem->size)] = remove_chunk(bins[get_index(next_in_mem->size)],next_in_mem);
			header->size = tot_size;
			footer_t *footer = get_footer(next_in_mem);
			footer->header = header;
			footer->free = 0;
			return ptr;
		}
	 }
	}

  void *new_chunk = mm_malloc(size);

  unsigned long int cpy_size = (prev_size < size) ? prev_size : size;

  if (new_chunk != NULL)
  {
    memcpy(new_chunk, ptr, cpy_size);
    mm_free(ptr);
    ptr = new_chunk;
		//printbins();
    return ptr;
  }
  else
  {
    return NULL;
  }

}
