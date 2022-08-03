#ifndef ALLOC_H
#define ALLOC_H

#define DEFAULT_STACK_SIZE (1024 * 1024 * 64)

typedef struct
{
	char *mem;
	char *top;
	size_t size;
} Stack;

Stack *makeStack(size_t size);
Stack *destroyStack(Stack *s);
void *stalloc(Stack *s, size_t size);
void stclear(Stack *s);

#endif
