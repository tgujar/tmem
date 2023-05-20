#include <stdlib.h>

typedef struct __blk_meta
{
    size_t size;
    struct __blk_meta *next, *prev;
} __attribute__((packed)) blk_meta;

void *tmalloc(size_t size);
void tfree(void *ptr);