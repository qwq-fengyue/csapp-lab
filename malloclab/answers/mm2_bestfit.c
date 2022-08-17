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
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE       4
#define DSIZE       8
#define CHUNKSIZE   (1<<12)

#define MAX(x, y)   ((x) > (y) ? (x) : (y))

#define PACK(size, alloc)   ((size) | (alloc))

#define GET(p)          (*(unsigned int *)(p))
#define PUT(p, val)     (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define GET_PREVNODE(ptr)           (*(unsigned int **)(ptr))
#define GET_NEXTNODE(ptr)           (*((unsigned int **)(ptr)+1))
#define SET_PREVNODE(ptr, addr)     (*(unsigned int **)(ptr) = (unsigned int *)(addr))
#define SET_NEXTNODE(ptr, addr)     (*((unsigned int **)(ptr)+1) = (unsigned int *)(addr))

int mm_init(void);
static void* extend_heap(size_t words);
static void* coalesce(void* bp);
void *mm_malloc(size_t size);
static void* find_fit(size_t asize);
static void *best_fit(size_t asize);
static void place(void* bp, size_t asize);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
static void insertNode(void* p);
static void deleteNode(void* p);

static char* heap_listp;
static void* head;

static void insertNode(void* p)
{
    if (head == NULL)
    {
        SET_PREVNODE(p, NULL);
        SET_NEXTNODE(p, NULL);
        head = p;
    }
    else
    {
        SET_PREVNODE(p, NULL);
        SET_NEXTNODE(p, head);
        SET_PREVNODE(head, p);
        head = p;
    }
}

static void deleteNode(void* p)
{
    void* prev = GET_PREVNODE(p);
    void* next = GET_NEXTNODE(p);

    if (prev == NULL && next == NULL)
        head = NULL;
    else if (prev == NULL)
    {
        SET_PREVNODE(next, NULL);
        head = next;
    }
    else if(next == NULL)
    {
        SET_NEXTNODE(prev, NULL);
    }
    else
    {
        SET_NEXTNODE(prev, next);
        SET_PREVNODE(next, prev);
    }
    return;
}
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));
    heap_listp += (2*WSIZE);
    head = NULL;

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

static void* extend_heap(size_t words)
{
    char* bp;
    size_t size;
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

static void* coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
        insertNode(bp);
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        deleteNode(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insertNode(bp);
    }
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        deleteNode(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}
/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char* bp;

    if (size == 0)
        return NULL;

    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    if ((bp = best_fit(asize)) == NULL)
    {
        extendsize = MAX(asize, CHUNKSIZE);
        if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
            return NULL;
    }
    place(bp, asize);
    return bp;
}
static void *best_fit(size_t asize)
{
	void* bp = head;
	void* best = NULL;
	size_t size;
	size_t min_size = (size_t)-1;

	while(bp != NULL)
	{
	    size = GET_SIZE(HDRP(bp));
	    if (size == asize)
        {
            return bp;
        }
		else if(size > asize && size < min_size)
        {
			min_size = size;
			best = bp;
		}
		bp = GET_NEXTNODE(bp);
	}
	return best;
}

static void* find_fit(size_t asize)
{
    void* bp;
    for (bp = head; bp != NULL; bp = GET_NEXTNODE(bp))
    {
        if (GET_SIZE(HDRP(bp)) >= asize)
            return bp;
    }
    return NULL;
}

static void place(void* bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    void* remain;

    if ((csize - asize) >= (2*DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        remain = NEXT_BLKP(bp);
        PUT(HDRP(remain), PACK(csize-asize, 0));
        PUT(FTRP(remain), PACK(csize-asize, 0));
        insertNode(remain);
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    deleteNode(bp);
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t asize, oldsize, newsize;
    void* prev = PREV_BLKP(ptr);
    void* next = NEXT_BLKP(ptr);

    if (ptr == NULL)
        return mm_malloc(size);
    if (size == 0)
        mm_free(ptr);

    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    if ((oldsize = GET_SIZE(HDRP(ptr))) >= asize)
    {
        return ptr;
    }
    else
    {
        if (!GET_ALLOC(HDRP(next)) && (newsize = GET_SIZE(HDRP(next)) + oldsize) >= asize)
        {
            PUT(HDRP(ptr), PACK(newsize, 1));
            PUT(FTRP(ptr), PACK(newsize, 1));
            deleteNode(next);
            return ptr;
        }
        else if (!GET_ALLOC(HDRP(prev)) && (newsize = GET_SIZE(HDRP(prev)) + oldsize) >= asize)
        {
            PUT(FTRP(ptr), PACK(newsize, 1));
            PUT(HDRP(prev), PACK(newsize, 1));
            deleteNode(prev);
            memmove(prev, ptr, oldsize - DSIZE);
            ptr = prev;
            return ptr;
        }
        else
        {
            void* newptr = mm_malloc(asize);
            if (ptr == NULL)
                return NULL;
            memmove(newptr, ptr, oldsize - DSIZE);
            mm_free(ptr);
            return newptr;
        }
    }
}
