#ifndef __MEM_H_
#define __MEM_H_


void *head_ptr;

void *Mem_Init(int sizeOfRegion);
void *Mem_Alloc(int size, int expand);
int Mem_Free(void *ptr, int coalesce, int release);
void Mem_Dump();

#endif