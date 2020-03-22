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


int main()
{
    struct header **free_list = (struct header **)Mem_Init(1*sysconf(_SC_PAGE_SIZE));
    struct header **allo_list = free_list +1;

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
            
            if(ptr == NULL)
            {
                printf("Main(): Mem_Alloc returned NULL\n");
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

            // strcpy(cur_ptr, tok);
            // cur_ptr +=1;

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
            // Dequue the request from the queue
            if(!(head==NULL && tail==NULL))
            {
                char *str_ptr = head->ptr;
                short req_pending = *((short *)head->ptr);
                // printf("%d-", req_pending);

                // char *str = str_ptr+2;
                // for(int k=0; k<8; k++)
                // {
                //     printf("%c", str[k]);
                // }
                // printf("\n");
                
                if(req_pending<=1)
                {
                    void *ptr = head;
                    head = head->next;
                    // Mem_Free(ptr,0,0);
                }
                else
                {
                    req_pending--;
                    *((short *)head->ptr) = req_pending;
                }
            }
        }
    }

    // print_free_alloc_list(*free_list, *allo_list);

    struct queue_element *cur = head;
    for(int i=0; i<48 && cur != NULL; i++)
    {
        void *ptr = (void *)cur->ptr;

        short req_pending = *((short *)cur->ptr);
        printf("%d-", req_pending);

        char *str_ptr = ptr + 2;
        for(int k=0; k<8; k++)
        {
            printf("%c", str_ptr[k]);
        }
        printf("\n");
        cur = cur->next;
    }

    return 0;
}