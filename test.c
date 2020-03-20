#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"

int main()
{

    int page_size = sysconf(_SC_PAGE_SIZE);

    struct header *free_list = (struct header *)Mem_Init(page_size);
    struct header *allo_list = free_list +1;

    printf("size requested: %d\n", 5*sizeof(int));
    int *arr = (int *)Mem_Alloc(5*sizeof(int), 0);
    
    // arr[0] = 5;
    // arr[1] = 4;
    // arr[2] = 3;
    // arr[3] = 2;
    // arr[4] = 1;

    // for(int i=0; i<5; i++)
    // {
    //     printf("-> %d\n", arr[i]);
    // }

    printf("-----------------------------\n");
    printf("Free list: %p\n", free_list);
    printf("size: %d\n", free_list->size);
    printf("next: %p\n",free_list->next);
    printf("-----------------------------\n");

    printf("-----------------------------\n");
    printf("Alloc list: %p\n", allo_list);
    printf("size: %d\n", allo_list->size);
    printf("next: %p\n", allo_list->next);
    printf("-----------------------------\n");

    printf("page size: %d\n", page_size);
    printf("size of struct header: %d\n", sizeof(struct header));

    return 0;
}