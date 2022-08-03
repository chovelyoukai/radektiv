#include <stdlib.h>

#include "alloc.h"

Stack *makeStack(size_t size)
{
	Stack *s = malloc(sizeof(Stack));
	if (!s)
		return NULL;

	s->mem = s->top = malloc(size);
	if (!s->mem)
	{
		free(s);
		return NULL;
	}

	s->size = size;

	return s;
}

Stack *destroyStack(Stack *s)
{
	free(s->mem);
	free(s);
	return NULL;
}

void *stalloc(Stack *s, size_t size)
{
	if (s->top + size > s->mem + s->size)
		return NULL;

	void *last = s->top;
	s->top += size;
	return last;
}

void stclear(Stack *s)
{
	s->top = s->mem;
}
