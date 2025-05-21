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
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic Macros */
#define WSIZE 4                                                  // Word and header/footer size (bytes)
#define DSIZE 8                                                  // Double word size (bytes)
#define CHUNK_SIZE (1 << (WSIZE + DSIZE))                        // Extend heap by this amount (bytes)
#define PACK(size, alloc) ((size) | (alloc))                     // Pack a size and allocated bit into a word
#define GET(p) (*(unsigned int *)(p))                            // Read the size and allocated fields from a pointer
#define SET(p, val) (*(unsigned int *)(p) = (val))               // Write a size and allocated bit into a pointer
#define GET_SIZE(p) (GET(p) & ~0x7)                              // Read the size from a pointer
#define GET_ALLOC(p) (GET(p) & 0x1)                              // Read the allocated bit from a pointer
#define HEADER(p) ((char *)(p) - WSIZE)                          // Find the header of a block
#define FOOTER(p) ((char *)(p) + GET_SIZE(HEADER(p)) - DSIZE)    // Find the footer of a block
#define FIND_NEXT(p) ((char *)(p) + GET_SIZE(HEADER(p)))         // Find the next block
#define FIND_PREV(p) ((char *)(p) - GET_SIZE((char *)p - DSIZE)) // Find the previous block
#define FIND_PAYLOAD(p) ((char *)(p) + WSIZE)                    // Find the bp of a block
#define STAT_USED 1                                              // Used block
#define STAT_FREE 0                                              // Free block
#define EPILOGUE_HEADER PACK(0, 1)                               // Epilogue block header
#define MAX(x, y) ((x) > (y) ? (x) : (y))                        // Max function
#define MIN(x, y) ((x) < (y) ? (x) : (y))                        // Min function
/* Global Variables */
static char *head = NULL,
            *tail = NULL; // Pointer to the first and last block in the heap
/* Utils functions */
void *merge(void *p);             // Merge the block with the next block
void *extend(size_t words);       // Extend the heap by a given number of words
void *find(size_t size);          // Find a free block of a given size
void split(void *p, size_t size); // Split a block to free blocks

/* Debug Functions*/
#ifdef DEBUG
void print_heap(); // Print the heap for debugging
#endif

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // Create the initial empty heap for 4 bytes
    head = mem_sbrk(4 * WSIZE);
    if (head == (void *)-1)
        return -1;
    SET(head, 0);                           // Alignment padding
    SET(head + 1 * WSIZE, PACK(DSIZE, 1));  // Prologue header
    SET(head + 2 * WSIZE, PACK(DSIZE, 1));  // Prologue footer
    SET(head + 3 * WSIZE, EPILOGUE_HEADER); // Epilogue header

    head = head + 2 * WSIZE; // Set the head pointer to the first block
    tail = head + DSIZE;     // Set the tail pointer to the epilogue header
    // Extend the empty heap with a free block of CHUNK_SIZE bytes
    if (extend(CHUNK_SIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size == 0)
        return NULL; // If the size is 0, return NULL
    size_t asize;    // Adjusted block size
    if (size <= DSIZE)
        asize = 2 * DSIZE; // Minimum block size
    else
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE); // Adjust the size to be a multiple of DSIZE
    char *bp = find(asize);                                     // Find a free block of the adjusted size
    if (bp == NULL)                                             // If no free block is found
    {
        size_t exsize = MAX(asize, CHUNK_SIZE); // Extend the heap by the maximum of the adjusted size and the chunk size
        bp = extend(exsize / WSIZE);            // Extend the heap
        if (bp == NULL)                         // If the extension fails
            return NULL;                        // Return NULL
    }
    split(bp, asize); // Split the block
#ifdef DEBUG
    // print_heap(); // Print the heap for debugging
#endif
    return bp; // Return the pointer to the block
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if (ptr == NULL)
        return;
    size_t size = GET_SIZE(HEADER(ptr));
    SET(HEADER(ptr), PACK(size, STAT_FREE)); // Set the header
    SET(FOOTER(ptr), PACK(size, STAT_FREE)); // Set the footer
    merge(ptr);                              // Merge the block with free blocks
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HEADER(oldptr)); // Get the size of the old block
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

void *extend(size_t words)
{
    void *bp;
    size_t size;
    if (words % 2 == 1)
        size = (words + 1) * WSIZE; // Align to double word
    else
        size = words * WSIZE;
    bp = mem_sbrk(size);
    if (bp == (void *)-1)
        return NULL;
    SET(HEADER(bp), PACK(size, STAT_FREE));      // Set the header
    SET(FOOTER(bp), PACK(size, STAT_FREE));      // Set the footer
    SET(HEADER(FIND_NEXT(bp)), EPILOGUE_HEADER); // Set the epilogue header
    return merge(bp);                            // Merge the block with free blocks
}

void split(void *p, size_t size)
{
    size_t blk_size = GET_SIZE(HEADER(p)); // Get the size of the block
    if ((blk_size - size) >= 2 * DSIZE)    // If the block is large enough to split
    {
        SET(HEADER(p), PACK(size, STAT_USED));                       // Set the header
        SET(FOOTER(p), PACK(size, STAT_USED));                       // Set the footer
        SET(HEADER(FIND_NEXT(p)), PACK(blk_size - size, STAT_FREE)); // Set the next block header
        SET(FOOTER(FIND_NEXT(p)), PACK(blk_size - size, STAT_FREE)); // Set the next block footer
    }
    else // If the block is not large enough to split
    {
        SET(HEADER(p), PACK(blk_size, STAT_USED)); // Set the header
        SET(FOOTER(p), PACK(blk_size, STAT_USED)); // Set the footer
    }
}

void *merge(void *p)
{
    void *prev_block = FIND_PREV(p);             // Get the previous block
    void *next_block = FIND_NEXT(p);             // Get the next block
    size_t prev = GET_ALLOC(FOOTER(prev_block)); // Get the previous block status
    size_t next = GET_ALLOC(HEADER(next_block)); // Get the next block status
    size_t size = GET_SIZE(HEADER(p));           // Get the size of the block
    // If the previous block is used and the next block is used
    if (prev && next)
        return p; // Do nothing
    // If the previous block is used and the next block is free
    if (prev && !next)
    {
        size += GET_SIZE(HEADER(next_block));  // Increase the size of the block
        SET(HEADER(p), PACK(size, STAT_FREE)); // Set the header
        SET(FOOTER(p), PACK(size, STAT_FREE)); // Set the footer
    }
    // If the previous block is free and the next block is used
    if (!prev && next)
    {
        size += GET_SIZE(HEADER(prev_block));           // Increase the size of the block
        SET(HEADER(prev_block), PACK(size, STAT_FREE)); // Set the header
        SET(FOOTER(prev_block), PACK(size, STAT_FREE)); // Set the footer
        p = prev_block;                                 // Set the pointer to the previous block
    }
    if (!prev && !next)
    {
        size += GET_SIZE(HEADER(prev_block)) + GET_SIZE(HEADER(next_block)); // Increase the size of the block
        SET(HEADER(prev_block), PACK(size, STAT_FREE));                      // Set the header
        SET(FOOTER(prev_block), PACK(size, STAT_FREE));                      // Set the footer
        p = prev_block;                                                      // Set the pointer to the previous block
    }
    return p; // Return the pointer to the block
}

void *find(size_t size)
{
    void *bp = NULL;
    size_t min_size = 0xFFFFFFFF; // Set the minimum size to the maximum value
    for (void *ptr = head; GET_SIZE(HEADER(ptr)) > 0; ptr = FIND_NEXT(ptr))
        if (GET_ALLOC(HEADER(ptr)) == STAT_FREE && GET_SIZE(HEADER(ptr)) >= size) // If the block is free
        {
            if (GET_SIZE(HEADER(ptr)) < min_size) // If the block is smaller than the minimum size
            {
                bp = ptr;                         // Set the block to the pointer
                min_size = GET_SIZE(HEADER(ptr)); // Set the minimum size to the size of the block
            }
        }
    return bp; // Return the pointer to the block
}

#ifdef DEBUG
void print_heap()
{
    printf("Heap: \n");
    for (void *ptr = head; GET_SIZE(HEADER(ptr)) > 0; ptr = FIND_NEXT(ptr))
    {
        printf("Block: %p, Size: %zu, Allocated: %d\n", ptr, GET_SIZE(HEADER(ptr)), GET_ALLOC(HEADER(ptr)));
    }
    printf("Epilogue: %p\n", tail);
}
#endif