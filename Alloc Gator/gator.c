/*======================================================================
 * PROJECT: Alloc Gator
 *----------------------------------------------------------------------
 * AUTHOR: Carmen Corral
 * EMAIL: ccorral@wisc.edu
 * ADDITIONAL SOURCES: <list any persons or resources (outside of those
 * provided by the course staff) that were used to complete this code here>
 * FILE: gator.c
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 */

#include <stdio.h>
#include <sys/mman.h>
#include "gator.h"

void *heap_start = NULL; // beginning of heap prologue block
void *heap_brk = NULL;   // first invalid address beyond the end of heap
BlockHeader *first_block = NULL;

/*
 * Allocates a page (4096 bytes) of memory, and initialized to be managed
 * and used for dynamic allocations via ag_alloc() and ag_free().
 *
 * This function can be called explicitly, or if not will automatically
 * be called by the first ag_alloc() call.
 */
void ag_init()
{
    if (heap_start != NULL)
    {
        fprintf(stderr, "ERROR: can only allocate Alloc Gator heap once.\n");
        exit(1);
    }

    // allocate space for heap memory:
    // - using mmap rather than sbrk to avoid standard library confounds
    //   for example, printf calls may call malloc and change the heap
    // - not calling munmap, memory to be freed when program ends
    heap_start = mmap(NULL, PAGE_SIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    heap_brk = heap_start + PAGE_SIZE;

    // initialize heap memory to contain a single large block
    // initialize the initial prologue padding to 354
    *(BlockHeader *)heap_start = 354;

    // prologue block - 8 bytes
    BlockHeader *prologue = (BlockHeader *)((char *)heap_start + 4);
    // size of 8 bytes, and make a-bit 1 to reflect that it is allocated
    *prologue = 8 | 0x1;
    // last 4 bytes of prologue payload initialized to 354
    *(BlockHeader *)((char *)heap_start + 8) = 354;

    // initialize the header - after 8 ^^ and 4 (epilogue) bytes.. "only block" 4080 bytes
    BlockHeader avail_size = PAGE_SIZE - 12;
    first_block = (BlockHeader *)((char *)heap_start + 12);
    // set first_block size to the 4080 bytes, and make p-bit 1 indicating previous block allocated
    *first_block = avail_size | 0x2;

    // initialize the footer - after 4080 bytes - 4 to get last 4 bytes
    BlockHeader *footer = (BlockHeader *)((char *)first_block + avail_size - 4);
    *footer = avail_size;

    // initialize the epilogue block header - last 4 bytes of block
    BlockHeader *epilogue = (BlockHeader *)((char *)heap_brk - 4);
    // intialize epilogue to size of 0 and indicate that it is allocated (a-bit is 1)
    *epilogue = 0 | 0x1;
}

/*
 * Loops through and displays information about each of the dynamic memory
 * blocks being managed by this code.  This is primarily helpful for debugging.
 */
void ag_visualize()
{
    int counter = 0;
    uint32_t size = 0;
    void *start = NULL;
    void *end = NULL;
    int a = 0;
    int p = 0;

    printf("Block#: A P Size Start          End\n");
    printf("==============================================\n");
    for (BlockHeader *block = first_block;
         *block != 1 && *block != 3 && ((void *)block < heap_brk);
         block += size / sizeof(BlockHeader))
    {
        counter++;
        size = *block & ~7;
        start = block;
        end = start + size;
        int footer_size = *((int *)end - 1);
        a = *block & 1;
        p = (*block & 2) >> 1;
        printf("%6d: %1d %1d %4d %p %p\n", counter, a, p, size, start, end);
        if (a == 0 && size != footer_size)
            printf("              * Different footer size: %d\n", footer_size);
    }
}

/*
 * This function can be used to dynamically retrieve a block of contiguous
 * memory that is aligned to a 16 byte boundary.  The specified size refers to
 * the payload size of usable memory, and does not include any related headers
 * or padding.  If no block of the requested size can be allocated, this
 * function instead returns a NULL pointer.
 */
void *ag_alloc(size_t size)
{
    if (heap_start == NULL)
        ag_init();

    // return NULL if sizes are out of bounds,
    // includes blocks with sizes that are less than one
    if (size < 1 || size > PAGE_SIZE)
        return NULL;

    // keep track of smallest block that is free and its size
    BlockHeader *smallest_block = NULL;
    BlockHeader smallest_size = 0;

    // keep track of current block that we are checking and the size that we are looking for
    // 4 header bytes, the payload, and round up to a multiple of 16
    BlockHeader *curr_block = first_block;
    // ensure that every block's payload is aligned to a 16 byte boundary by only allocating bloc>
    BlockHeader size_needed = (BlockHeader)((4 + size + (ALIGNMENT - 1)) & ~(size_t)(ALIGNMENT - 1));

    // use best-fit placement policy to find a free block to use for new allocation - check a-bit>
    // while the current block is not the end of memory / the epilogue
    while ((*curr_block & 0x3) != 0)
    {
        // if the current block is free and big enough
        if (!(*curr_block & 0x1) && ((*curr_block & 0x3) >= size_needed))
        {
            // if we haven't found a block or the current size we just found is less than t>
            if (smallest_block == NULL || (*curr_block & 0x3) < smallest_size)
            {
                smallest_block = curr_block;
                smallest_size = (*curr_block & 0x3);
            }
            // if we found a perfect fit, we're done (already set it ^^)
            if ((*curr_block & 0x3) == size_needed)
                break;
        }

        curr_block = (BlockHeader *)((char *)curr_block + (*curr_block & 0x3));
    }

    // return NULL when no available block with the needed size can be allocated,
    // if we did not find a suitable block, return NULL
    if (smallest_block == NULL)
        return NULL;

    // split oversize blocks, when a perfect sized one is not available
    // leftover bytes after we allocated above ^^
    BlockHeader leftover = smallest_size - size_needed;

    if (leftover > 0)
    {
        // update the header for the affected block
        // setting the block as allocated
        *smallest_block = size_needed | (*smallest_block & 0x2) | 0x1;

        BlockHeader *leftover_block = (BlockHeader *)((char *)smallest_block + size_needed);
        *leftover_block = leftover | 0x2;

        // maintain footers only inside of unallocated blocks, so these must be updated in the >
        BlockHeader *footer = (BlockHeader *)((char *)leftover_block + leftover - 4);
        *footer = leftover;
    }
    // else, our perfect fit
    else
    {
        // mark our block as allocated
        *smallest_block = smallest_size | (*smallest_block & 0x2) | 0x1;

        // set the next block's p-bit to 1 to signal previous ^^ block's allocation
        BlockHeader *next_block = (BlockHeader *)((char *)smallest_block + smallest_size);
        *next_block = *next_block | 0x2;
    }

    // return the payload
    return (char *)smallest_block + 4;
}

/*
 * This function is used to free a block of dynamica memory that was previously
 * allocated using ag_alloc().  Note that the ptr argument refers to the
 * payload address that bp_alloc returned, rather than the block header.
 */
int ag_free(void *ptr)
{
    // return an error code of 1 if the ptr argument is not within range of 4096 bytes
    if (ptr == NULL || (char *)ptr < (char *)heap_start + 16 || (char *)ptr >= (char *)heap_brk - 4)
        return 1;

    // update the headers for all affected blocks
    BlockHeader *header = (BlockHeader *)((char *)ptr - 4);

    // return an error code of 2 if the ptr argument refers to a block that is not allocated and already free
    if (!(*header & 0x1))
        return 2;

    // mark the block as free
    *header = *header & ~0x1;

    // footer of free block set to the size
    BlockHeader *footer = (BlockHeader *)(char *)header + (*header & ~0x3) - 4;
    *footer = (*header & ~0x3);

    // have to set the block after allocated block's p-bit to free
    BlockHeader *next_block = (BlockHeader *)((char *)header + (*header & ~0x3));
    *next_block = *next_block & ~0x2;

    // call ag_coalesce() helper function after successfully updating the affect block headers
    // should pass the block header for the newly free block as the argument
    ag_coalesce(header);

    // return 0 when the block corresponding to the provided payload pointer is successfully freed
    return 0;
}

/*
 * This helper function should be called from ag_free() to perform immediate
 * coalescing that combines the provided block with either the previous or
 * next or both, when those neighboring blocks are free.
 */
void ag_coalesce(BlockHeader *header)
{
    // must check whether the previous and next blocks are allocated vs free
    // and when either or both of these neighboring blocks are free, this func should
    // combine these blocks by updating relevant headers and footers.

    // since this coalescing is done with each free-operation, do not need to check more
    // than one block before and after the block that is currently being freed
}
