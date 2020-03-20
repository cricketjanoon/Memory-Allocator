#ifndef __MEM_H_
#define __MEM_H_

#include <sys/mman.h>
#include <stdio.h>


void *head_ptr;

void *Mem_Init(int sizeOfRegion);
void *Mem_Alloc(int size, int expand);
int Mem_Free(void *ptr, int coalesce, int release);
void Mem_Dump();

struct header
{
    int size;
    struct header *next;
};

void log_address(char *s, void *ptr)
{
    printf("%s: %p\n", s, ptr);
}

void *Mem_Init(int sizeOfRegion)
{
    head_ptr = mmap(NULL, sizeOfRegion, PROT_READ| PROT_WRITE , MAP_PRIVATE | MAP_ANON , -1 , 0);
    log_address("head_ptr after mmap", head_ptr);

    if(head_ptr == MAP_FAILED)
    {
        return NULL;
    }
    else
    {
        //at the start of memory I am storing ref to 2 list(free mem list and allocated mem list)
        struct header *free_list = (struct header *)head_ptr;
        log_address("free_list", free_list);
        free_list->size =sizeOfRegion-2*sizeof(struct header);        
        free_list->next  = NULL; //for first time allocation

        struct header *alloc_list = free_list+1;
        log_address("alloc_list", alloc_list);
        alloc_list = NULL; //for first time allocation

        return head_ptr;
    }
}


void *Mem_Alloc(int size, int expand)
{
    struct header *free_list = (struct header *)head_ptr;
    struct header *allo_list = free_list+1;
    
    struct header *next_free_block;
    struct header *new_free_block;
    struct header *prev_free_block;
    struct header *cur_free_block;

    //if user request memory of size 0 or less
    if(size <= 0)
        return NULL;

    //including the size of header as well
    size = size + sizeof(struct header);

    //find the next suitable block by traversing the free list
    cur_free_block = free_list;
    prev_free_block = free_list;
    while(cur_free_block->next)
    {
        if(cur_free_block->size < size)
        {
            prev_free_block = cur_free_block;
            cur_free_block->next = cur_free_block->next;
        }
        else
        {
            break;
        }
    }

    if(cur_free_block->size >= size)
    {
        next_free_block = cur_free_block->next;

        //if found block has excess memory than required
        if(cur_free_block->size > size)
        {
            new_free_block = (struct header *) (cur_free_block + size);
            new_free_block->size = cur_free_block->size-size-sizeof(struct header);
            new_free_block->next = next_free_block;

            if(cur_free_block == prev_free_block) //means what we had only one block
            {
                free_list = new_free_block; 
            }
            else
            {
                prev_free_block->next = new_free_block;
            }
        }
        else //found block has exactly the same memory
        {
            if(cur_free_block == prev_free_block)
            {
                free_list = next_free_block;
            }
            else
            {
                prev_free_block->next = next_free_block;
            }
        }
        
        //Now adding the given memory to allocated-list
        cur_free_block->size = size;
        if(allo_list==NULL)
        {
            allo_list=cur_free_block;
            cur_free_block->next = NULL;
        }
        else //adding new block to the front of the list
        {
            cur_free_block->next = allo_list;
            allo_list = cur_free_block;
        }
        
        return cur_free_block + sizeof(struct header);

    }
    else
    {
        //TODO: handle the case of memory expansion and error return
    }
}


#endif