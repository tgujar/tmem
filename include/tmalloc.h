#ifndef TMALLOC_H
#define TMALLOC_H
#endif

#include <stdlib.h>
#include <stdbool.h>

typedef struct __blk_meta
{
    size_t size;
    struct __blk_meta *next, *prev;
} __attribute__((packed)) blk_meta;

// gets the pointer to the metadata stored at footer of a block
inline blk_meta *get_footer(blk_meta *blk);

// copies header of block to another block, used to copy changes to footer
inline void copy_meta(blk_meta *header, blk_meta *dest);

// get size of the block, excludes the header and footer
inline size_t get_size(blk_meta *blk);

// check if block is allocated, returns true if allocated
inline bool get_blk_status(blk_meta *blk);

// mark allocated bit as true
inline void set_allocated(blk_meta *blk);

// mark allocated bit as false
inline void set_free(blk_meta *blk);

// sets the size of the block, leaves allocated bit unchanged
inline void set_size(size_t sz, blk_meta *blk);

// sets stopping blocks at start and end of region
inline blk_meta *init_region(size_t sz);

typedef struct __free_list
{
    blk_meta *head, *tail;
} free_list;

// remove block from free list and mark allocated
// note: does not add block to free list
inline void erase(blk_meta *blk);

// adds a block to the front of the list;
inline void push_front(free_list *fl, blk_meta *blk);

// adds a block to the back of the list;
inline void push_back(free_list *fl, blk_meta *blk);

// get a block of a size sz, allocated memory if necessary
inline blk_meta *get_block(
    free_list *fl,
    size_t sz,
    blk_meta *(*find_fit)(free_list *fl, size_t sz));

// split a block into two blocks, returns the first block
// note: splits only if necessary, if it finds that the block is
// of exact size required, it will only return the block
inline blk_meta *split_block(free_list *fl, blk_meta *blk, size_t sz);

// merge two blocks together, blk1 must be before blk2
inline blk_meta *merge_block(free_list *fl, blk_meta *blk1, blk_meta *blk2);

// malloc and free functions
void *tmalloc(size_t size);
void tfree(void *ptr);