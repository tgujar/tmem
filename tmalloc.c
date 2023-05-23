#ifndef TMALLOC_H
#define TMALLOC_H
#endif

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include "tmalloc.h"

#define ALIGNMENT 8
#define ALIGN(size, align) (((size) + (align - 1)) & ~(align - 1))
#define HEADER_SIZE sizeof(blk_meta)

static blk_meta head;
static blk_meta tail;
static bool mem_initialized = false;
static free_list fl;

// TODO: footer does not need to store free list pointers

blk_meta *get_footer(blk_meta *blk)
{
    size_t sz = get_size(blk);
    return (blk_meta *)((char *)blk + sz);
}

size_t get_size(blk_meta *blk)
{
    return blk->size & ~(ALIGNMENT - 1);
}

bool get_blk_status(blk_meta *blk)
{
    return blk->size & 1;
}

void set_free(blk_meta *blk)
{
    blk->size = blk->size & ~1;
    blk_meta *footer = get_footer(blk);
    footer->size = blk->size & ~1;
}

void set_allocated(blk_meta *blk)
{
    blk->size = blk->size | 1;
    blk_meta *footer = get_footer(blk);
    footer->size = blk->size | 1;
}

void set_size(size_t sz, blk_meta *blk)
{
    bool allocated = get_blk_status(blk);
    blk->size = sz;
    blk_meta *footer = get_footer(blk);
    footer->size = sz;
    if (allocated)
    {
        set_allocated(footer);
        set_allocated(blk);
    }
}

blk_meta *init_region(size_t sz)
{
    const int ENDPOINT_HEADER_SIZE = HEADER_SIZE * 2;
    size_t alloc_size = ALIGN(sz + HEADER_SIZE * 2 + ENDPOINT_HEADER_SIZE, ALIGNMENT);
    size_t region_size = ALIGN(alloc_size, getpagesize());

    void *region = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    blk_meta *start_block = region;
    blk_meta *end_block = (blk_meta *)((char *)region + region_size - HEADER_SIZE);
    set_allocated(start_block);
    set_allocated(end_block);

    blk_meta *allocated = (blk_meta *)((char *)region + HEADER_SIZE);
    set_size(region_size - ENDPOINT_HEADER_SIZE - HEADER_SIZE * 2, allocated);
    return allocated;
}

// free list functions
void erase(blk_meta *blk)
{
    blk->prev->next = blk->next;
    blk->next->prev = blk->prev;
}

void push_front(free_list *fl, blk_meta *blk)
{
    blk->next = fl->head->next;
    blk->prev = fl->head;
    fl->head->next->prev = blk;
    fl->head->next = blk;
}

void push_back(free_list *fl, blk_meta *blk)
{
    blk->next = fl->tail;
    blk->prev = fl->tail->prev;
    fl->tail->prev->next = blk;
    fl->tail->prev = blk;
}

blk_meta *split_block(free_list *fl, blk_meta *blk, size_t sz)
{
    if (get_size(blk) == sz)
        return blk;
    else if (get_size(blk) < (sz) + HEADER_SIZE * 2)
        return NULL;

    erase(blk);
    size_t og_sz = get_size(blk);
    set_size(sz, blk);

    blk_meta *other_blk = (blk_meta *)((char *)blk + HEADER_SIZE * 2 + sz);
    set_size(og_sz - sz - HEADER_SIZE * 2, other_blk);

    push_back(fl, blk);
    push_back(fl, other_blk);
    return blk;
}

blk_meta *get_block(free_list *fl, size_t sz, blk_meta *(*find_fit)(free_list *fl, size_t sz))
{
    blk_meta *ret = (blk_meta *)find_fit(fl, sz + 2 * HEADER_SIZE);
    if (ret)
    {
        ret = split_block(fl, ret, sz);
        assert(ret != NULL);
        erase(ret);
        return ret;
    }
    ret = init_region(sz);
    push_back(fl, ret);
    return get_block(fl, sz, find_fit);
}

blk_meta *best_fit(free_list *fl, size_t sz)
{
    blk_meta *temp = fl->head->next;
    size_t fit = INT_MAX;
    blk_meta *ret = NULL;
    while (temp != fl->tail)
    {
        size_t temp_size = get_size(temp);
        if (temp_size >= sz && temp_size < fit)
        {
            ret = temp;
            fit = temp_size;
            if (temp_size == sz)
                return ret;
        }
        temp = temp->next;
    }
    return ret;
}

void mem_init()
{
    mem_initialized = true;
    head.next = &tail;
    tail.prev = &head;
    fl.head = &head;
    fl.tail = &tail;
}

void *dummy(size_t sz)
{
    return NULL;
}

void *tmalloc(size_t size)
{
    if (!mem_initialized)
        mem_init();
    blk_meta *header = get_block(&fl, size, &best_fit);
    return (char *)header + HEADER_SIZE;
}

void tfree(void *ptr)
{
    blk_meta *header = (blk_meta *)((char *)ptr - HEADER_SIZE);
    set_free(header);
    push_back(&fl, header);
}