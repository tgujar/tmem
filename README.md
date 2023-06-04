# tmem

Memory allocation library

## Construction

Memory regions are allocated using `mmap()` calls when required. This happens when the allocator does not have a usable free block of required size in free list.

**Note**: Memory regions are always allocated in multiples of the page size.

Memory block metadata maintains size of the available memory block along with pointers to the next and previous blocks in the free list. This is placed at the start of the block and also at the end, allowing us to traverse the free list in any direction.

```C
typedef struct __blk_meta
{
    size_t size;
    struct __blk_meta *next, *prev;
} __attribute__((packed)) blk_meta;
```

Blocks are always allocated aligned to a `8 byte` boundary. This allows the LSB of the `size` variable in the metadata to be used to determine if a block is free or allocated.

Blocks are allocated by using the Best fit algorithm which scans the free list. When a block is unallocated, it is merged with adjacent free blocks.

**Note**: To avoid accessing invalid memory while checking if next or previous adjacent block in memory is free, there is a metadata header at the start and end of every memory region. These headers are set as allocated and effectively act as delimiters for the memory region.

## Running tests

Tests are written using the [Check](https://libcheck.github.io/check/) framework. To run tests, run the following commands from the build directory

```
$ cmake ..
$ make
$ make test
```
