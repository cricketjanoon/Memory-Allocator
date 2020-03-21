#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"


void print_free_alloc_list(struct header *free_list, struct header *alloc_list)
{
        printf("---------------------Free List-------------------------\n");
    struct header *c = free_list;
    while(c)
    {
        printf("Address: %p, Size: %d, Next: %p\n", c, c->size, c->next);
        c = c->next;
    }
    printf("-------------------------------------------------------\n");


    printf("---------------------Allocated List--------------------\n");
    c = alloc_list;
    while(c)
    {
        printf("Address: %p, Size: %d, Next: %p\n", c, c->size, c->next);
        c = c->next;
    }
    printf("-------------------------------------------------------\n");
}



int main()
{

    int page_size = sysconf(_SC_PAGE_SIZE);

    struct header **free_list = (struct header **)Mem_Init(page_size);
    struct header **allo_list = free_list +1;

    printf("size requested: %d\n", 5*sizeof(int));
    int *arr = (int *)Mem_Alloc(5*sizeof(int), 0);
    int *arr2 = (int *)Mem_Alloc(6*sizeof(int), 0);
    int *arr3 = (int *)Mem_Alloc(8*sizeof(int), 0);
    int *arr4 = (int *)Mem_Alloc(9*sizeof(int), 0);
    
    arr[0] = 5; printf("addres of arr[0]: %p\n", arr);
    arr[1] = 4;
    arr[2] = 3;
    arr[3] = 2;
    arr[4] = 1;

    arr2[0] = 5; printf("addres of arr2[0]: %p\n", arr2);
    arr2[1] = 4;
    arr2[2] = 3;
    arr2[3] = 2;
    arr2[4] = 1;
    arr2[5] = 7;

    print_free_alloc_list(*free_list, *allo_list);

    Mem_Free(arr2, 0, 0);

    print_free_alloc_list(*free_list, *allo_list);

    // printf("%d: ", sizeof(int)); //4
    // printf("page size: %d\n", page_size); //4096
    // printf("size of struct header: %d\n", sizeof(struct header)); //16
    // printf("size of struct header *: %d\n", sizeof(struct header*)); //8
    // printf("size of struct header **: %d\n", sizeof(struct header **)); //8

    // hexDump(NULL, head_ptr, 256);

    return 0;
}