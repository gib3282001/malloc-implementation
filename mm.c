// This adds coalescing of free blocks.
// Improves performance to 54/100 ... takes less time.

/*-------------------------------------------------------------------
 *  Malloc Lab Starter code:
 *        single doubly-linked free block list with LIFO policy
 *        with support for coalescing adjacent free blocks
 *
 * Terminology:
 * o We will implement an explicit free list allocator.
 * o We use "next" and "previous" to refer to blocks as ordered in
 *   the free list.
 * o We use "following" and "preceding" to refer to adjacent blocks
 *   in memory.
 *-------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/* Macros for unscaled pointer arithmetic to keep other code cleaner.
   Casting to a char* has the effect that pointer arithmetic happens at
   the byte granularity (i.e. POINTER_ADD(0x1, 1) would be 0x2).  (By
   default, incrementing a pointer in C has the effect of incrementing
   it by the size of the type to which it points (e.g. Block).)
   We cast the result to void* to force you to cast back to the
   appropriate type and ensure you don't accidentally use the resulting
   pointer as a char* implicitly.
*/
#define UNSCALED_POINTER_ADD(p, x) ((void*)((char*)(p) + (x)))
#define UNSCALED_POINTER_SUB(p, x) ((void*)((char*)(p) - (x)))


/******** FREE LIST IMPLEMENTATION ***********************************/


/* An BlockInfo contains information about a block, including the size
as well as pointers to the next and previous blocks in the free list.
 This is similar to the "explicit free list" structure illustrated in
 the lecture slides.

 Note that the next pointer are only needed when the block is free. To
 achieve better utilization, mm_malloc should use the space for next as
 part of the space it returns.

 +--------------+
 |     size     |  <-  Block pointers in free list point here
 |              |
 |   (header)   |
 |              |
 |     prev     |
 +--------------+
 |   nextFree   |  <-  Pointers returned by mm_malloc point here
 |   prevFree   |
 +--------------+      (allocated blocks do not have a 'nextFree' field)
 |  space and   |      (this is a space optimization...)
 |   padding    |
 |     ...      |      Free blocks write their nextFree/prevFree pointers in
 |     ...      |      this space.
 +--------------+

*/
typedef struct _BlockInfo {
// Size of the block and whether or not the block is in use or free.
// When the size is negative, the block is currently free.
long int size;
// Pointer to the previous block in the list.
struct _Block* prev;
} BlockInfo;

/* A FreeBlockInfo structure contains metadata just for free blocks.
* When you are ready, you can improve your naive implementation by
* using these to maintain a separate list of free blocks.
*
* These are "kept" in the region of memory that is normally used by
* the program when the block is allocated. That is, since that space
* is free anyway, we can make good use of it to improve our malloc.
*/
typedef struct _FreeBlockInfo {
  // Pointer to the next free block in the list.
  struct _Block* nextFree;
  // Pointer to the previous free block in the list.
  struct _Block* prevFree;
} FreeBlockInfo;

/* This is a structure that can serve as all kinds of nodes.
 */
typedef struct _Block {
  BlockInfo info;
  FreeBlockInfo freeNode;
} Block;

/* Pointer to the first FreeBlockInfo in the free list, the list's head. */
static Block* free_list_head = NULL;
static Block* malloc_list_tail = NULL;

static size_t heap_size = 0;

/* Size of a word on this architecture. */
#define WORD_SIZE sizeof(void*)

/* Alignment of blocks returned by mm_malloc.
 * (We need each allocation to at least be big enough for the free space
 * metadata... so let's just align by that.)  */
#define ALIGNMENT (sizeof(FreeBlockInfo))

/* This function will have the OS allocate more space for our heap.
 *
 * It returns a pointer to that new space. That pointer will always be
 * larger than the last request and be continuous in memory.
 */
void* requestMoreSpace(size_t reqSize);

/* This function will get the first block or returns NULL if there is not
 * one.
 *
 * You can use this to start your through search for a block.
 */
Block* first_block();

/* This function will get the adjacent block or returns NULL if there is not
 * one.
 *
 * You can use this to move along your malloc list one block at a time.
 */
Block* next_block(Block* block);

/* Use this function to print a thorough listing of your heap data structures.
 */
void examine_heap();

/* Checks the heap for any issues and prints out errors as it finds them.
 *
 * Use this when you are debugging to check for consistency issues. */
int check_heap();

Block* searchList(size_t reqSize) {
  Block* ptrFreeBlock = first_block();
  long int checkSize = -reqSize;

  // ptrFreeBlock will point to the beginning of the memory heap!
  // end will point to the end of the memory heap.
  //
  // You want to go through every block until you hit the end.
  // Make sure you read the explanation for the next_block function above.
  // It should come in handy

  while(ptrFreeBlock && ptrFreeBlock->info.size > checkSize){
        ptrFreeBlock = next_block(ptrFreeBlock);
  }

  if(!ptrFreeBlock){
        return NULL;
  }
  // To begin, you can ignore the free list and just go through every single
  // block in your memory looking for a free space big enough.
  //
  // Return NULL when you cannot find any available node big enough.

  return ptrFreeBlock;
}

/* Find a free block of at least the requested size in the free list.  Returns
 NULL if no free block is large enough. */
Block* searchFreeList(size_t reqSize) {
Block* ptrFreeBlock = free_list_head;
long int checkSize = -reqSize;

while(ptrFreeBlock && ptrFreeBlock->info.size > checkSize){
      ptrFreeBlock = ptrFreeBlock->freeNode.nextFree;
}

if(!ptrFreeBlock){
      return NULL;
}

// When you are ready, you can implement the free list.

return ptrFreeBlock;
}

// TOP-LEVEL ALLOCATOR INTERFACE ------------------------------------

/* Allocate a block of size size and return a pointer to it. If size is zero,
* returns null.
*/
void* mm_malloc(size_t size) {
Block* ptrFreeBlock = NULL;
long int reqSize;

// Zero-size requests get NULL.
if (size == 0) {
  return NULL;
}

// Determine the amount of memory we want to allocate
reqSize = size;

// Round up for correct alignment
reqSize = ALIGNMENT * ((reqSize + ALIGNMENT - 1) / ALIGNMENT);

  // You can change or remove the declarations
  // above.  They are included as minor hints.

  // Implement mm_malloc.  You can change or remove any of the above
  // code.  It is included as a suggestion of where to start.
  //
  // Remember to maintain your free_list_head
  //
  // You will want to replace this return statement...

  // When you are ready to implement a free list, remove the searchList call
  // and uncomment the searchFreeList call below it.
 // ptrFreeBlock = searchList(reqSize);
  ptrFreeBlock = searchFreeList(reqSize);

  //Request more space if ptrFreeBlock is null
 //We don't split the block in this case
  if(ptrFreeBlock == NULL){
        ptrFreeBlock = requestMoreSpace(reqSize + sizeof(BlockInfo));
        ptrFreeBlock->info.prev = malloc_list_tail;
        ptrFreeBlock->info.size = reqSize;
        malloc_list_tail = ptrFreeBlock;
        //return pointer to the user data. ptr to header + meta data = ptr to user data
        return (void*)UNSCALED_POINTER_ADD(ptrFreeBlock, (sizeof(BlockInfo)));
  }

  //Splitting the block:
  long int actualSize = ((long int)0 - ptrFreeBlock->info.size);
  //We only split the block if the size of the block is more than or equal to the size of the metadata after subtracting reqSize from it
  if((actualSize - reqSize) >= (sizeof(Block))){
        //freeBlock = extra space being taken up by the new block
        Block* freeBlock = (Block*)(((char*)ptrFreeBlock) + sizeof(BlockInfo) + reqSize);
        freeBlock->info.prev = ptrFreeBlock;

        //ptrFreeBlock->info.size is negative, so freeBlock's size will be the difference between (reqSize + metadata size) and the size of the new block
        freeBlock->info.size = ptrFreeBlock->info.size + (reqSize + sizeof(BlockInfo));
        insert_head_freeList(freeBlock);

        //adjust size of new block to the actual required size
        ptrFreeBlock->info.size = reqSize;
        remove_node_freeList(ptrFreeBlock);

         Block* nxt = next_block(freeBlock);
         //nxt == NULL if ptrFreeblock points to the tail
         if(malloc_list_tail == ptrFreeBlock){
                 //nxt == NULL so only adjust the tail which is ptrFreeBlock in this case
                 malloc_list_tail = freeBlock;
         }else{
                 //adjust prev pointer if there is a next block after the new block
                 nxt->info.prev = freeBlock;
         }
         return (void*)UNSCALED_POINTER_ADD(ptrFreeBlock, (sizeof(BlockInfo)));
   }

   //General case: append new Block to tail of list
   ptrFreeBlock->info.size = actualSize;
   remove_node_freeList(ptrFreeBlock);
   return (void*)UNSCALED_POINTER_ADD(ptrFreeBlock, (sizeof(BlockInfo)));
 }

 void coalesce(Block* blockInfo) {
   Block* nextBlock = next_block(blockInfo);
   Block* previousBlock = blockInfo->info.prev;
   Block* tmpBlock = NULL;

   if(previousBlock){
         if(previousBlock->info.size < 0){
                 previousBlock->info.size -= (-(blockInfo->info.size) + sizeof(BlockInfo));
                 blockInfo->info.prev = previousBlock->info.prev;
                 if(nextBlock){
                         nextBlock->info.prev = previousBlock;
                 }else{
                         malloc_list_tail = previousBlock;
                 }
                 remove_node_freeList(blockInfo);
                 blockInfo = previousBlock;
         }
   }
   if(nextBlock && nextBlock->info.size < 0){
         blockInfo->info.size -= (-(nextBlock->info.size) + sizeof(BlockInfo));
         tmpBlock = next_block(nextBlock);
         if(tmpBlock){
                 tmpBlock->info.prev = blockInfo;
         }else{
                 malloc_list_tail = blockInfo;
         }
         nextBlock->info.prev = blockInfo->info.prev;
         remove_node_freeList(nextBlock);
         nextBlock = blockInfo;
   }
   // You can change or remove the declarations
   // above.  They are included as minor hints.
 }

 void insert_head_freeList(Block* blockInfo){
         //base case: nothing is in the free list
         if(free_list_head == NULL){
                 free_list_head = blockInfo;
                 free_list_head->freeNode.nextFree = NULL;
                 free_list_head->freeNode.prevFree = NULL;
         }else{
                 //general case: perform insert head operation on free list
                 blockInfo->freeNode.nextFree = free_list_head;
                 free_list_head->freeNode.prevFree = blockInfo;
                 free_list_head = blockInfo;
                 free_list_head->freeNode.prevFree = NULL;
         }
 }
 void remove_node_freeList(Block* blockInfo){
         if(free_list_head != NULL){
                 if(blockInfo->freeNode.nextFree){
                         blockInfo->freeNode.nextFree->freeNode.prevFree = blockInfo->freeNode.prevFree;
                 }
                 if(blockInfo->freeNode.prevFree){
                         blockInfo->freeNode.prevFree->freeNode.nextFree = blockInfo->freeNode.nextFree;
                 }
                 if(free_list_head == blockInfo){
                         free_list_head = blockInfo->freeNode.nextFree;
                 }
         }
 }

 /* Free the block referenced by ptr. */
 void mm_free(void* ptr) {
   Block* blockInfo = (Block*)UNSCALED_POINTER_SUB(ptr, sizeof(BlockInfo));

   blockInfo->info.size = (long int)0 - blockInfo->info.size;

   // Remember to maintain your free_list_head
   //Base case: if free_list_head == Null the free list hasn't been initialized yet
   insert_head_freeList(blockInfo);


   // When you are ready... you will want to implement coalescing:
   coalesce(blockInfo);
 }

 // PROVIDED FUNCTIONS -----------------------------------------------
 //
 // You do not need to modify these, but they might be helpful to read
 // over.

 /* Get more heap space of exact size reqSize. */
 void* requestMoreSpace(size_t reqSize) {
   void* ret = UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
   heap_size += reqSize;

   void* mem_sbrk_result = mem_sbrk(reqSize);
   if ((size_t)mem_sbrk_result == -1) {
     printf("ERROR: mem_sbrk failed in requestMoreSpace\n");
     exit(0);
   }

   return ret;
 }

 /* Initialize the allocator. */
 int mm_init() {
   free_list_head = NULL;
   malloc_list_tail = NULL;
   heap_size = 0;

     return 0;
   }

   /* Gets the first block in the heap or returns NULL if there is not one. */
   Block* first_block() {
     Block* first = (Block*)mem_heap_lo();
     if (heap_size == 0) {
       return NULL;
     }

     return first;
   }

   /* Gets the adjacent block or returns NULL if there is not one. */
   Block* next_block(Block* block) {
     size_t distance = (block->info.size > 0) ? block->info.size : -block->info.size;

     Block* end = (Block*)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
     Block* next = (Block*)UNSCALED_POINTER_ADD(block, sizeof(BlockInfo) + distance);
     if (next >= end) {
       return NULL;
     }

     return next;
   }

   /* Print the heap by iterating through it as an implicit free list. */
   void examine_heap() {
     /* print to stderr so output isn't buffered and not output if we crash */
     Block* curr = (Block*)mem_heap_lo();
     Block* end = (Block*)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
     fprintf(stderr, "heap size:\t0x%lx\n", heap_size);
     fprintf(stderr, "heap start:\t%p\n", curr);
     fprintf(stderr, "heap end:\t%p\n", end);

     fprintf(stderr, "free_list_head: %p\n", (void*)free_list_head);

     fprintf(stderr, "malloc_list_tail: %p\n", (void*)malloc_list_tail);

     while(curr && curr < end) {
       /* print out common block attributes */
        fprintf(stderr, "%p: %ld\t", (void*)curr, curr->info.size);

        /* and allocated/free specific data */
        if (curr->info.size > 0) {
          fprintf(stderr, "ALLOCATED\tprev: %p\n", (void*)curr->info.prev);
        } else {
          fprintf(stderr, "FREE\tnextFree: %p, prevFree: %p, prev: %p\n", (void*)curr->freeNode.nextFree, (void*)curr->freeNode.prevFree, (void*)curr->info.prev);
        }

        curr = next_block(curr);
      }
      fprintf(stderr, "END OF HEAP\n\n");

      curr = free_list_head;
      fprintf(stderr, "Head ");
      while(curr) {
        fprintf(stderr, "-> %p ", curr);
        curr = curr->freeNode.nextFree;
      }
      fprintf(stderr, "\n");
    }

    /* Checks the heap data structure for consistency. */
    int check_heap() {
      Block* curr = (Block*)mem_heap_lo();
      Block* end = (Block*)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
      Block* last = NULL;
      long int free_count = 0;

      while(curr && curr < end) {
        if (curr->info.prev != last) {
          fprintf(stderr, "check_heap: Error: previous link not correct.\n");
          examine_heap();
        }

        if (curr->info.size <= 0) {
          // Free
          free_count++;
        }

            last = curr;
            curr = next_block(curr);
          }

          curr = free_list_head;
          last = NULL;
          while(curr) {
            if (curr == last) {
              fprintf(stderr, "check_heap: Error: free list is circular.\n");
              examine_heap();
            }
            last = curr;
            curr = curr->freeNode.nextFree;
            if (free_count == 0) {
              fprintf(stderr, "check_heap: Error: free list has more items than expected.\n");
              examine_heap();
            }
            free_count--;
          }

          return 0;
        }
