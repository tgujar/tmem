#ifndef TMALLOC_H
#define TMALLOC_H
#endif

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include "tmalloc.h"
#include "logger.h"

#define ALIGNMENT 8
#define ALIGN(size, align) (((size) + (align - 1)) & ~(align - 1))

// 24 bytes, doesnt need alignment, this is a precaution
// needs to be aligned, so we can use the last bit to store if the block is allocated
// reducing size of a block by aligned bytes, will leave a aligned rest of the block
#define HEADER_SIZE ALIGN(sizeof(blk_meta), ALIGNMENT)

static blk_meta head;
static blk_meta tail;
static bool mem_initialized = false;
static free_list fl;

// TODO: footer does not need to store free list pointers
// TODO: next and prev pointers can be removed when we allocate block

blk_meta *get_footer(blk_meta *blk)
{
    size_t sz = get_size(blk);
    return (blk_meta *)((char *)blk + sz + HEADER_SIZE);
}

void copy_meta(blk_meta *header, blk_meta *dest)
{
    memcpy(dest, header, HEADER_SIZE);
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
    copy_meta(blk, get_footer(blk));
}

void set_allocated(blk_meta *blk)
{
    blk->size = blk->size | 1;
    copy_meta(blk, get_footer(blk));
}

void set_size(size_t sz, blk_meta *blk)
{
    assert(sz % ALIGNMENT == 0);
    bool allocated = get_blk_status(blk);
    blk->size = sz;
    if (allocated)
    {
        set_allocated(blk);
    }
    copy_meta(blk, get_footer(blk));
}

blk_meta *init_region(size_t sz)
{
    logger("INFO", "asked to init a region of size atleast: %zu", sz);
    // header and footer for the region so we dont access invalid memory
    const int ENDPOINT_HEADER_SIZE = HEADER_SIZE * 2;

    size_t alloc_size = ALIGN(sz + HEADER_SIZE * 2 + ENDPOINT_HEADER_SIZE, ALIGNMENT);
    size_t region_size = ALIGN(alloc_size, getpagesize());

    logger("INFO", "aligned alloc size is %zu bytes", alloc_size);
    logger("INFO", "allocating region of size %zu bytes", region_size);

    void *region = mmap(
        NULL,
        region_size,
        PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED,
        -1,
        0);

    blk_meta *start_block = region;
    blk_meta *end_block = (blk_meta *)((char *)region + region_size - HEADER_SIZE);

    // set blocks to allocated so blocks within region never access invalid memory
    start_block->size = start_block->size | 1;
    end_block->size = end_block->size | 1;

    blk_meta *allocated = (blk_meta *)((char *)region + HEADER_SIZE);
    set_size(region_size - ENDPOINT_HEADER_SIZE - HEADER_SIZE * 2, allocated);

    logger("INFO", "usable block is of size %zu bytes", get_size(allocated));
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
    copy_meta(blk, get_footer(blk));
}

void push_back(free_list *fl, blk_meta *blk)
{
    blk->next = fl->tail;
    blk->prev = fl->tail->prev;
    fl->tail->prev->next = blk;
    fl->tail->prev = blk;
    copy_meta(blk, get_footer(blk));
}

blk_meta *split_block(free_list *fl, blk_meta *blk, size_t sz)
{

    assert(sz % ALIGNMENT == 0);
    assert(get_blk_status(blk) == false);
    if (get_size(blk) == sz)
        return blk;

    // we need to add a header and footer meta to the region
    else if (get_size(blk) < (sz) + HEADER_SIZE * 2)
        return NULL;

    erase(blk);
    size_t og_sz = get_size(blk);
    logger("INFO", "splitting original block is of size %zu bytes", og_sz);

    set_size(sz, blk);

    blk_meta *other_blk = (blk_meta *)((char *)blk + HEADER_SIZE * 2 + sz);
    set_size(og_sz - sz - HEADER_SIZE * 2, other_blk);

    push_back(fl, blk);
    push_back(fl, other_blk);

    logger("INFO",
           "split into two blocks of size %zu bytes and %zu bytes, with header size %zu bytes ",
           get_size(blk),
           get_size(other_blk), HEADER_SIZE);

    return blk;
}

blk_meta *merge_block(free_list *fl, blk_meta *blk1, blk_meta *blk2)
{
    assert(get_blk_status(blk1) == false && get_blk_status(blk2) == false);
    logger("INFO",
           "merging two blocks of size %zu bytes and %zu bytes, with header size %zu bytes",
           get_size(blk1),
           get_size(blk2), HEADER_SIZE);
    erase(blk1);
    erase(blk2);

    // footer of the blk1 and header of blk2 are the removed
    size_t new_blk_size = get_size(blk1) + get_size(blk2) + HEADER_SIZE * 2;
    set_size(new_blk_size, blk1);

    logger("INFO",
           "combined blocks into block with size %zu bytes",
           get_size(blk1));

    push_back(fl, blk1);
    return blk1;
}

blk_meta *get_block(
    free_list *fl,
    size_t sz,
    blk_meta *(*find_fit)(free_list *fl, size_t sz))
{
    blk_meta *ret = (blk_meta *)find_fit(fl, sz);
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
    sz = ALIGN(sz, ALIGNMENT);
    blk_meta *temp = fl->head->next;
    size_t fit = INT_MAX;
    blk_meta *ret = NULL;
    while (temp != fl->tail)
    {
        size_t temp_size = get_size(temp);
        if (temp_size >= sz && temp_size < fit)
        {
            if (temp_size == sz)
                return ret;

            // WARNING: we might end up with blocks of size 0
            else if (temp_size >= sz + HEADER_SIZE * 2)
            {
                ret = temp;
                fit = temp_size;
            }
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
    blk_meta *blk = (blk_meta *)((char *)ptr - HEADER_SIZE);
    logger("INFO", "freeing block of size %zu bytes", get_size(blk));
    set_free(blk);
    push_back(&fl, blk);

    blk_meta *prev_blk = (blk_meta *)((char *)blk - HEADER_SIZE);
    logger("INFO", "previous block to freed block is  of size %zu bytes",
           get_size(prev_blk));
    blk_meta *next_blk = (blk_meta *)((char *)get_footer(blk) + HEADER_SIZE);
    logger("INFO", "next block to freed block is  of size %zu bytes",
           get_size(next_blk));

    if (get_blk_status(prev_blk) == false)
        blk = merge_block(&fl, prev_blk, blk);
    if (get_blk_status(next_blk) == false)
        blk = merge_block(&fl, blk, next_blk);
}