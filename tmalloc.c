#ifndef TMALLOC_H
#define TMALLOC_H
#endif

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include "tmalloc.h"

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE sizeof(blk_meta)

void *tmalloc(size_t size)
{
    size_t blk_size = ALIGN(size + HEADER_SIZE);
    blk_meta *header = sbrk(blk_size);
    header->size = size;
    return (char *)header + size;
}

void tfree(void *ptr)
{
    blk_meta *header = (blk_meta *)((char *)ptr - HEADER_SIZE);
    header->size = 0;
}