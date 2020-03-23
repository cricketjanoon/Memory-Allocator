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

    if(head_ptr == MAP_FAILED)
    {
        printf("Mem_Init(): Requested memory cannot be assigned.\n");
        return NULL;
    }
    else
    {
        //at the start of memory I am storing ref to 2 list(free mem list and allocated mem list)
        struct header **free_list = (struct header **)head_ptr;
        struct header **alloc_list = free_list + 1;
        struct header *first_free_block = alloc_list + 1;

        *alloc_list = NULL; //because nothing allocated yet

        first_free_block->size =sizeOfRegion-2*sizeof(struct header **)-sizeof(struct header);        
        first_free_block->next  = NULL;

        *free_list = first_free_block;
        
        return head_ptr;
    }
}


void *Mem_Alloc(int size, int expand)
{
    struct header **free_list = (struct header **)head_ptr;
    struct header **allo_list = free_list+1;
    
    // log_address("Mem_Alloc, freelist:", free_list);
    // log_address("Mem_Alloc, alloclist:", allo_list);

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
    cur_free_block = *free_list;
    prev_free_block = *free_list;
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
            new_free_block = (struct header *) ((void *)cur_free_block + size);
            new_free_block->size = cur_free_block->size-size;
            new_free_block->next = next_free_block;

            if(cur_free_block == prev_free_block) //means what we had only one block
            {
                *free_list = new_free_block; 
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
                *free_list = next_free_block;
            }
            else
            {
                prev_free_block->next = next_free_block;
            }
        }
        
        //Now adding the given memory to allocated-list
        cur_free_block->size = size;

        // printf("MemALloc, cur_free_block %p", cur_free_block->size);

        if(*allo_list==NULL)
        {
            *allo_list=cur_free_block;
            cur_free_block->next = NULL;
        }
        else //adding new block to the front of the list
        {
            cur_free_block->next = *allo_list;
            *allo_list = cur_free_block;
        }
        
        return (void *)cur_free_block + sizeof(struct header);

    }
    else
    {
        if(expand)
        {
            //TODO: handle the case of memory expansion
        }
        else
        {
            printf("Mem_Alloc(): Cannot provide more memory.\n");
            return NULL;
        }
    }
}

int Mem_Free(void *ptr, int coalesce, int release)
{
    if(ptr == NULL)
        return 0;

    struct header **free_list = (struct header **)head_ptr;
    struct header **allo_list = free_list+1;

    struct header *block_to_free = ptr - sizeof(struct header);


    printf("Address: %p, Size: %d, Next: %p\n", block_to_free, block_to_free->size, block_to_free->next);

    struct header *cur = *allo_list, *prev = *allo_list;
    while (cur && cur->next != block_to_free)
    {
        prev = cur;
        cur = cur->next;
    }
    
    if(cur)
    {
        //remove from allocated list
        if(cur == prev) // if there is the first in the list
        {
            (*allo_list) = cur->next;
        }
        else // if in the middle of the list
        {
            prev->next = cur->next;
        }
        
        //add block at the start of free block
        cur->next = *free_list;
        *free_list = cur;
    }
}


void hexDump(char *desc, void *addr, int len) 
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

void print_free_alloc_list()
{

    struct header **free_l = (struct header **)head_ptr;
    struct header **allo_l = free_l +1;

    struct header *free_list = *free_l;
    struct header *alloc_list = *allo_l;

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
#endif