#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mem.h"

struct queue_element {
    struct queue_element *next;
    void *ptr;
};

struct queue_element *head=NULL, *tail=NULL;

void print_requests_queue();
void rigorous_testing();
void read_file_process_requests(char *filename,  int expand, int coalesce, int release);

int main(int argc, char *argv[])
{

    if(argc<2)
    {
        printf("Pleasep provide number of pages to initialize in cmd line argument. \n");;
        return 0;
    }

    int user_entered_size = atoi(argv[1]);
    if(user_entered_size <= 0)
    {
        printf("Please enter size greater than zero.\n");
        return 0;
    }

    //calculate the page aligned size
    int page_size = sysconf(_SC_PAGE_SIZE);
    int new_mem_size = page_size + user_entered_size;
    int a = new_mem_size % page_size;
    int page_aligned_size = new_mem_size - a;


    void *head = Mem_Init(page_aligned_size); 
    if(head == NULL)
    {
        printf("Mem_Init() return NULL.\n");
        return 0;
    }

    char *filename = "test3.txt";
    read_file_process_requests(filename, 1,1,1);

    Mem_Dump();

    return 0;
}

void read_file_process_requests(char *filename, int expand, int coalesce, int release)
{
    FILE *file = fopen(filename, "r");
    if(file == NULL)
    {    
        printf("Error opening the file: \"%s\"\n", filename);
        return;
    }

    char line[12];
    fgets(line, 12, file);
    // int totalUsers = atoi(line);

    while(fgets(line, 12, file) != NULL)
    {
        strtok(line, "\n"); //remove '\n' from the string

        if(line[0]=='1' && line[1]=='9')
        {    
            char *rollnumber_str = strtok(line, " ");
            // long int rollnuber = atoi(tok1);
            char *tok = strtok(NULL, " ");
            short number_of_requests = atoi(tok);
            
            int memory_required =  sizeof(short) + 8 + number_of_requests*9 + sizeof(struct queue_element);

            void *ptr = Mem_Alloc(memory_required, expand);


            if(ptr == NULL)
            {
                printf("Main(): Mem_Alloc returned NULL for size (%d), request (%d - %s),\n", memory_required, number_of_requests, line);
                continue;
            }

            //adding element to the queue
            struct queue_element *node_ptr = (struct queue_element *)ptr;
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

            //copying the number of request to the memory
            short *req_ptr_1 = (short *)cur_ptr;
            *req_ptr_1 = number_of_requests;
            cur_ptr +=2; // for short
            
            //copying the rollnumber to the memory
            strcpy(cur_ptr, rollnumber_str);
            cur_ptr += 8;

            //copying the requests to the memory
            char *req_ptr = (char *)cur_ptr;
            for(int i=0; i<number_of_requests; i++)
            {
                fgets(line, 9, file);
                strtok(line, "\n");
                strcpy(req_ptr, line);
                req_ptr +=9;
            }
        }
        else if (line[0] == '!')
        {
            // Dequue the request from the queue if possible
            if(head!=NULL && tail!=NULL)
            {
                short req_pending = *((short *)head->ptr);
                
                //its time to remove the user with all its requests processed
                if(req_pending<=1)
                {
                    void *ptr = head;
                    head = head->next;
                    int return_value = Mem_Free(ptr, coalesce, release);

                    if(return_value == -1)
                        printf("Mem_Free(): Could not free the memory.");
                }
                //process the request and decrement it's count
                else
                {
                    //TODO: process the request
                    req_pending--;
                    *((short *)head->ptr) = req_pending;
                }
            }
        }
    }

    // print_requests_queue();    
}

void print_requests_queue()
{
    struct queue_element *cur = head;
    printf("----------------------head of queue: (%p)----------------------\n", head);

    for(int i=0; i<1000 && cur != NULL; i++)
    {

        void *ptr = (void *)cur->ptr;

        short req_pending = *((short *)cur->ptr);
        printf("addr: %p, next: %p, ptr: %p, ", cur, cur->next, cur->ptr);

        printf("(");
        char *str_ptr = ptr + 2;
        for(int k=0; k<8; k++)
        {
            printf("%c", str_ptr[k]);
        }
        printf("% d", req_pending);
        printf(")");
        printf("\n");
        cur = cur->next;
    }
    printf("---------------------------------------------------------------------------\n");
}

void rigorous_testing()
{
    void* p[100];

    for(int j = 0 ; j < 10; j ++ )
    {
        for(int i = 0; i < 100; i++)
        {
            int c = random();
            c = c % 100;
            p[i] = Mem_Alloc(c,1);
        }

        for(int i = 99; i >= 0; i--)
        {
            Mem_Free(p[i],1,1); 
        }
    }
}