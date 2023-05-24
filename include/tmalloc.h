#include <stdlib.h>
#include <stdbool.h>

typedef struct __blk_meta
{
    size_t size;
    struct __blk_meta *next, *prev;
} __attribute__((packed)) blk_meta;

inline blk_meta *get_footer(blk_meta *blk);
inline size_t get_size(blk_meta *blk);          // get size of the block, excludes the header size
inline bool get_blk_status(blk_meta *blk);      // check if block is allocated
inline void set_allocated(blk_meta *blk);       // mark allocated bit as true
inline void set_free(blk_meta *blk);            // mark allocated bit as false
inline void set_size(size_t sz, blk_meta *blk); // sets the size of the block, leaves allocated bit unchanged
inline blk_meta *init_region(size_t sz);        // sets stopping blocks at start and end of region

typedef struct __free_list
{
    blk_meta *head, *tail;
} free_list;

inline void erase(blk_meta *blk);                                                                      // remove block from free list and mark allocated
inline void push_front(free_list *fl, blk_meta *blk);                                                  // adds a block to the front of the list;
inline void push_back(free_list *fl, blk_meta *blk);                                                   // adds a block to the back of the list;
inline blk_meta *get_block(free_list *fl, size_t sz, blk_meta *(*find_fit)(free_list *fl, size_t sz)); // get a block of a size sz, allocated memory if necessary
inline blk_meta *split_block(free_list *fl, blk_meta *blk_meta, size_t sz);

void *tmalloc(size_t size);
void tfree(void *ptr);