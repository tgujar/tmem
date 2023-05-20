#include <stdlib.h>
#include <stdbool.h>

typedef struct __blk_meta
{
    size_t size;
    struct __blk_meta *region;
    struct __blk_meta *next, *prev;
} __attribute__((packed)) blk_meta;

inline size_t get_size(blk_meta *blk);
inline bool get_blk_status(blk_meta *blk);
inline void set_allocated(blk_meta *blk);
inline void set_free(blk_meta *blk);

void *tmalloc(size_t size);
void tfree(void *ptr);