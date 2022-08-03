#ifndef MODEL_H
#define MODEL_H

#include "alloc.h"
#include "render.h"

float *readVecs(const char *const filename, unsigned int *size, Stack *s);
float *combineVecs(const float *const v1, const float *const v2,
	const unsigned int size, Stack *s);
Light *readLights(const char *const filename, unsigned int *size, Stack *s);

#endif
