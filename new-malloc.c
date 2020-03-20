/**************************************************************
**
**  Auto-generated to help solve interview questions.
**
**  Question : 
    implement your own malloc and free for application x, which should
control the heap memory usage of the application x. 
**  Carreercup Link:
    careercup.com/question?id=3492344

***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#define BSIZE ((10000 * 1024))

struct packet {
    struct packet *next;
    long  psize;
    char pp[0];
};

typedef struct packet* PKT;
PKT FH = NULL;
PKT AH = NULL; 
long rc=0;

int minit()
{
    FH = (PKT) mmap( 0, BSIZE , PROT_READ| PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS , -1 , 0);
    if( FH)
    {
        printf(" Starting address %p \n", FH);
        FH->next =  NULL;
        FH->psize = BSIZE - sizeof(struct packet);
    }
    else
    {
        printf(" mmap failed \n");
        exit(1);
    }
}

void * mmalloc(int count) {
    // find a free node whose size is grater than count + sizeof(struct packet)
    PKT q, p = FH, newfreepacket, nextfreepacket;
    int req = count + sizeof(struct packet);
    if (count <= 0) {
        return NULL;
    }

    rc++;
    q = p;
    while ((p-> psize < req) && p-> next) {
        q = p;
        p = p-> next;
    }
    if (p-> psize >= req) {
        nextfreepacket = p-> next;
        if (p-> psize > req) {
            // calculate next in chain
            newfreepacket = (PKT) & (p-> pp[count]);
            newfreepacket-> psize = p-> psize - req;
            newfreepacket-> next = nextfreepacket;
            // link with previous in chain
            if (p == q) {
                FH = newfreepacket;
            } else {
                q-> next = newfreepacket;
            }
        } else {
            // Free list is loosing one node.
            if (p == q) {
                FH = nextfreepacket;
            } else {
                q-> next = nextfreepacket;
            }
        }
        // Adjust p's size
        p-> psize = count;
        // Add p to Allocated list.
        if (AH) {
            p-> next = AH;
            AH = p;
        } else {
            AH = p;
        }
        return p-> pp;
    } else {
        printf(" Requested size %d can not be allocated %ld \n", count, rc);
        return NULL;
    }
}
void mdefrag(void)
{
    PKT p, q, next;
    p = FH;
    while(p)
    {
        q = p;
        p = p->next;
        if(p)
        {
            next = (PKT) &(q->pp[q->psize]);
            // can we remove p ?
            if( next == p )
            {
                q->psize +=  p->psize + sizeof(struct packet);
                q->next = p->next;
                // delete the node
                p = q;
            }
        }
    }
}
void mfree(void *r)
{
    PKT p, q;
    // traverse the A list 
    if( r == NULL)
        return;

    if((rc % 1000) == 0)
    mdefrag();

    q = p = AH;
    while(p && (p->pp != r))
    {
        q = p;
        p = p->next;
    }
    if(p)
    {
        // first node
        if( q == p)
        {
            AH =  p->next;
        }
        // rest
        else
        {
            q->next = p->next;
        }
        // Add the node to the F list.
        p->next = FH;
        FH = p;
    }
    else
    {
        printf("Unallocated memory freed  at %p \n", r);
    }
}

int main()
{
  int i, j , c ;
  char* p[100];

  minit();

  // algo goes here.
  for( j = 0 ; j < 5000; j ++ )
  {
    for(i = 0; i < 100; i++)
    {
      c = random();
      c = c % 100;
      p[i] = mmalloc(c);
    }
    for(i = 99; i >= 0; i--)
    {
      mfree(p[i]); 
    }
  }
    printf(" AH : %p \n", AH);
    printf(" FH : %p \n", FH);
    printf(" C  : %ld\n", rc);
  return 0;
}