#ifndef TMALLOC_H
#define TMALLOC_H
#endif

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include "tmalloc.h"

#define ALIGNMENT 8
#define ALIGN(size, align) (((size) + (align - 1)) & ~(align - 1))
#define HEADER_SIZE sizeof(blk_meta)

static blk_meta head;
static blk_meta tail;
static bool mem_initialized = false;

size_t get_size(blk_meta *blk)
{
    return blk->size & ~(ALIGNMENT - 1);
}

bool get_blk_status(blk_meta *blk)
{
    return blk->size & 1;
}

void mem_init()
{
    mem_initialized = true;
    head.next = &tail;
    tail.prev = &head;
}

void *dummy(size_t sz)
{
    return NULL;
}

void *best_fit(size_t size)
{
    blk_meta *temp = head.next;
    size_t fit = INT_MAX;
    blk_meta *ret = NULL;
    while (temp != &tail)
    {
        size_t temp_size = get_size(temp);
        if (get_blk_status(temp) && temp_size >= size && temp_size < fit)
        {
            ret = temp;
            fit = temp_size;
            if (temp_size == size)
                return ret;
        }
        temp = temp->next;
    }
    return ret;
}

void *make_fit(size_t size, void *(*try_fit)(size_t sz))
{
    blk_meta *ret = (blk_meta *)try_fit(size);
    size_t alloc_size = ALIGN(size + HEADER_SIZE, ALIGNMENT);
    if (ret)
    {
        printf("found block");
        ret->size = ret->size | 1; // mark block allocated
        return ret;
    }

    size_t region_size = ALIGN(alloc_size, getpagesize());

    ret = (blk_meta *)mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    ret->region = ret;
    ret->size = region_size;

    ret->prev = tail.prev;
    ret->next = &tail;
    tail.prev->next = ret;
    tail.prev = ret;
    return ret;
}

void *tmalloc(size_t size)
{
    if (!mem_initialized)
        mem_init();
    blk_meta *header = make_fit(size, &best_fit);
    return (char *)header + HEADER_SIZE;
}

void tfree(void *ptr)
{
    blk_meta *header = (blk_meta *)((char *)ptr - HEADER_SIZE);
    header->size = header->size ^ 1; // unallocate block
}