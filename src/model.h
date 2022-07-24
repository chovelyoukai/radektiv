#ifndef MODEL_H
#define MODEL_H

#include "render.h"

float *readVecs(const char *const filename, unsigned int *size);
float *combineVecs(const float *const v1, const float *const v2,
	const unsigned int size);
Light *readLights(const char *const filename, unsigned int *size);

#endif
