#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mem.h"


struct queue_element 
{
    struct queue_element *next;
    void *ptr;
};

struct queue_element *head=NULL, *tail=NULL;

void print_queue();
void test_my_code();

void read_file()
{
    FILE *file = fopen("test1.txt", "r");

    char line[12];
    fgets(line, 12, file);
    int totalUsers = atoi(line);

    while(fgets(line, 12, file) != NULL)
    {
        strtok(line, "\n"); //remove '\n' from the string

        if(line[0]=='1' && line[1]=='9')
        {    
            char *tok1 = strtok(line, " ");
            // long int rollnuber = atoi(tok1);
            char *tok = strtok(NULL, " ");
            short number_of_requests = atoi(tok);
            
            int memory_required =  sizeof(short) + 8 + number_of_requests*9 + sizeof(struct queue_element);
            void *ptr = Mem_Alloc(memory_required, 0);

            printf("Memory required: %d\n", memory_required);
            // getchar();
            // print_free_alloc_list();

            if(ptr == NULL)
            {
                printf("Main(): Mem_Alloc returned NULL for size (%d), request (%d - %s),\n", memory_required, number_of_requests, line);
                // print_free_alloc_list();
                // getchar();
                break;
            }

            struct queue_element *node_ptr = (struct queue_element *)ptr;
            struct queue_element node;
            *node_ptr = node;

            //adding element to the queue
            if(head==NULL && tail==NULL) //if the first node of the queue
            {   
                tail = node_ptr;
                head = node_ptr;
                node_ptr->next = NULL;
            }
            else //for the rest of the nodes
            {
                tail->next = node_ptr;
                tail = node_ptr;
                node_ptr->next = NULL;
            }

            char *cur_ptr = ptr + sizeof(struct queue_element);
            node_ptr->ptr = cur_ptr;

            short *req_ptr_1 = (short *)cur_ptr;
            *req_ptr_1 = number_of_requests;
            cur_ptr +=2; // for short
            
            strcpy(cur_ptr, tok1);
            cur_ptr += 8;

            char *req_ptr = (char *)cur_ptr;

            for(int i=0; i<number_of_requests; i++)
            {
                fgets(line, 12, file);
                strtok(line, "\n");
                strcpy(req_ptr, line);
                req_ptr +=9;
            }
        }
        else if (line[0] == '!')
        {
            // Dequue the request from the queue if possible
            if(!(head==NULL) && !(tail==NULL))
            {
                char *str_ptr = head->ptr;
                short req_pending = *((short *)head->ptr);
                
                if(req_pending<=1)
                {
                    void *ptr = head;
                    head = head->next;
                    printf("Freeing memory.\n");
                    // getchar();
                    Mem_Free(ptr,0,0);
                    // print_free_alloc_list();
                }
                else
                {
                    req_pending--;
                    *((short *)head->ptr) = req_pending;
                }
            }
        }
    }

    // print_queue();
    
}

void print_queue()
{
    struct queue_element *cur = head;
    printf("head of queue: %p\n", head);

    for(int i=0; i<1000 && cur != NULL; i++)
    {

        void *ptr = (void *)cur->ptr;

        short req_pending = *((short *)cur->ptr);
        printf("addr: %p, next: %p, ptr: %p, ", cur, cur->next, cur->ptr);
        printf("%d-", req_pending);

        char *str_ptr = ptr + 2;
        for(int k=0; k<8; k++)
        {
            printf("%c", str_ptr[k]);
        }
        printf("\n");
        cur = cur->next;
    }
}


int main()
{
    Mem_Init(1*sysconf(_SC_PAGE_SIZE)); //initializing two pages


    void *ptr1 = Mem_Alloc(1, 0);
    void *ptr2 = Mem_Alloc(2, 0);
    void *ptr3 = Mem_Alloc(3, 0);
    void *ptr4 = Mem_Alloc(4, 0);
    void *ptr5 = Mem_Alloc(5, 0);
    void *ptr6 = Mem_Alloc(6, 0);
    void *ptr7 = Mem_Alloc(7, 0);
    

    Mem_Free(ptr2, 1, 0);
    Mem_Free(ptr3, 1, 0);
    Mem_Free(ptr1, 1, 0);

    // test_my_code();

    // read_file();

    print_free_alloc_list();

    return 0;
}

void test_my_code()
{
    int i, j , c ;
    void* p[100];

    // algo goes here.
    for( j = 0 ; j < 10; j ++ )
    {
        for(i = 0; i < 100; i++)
        {
            c = random();
            c = c % 100;
            // print_free_alloc_list();
            // printf("Requesting memory of size %d\n", c);
            // getchar();
            p[i] = Mem_Alloc(c,0);
        }

        // print_free_alloc_list();
        // getchar();

        for(i = 99; i >= 0; i--)
        {
            // print_free_alloc_list();
            // getchar();
            Mem_Free(p[i],0,0); 
        }

        // print_free_alloc_list();
        // getchar();
    }
}