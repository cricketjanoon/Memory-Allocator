#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"

//to print out extra logs
int DEBUG = 0;

void *Mem_Init(int sizeOfRegion)
{
    head_ptr = mmap(NULL, sizeOfRegion, PROT_READ| PROT_WRITE , MAP_PRIVATE | MAP_ANON , -1 , 0);

    if(head_ptr == MAP_FAILED)
    {
        printf("Mem_Init(): mmap returned NULL.\n");
        return NULL;
    }
    else
    {
        //at the start of memory I am storing ref to 3 listS(mem_chuncks(by mmap), free_list and alloc_list)
        struct header **mem_chunck_list = (struct header **)head_ptr;
        struct header **free_list = (struct header **)(head_ptr + sizeof(struct header **));
        struct header **alloc_list = (struct header **)(head_ptr + 2*sizeof(struct header **));
        
        //at this moment only one chuck
        *mem_chunck_list = NULL;

        //because nothing allocated yet
        *alloc_list = NULL; 

        //creating first free block
        struct header *first_free_block =  head_ptr + 3*sizeof(struct header **);
        first_free_block->size =sizeOfRegion-3*sizeof(struct header **)-sizeof(struct header);        
        first_free_block->next  = NULL;

        *free_list = first_free_block;
        
        if(DEBUG)
            printf("Mem_Init(): (mmap_chuncks): %p, (free_list): %p, (alloc_list): %p\n", *mem_chunck_list, *free_list, *alloc_list);

        return head_ptr;
    }
}

int Mem_Free(void *ptr, int coalesce, int release)
{
    if(ptr == NULL)
        return 0;

    //getting references to the heads of three lists
    struct header **mem_chunck_list = (struct header **)head_ptr;
    struct header **free_list = (struct header **)(head_ptr + sizeof(struct header **));
    struct header **alloc_list = (struct header **)(head_ptr + 2*sizeof(struct header **));

    //getting the address where the header starts
    struct header *block_to_free = ptr - sizeof(struct header);

    //find the block in the list
    struct header *cur = *alloc_list, *prev = *alloc_list;
    while (cur && cur != block_to_free)
    {
        prev = cur;
        cur = cur->next;
    }
    
    if(DEBUG)
        printf("Mem_Free: Addr: %p, Size: %d, Next: %p, Cur: %p, Prev: %p\n", block_to_free, block_to_free->size, block_to_free->next, cur, prev);

    if(cur)
    {
        //remove from allocated list
        if(cur == prev) // if there is the first in the list
        {
            *alloc_list = cur->next;
        }
        else // if in the middle of the list
        {
            prev->next = cur->next;
        }
        
        //add block at the start of free block
        cur->next = *free_list;
        *free_list = cur;
        
        if(DEBUG)
            printf("Mem_Free: free_list(%p), alloc_list(%p)\n", *free_list, *alloc_list);
    }
    else
    {
        //cases when pointer is not assigned by Mem_Alloc()
        printf("Mem_Free: Could not free memory, invalid pointer.\n");
        return -1;
    }
    
    if(coalesce)
    {
        //because recently freed block is added to the front of the list
        struct header *freed_block = *free_list;

        //traverse the list and find if adjacents blocks are free to coalesce
        struct header *cur_block = (*free_list)->next;
        struct header *prev_block = *free_list;
        int counter = 0;
        while(cur_block)
        {
            //when block before freed block is free
            if((void *)cur_block + sizeof(struct header) + cur_block->size == freed_block)
            {
                //ditching the recetly created freed block
                *free_list = (*free_list)->next;
                cur_block->size = freed_block->size + sizeof(struct header) +cur_block->size;

                counter++;
                if(counter==2)
                    break;
            }
            //when block after freed block is free
            else if ((void *)freed_block + sizeof(struct header) + freed_block->size == cur_block)
            {
                //ditching the block after the freed block
                prev_block->next = cur_block->next;
                freed_block->size = cur_block->size + sizeof(struct header) + freed_block->size;

                counter++;
                if(counter == 2)
                    break;
            }
            prev_block = cur_block;
            cur_block = cur_block->next;
        }
    }

    if(release)
    {
        int _continue;

        struct header *cur_chunck = *mem_chunck_list, *prev_chunck = *mem_chunck_list;
        while(cur_chunck)
        {
            struct header *possible_free_block = (void *)cur_chunck + sizeof(struct header);
            struct header *cur_free_block = *free_list, *prev_free_block=*free_list;
            _continue = 0;

            while (cur_free_block)
            {
                //check if the there is any free block which is entirely on the free_chunck
                if(cur_free_block == possible_free_block && cur_free_block->size == cur_chunck->size-2*sizeof(struct header))
                {
                    void *mem_to_unmap = cur_chunck;
                    int length = cur_chunck->size;

                    //ditching the free chunck from mem_chunck_list
                    if(cur_chunck == prev_chunck) 
                    { 
                        *mem_chunck_list = cur_chunck->next;
                        cur_chunck = *mem_chunck_list;
                        prev_chunck = *mem_chunck_list;
                    }
                    else
                    {
                        prev_chunck->next = cur_chunck->next;
                        cur_chunck=prev_chunck->next;
                    }                        
                    _continue = 1;

                    //ditching the free block from free_list
                    if(prev_free_block == cur_free_block) 
                    {
                        *free_list = cur_free_block->next;
                        cur_free_block = *free_list;
                        prev_free_block = *free_list;
                    }
                    else
                    {
                        prev_free_block->next = cur_free_block->next;
                        cur_free_block = prev_free_block->next;
                    }

                    //unmapping the memory
                    munmap(mem_to_unmap, length);
                }
                else
                {
                    prev_free_block=cur_free_block;
                    cur_free_block = cur_free_block->next;
                }
                
            }
            
            if(_continue)
                continue;

            prev_chunck = cur_chunck;
            cur_chunck = cur_chunck->next;
        }
    }

    return 0;
}

void *Mem_Alloc(int size, int expand)
{
    //if user request memory of size 0 or less
    if(size <= 0)
        return NULL;
    
    struct header **mem_chunck_list;
    struct header **free_list;
    struct header **alloc_list;

    retry_allocation:

    //getting references of the lists
    mem_chunck_list = (struct header **)head_ptr;
    free_list = (struct header **)(head_ptr + sizeof(struct header **));
    alloc_list = (struct header **)(head_ptr + 2*sizeof(struct header **));


    struct header *next_free_block;
    struct header *new_free_block;
    struct header *prev_free_block;
    struct header *cur_free_block;

    //find the next suitable block by traversing the free list
    cur_free_block = *free_list;
    prev_free_block = *free_list;
    while(cur_free_block && cur_free_block->next)
    {
        if (cur_free_block->size == size)
        {
            break;
        }
        else if(cur_free_block->size >= (size + sizeof(struct header)))
        {
            break;
        }
        else
        {
            prev_free_block = cur_free_block;
            cur_free_block = cur_free_block->next;
        }
    }

    if(cur_free_block && ((cur_free_block->size >= (size + sizeof(struct header))) || (cur_free_block->size == size)))
    {
        next_free_block = cur_free_block->next;

        //if found block has excess memory than required
        if(cur_free_block->size > size + sizeof(struct header))
        {
            // printf("point1\n");
            // print_free_alloc_list();
            // getchar();
            new_free_block = (struct header *) ((void *)cur_free_block + size + sizeof(struct header));
            new_free_block->size = cur_free_block->size-size-sizeof(struct header);
            new_free_block->next = next_free_block;

            // printf("new_free_block->size: %d\n", new_free_block->size);

            if(cur_free_block == prev_free_block) //means what we had only one block
            {
                *free_list = new_free_block; 
                // printf("point2\n");
                // print_free_alloc_list();
                // getchar();
            }
            else
            {
                // printf("point3\n");
                // print_free_alloc_list();
                // getchar();
                prev_free_block->next = new_free_block;
            }
        }
        else //if(cur_free_block->size == size) //found block has exactly the same memory
        {
            // printf("point4\n");
            // print_free_alloc_list();
            // getchar();
            // printf("ooolim\n");
            // printf("found block exatcly the same size.\n");
            if(cur_free_block == prev_free_block)
            {
                // printf("point5\n");
                // print_free_alloc_list();
                // getchar();
                *free_list = next_free_block;
            }
            else
            {
                // printf("point6\n");
                // print_free_alloc_list();
                // getchar();
                prev_free_block->next = next_free_block;
            }
        }
        
        //Now adding the given memory to allocated-list
        cur_free_block->size = size;

        if(*alloc_list==NULL)
        {
            // printf("point7\n");
            // print_free_alloc_list();
            // getchar();
            *alloc_list=cur_free_block;
            cur_free_block->next = NULL;
        }
        else //adding new block to the front of the list
        {
            // printf("point8\n");
            // print_free_alloc_list();
            // getchar();
            cur_free_block->next = *alloc_list;
            *alloc_list = cur_free_block;
        }
        
        if(DEBUG)
        {
            printf("Mem_Alloc: Block Assigned: %p, Size: %d, Next: %p\n", cur_free_block, cur_free_block->size, cur_free_block->next);
            printf("Mem_Alloc: free_list(%p), alloc_list(%p)\n", *free_list, *alloc_list);
        }

        // printf("point9\n");
        // print_free_alloc_list();
        // getchar();
        // printf("cur_free_block->size: %d\n", cur_free_block->size);
        return (void *)cur_free_block + sizeof(struct header);

    }
    else
    {
        if(expand)
        {
            //calculating the page aligned size for new memory allocation
            int required_size = size + 2*sizeof(struct header);
            int page_size = sysconf(_SC_PAGE_SIZE);
            int new_mem_size = page_size + required_size;
            int a = new_mem_size % page_size;
            new_mem_size = new_mem_size - a;

            void *new_chunck = mmap(NULL, new_mem_size, PROT_READ| PROT_WRITE , MAP_PRIVATE | MAP_ANON , -1 , 0);

            struct header *chunck_header = new_chunck;
            chunck_header->size = new_mem_size; //original chuck size as given by mmap() (excluding size of headers)

            if(*mem_chunck_list == NULL) //means first extra chunck
            {
                chunck_header->next = NULL;
                *mem_chunck_list = chunck_header;
            } 
            else //already have extra chucks
            {
                chunck_header->next = *mem_chunck_list;
                *mem_chunck_list = chunck_header;
            }

            //creating new free block on newly allocated chunck
            struct header *new_free_block = new_chunck + sizeof(struct header);
            new_free_block->size = new_mem_size - 2*sizeof(struct header);
            new_free_block->next = *free_list;
            *free_list = new_free_block;

            //now that new memory is allocated, try allocating the memory again
            goto retry_allocation;
        }
        else
        {
            printf("Mem_Alloc(expand=0): Cannot provide memory of %d bytes.\n", size);
            return NULL;
        }
    }
}

void Mem_Dump()
{
    struct header **mem_chunck_list = (struct header **)head_ptr;
    struct header **free_list = (struct header **)(head_ptr+sizeof(struct header **));
    struct header **alloc_list = (struct header **)(head_ptr + 2*sizeof(struct header **));

    printf("----------Free List (head: %p)-------------", *free_list);
    if(*free_list == NULL) printf("---------\n"); else printf("\n");
    struct header *cur = *free_list;
    while(cur)
    {
        printf("Address: %p, Size: %d, Next: %p\n", cur, cur->size, cur->next);
        cur = cur->next;
    }
    printf("-------------------------------------------------------\n");

    printf("-------Allocated List (head: %p)-----------", *alloc_list);
    if(*alloc_list == NULL) printf("---------\n"); else printf("\n");
    cur = *alloc_list;
    while(cur)
    {
        printf("Address: %p, Size: %d, Next: %p\n", cur, cur->size, cur->next);
        cur = cur->next;
    }
    printf("-------------------------------------------------------\n");

    printf("--------Chunck List (head: %p)-------------", *mem_chunck_list); 
    if(*mem_chunck_list == NULL) printf("---------\n"); else printf("\n");
    cur = *mem_chunck_list;
    while(cur)
    {
        printf("Address: %p, Size: %d, Next: %p\n", cur, cur->size, cur->next);
        cur = cur->next;
    }
    printf("-------------------------------------------------------\n");
}