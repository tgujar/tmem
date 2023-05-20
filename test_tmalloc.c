#include "tmalloc.h"
#include "stdio.h"

void simple_check()
{
    int *arr = (int *)tmalloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }
    for (int i = 0; i < 10; i++)
    {
        printf("%d", arr[i]);
    }

    tfree(arr);
}

void alloc_freed()
{
    int *arr = (int *)tmalloc(sizeof(int) * 10);

    tfree(arr);
    arr = (int *)tmalloc(sizeof(int) * 10);
}

int main()
{
    simple_check();
    alloc_freed();
}