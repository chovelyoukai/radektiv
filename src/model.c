#include <stdio.h>
#include <stdlib.h>

#include "model.h"
#include "render.h"

static const int LINE_LEN = 64;

float *readVecs(const char *const filename, unsigned int *size)
{
	FILE *filePtr = fopen(filename, "r");
	if (!filePtr)
		return NULL;

	char line[LINE_LEN];
	fgets(line, LINE_LEN, filePtr);
	unsigned int numVecs = strtol(line, NULL, 10);

	float *vecs = malloc(sizeof(float) * numVecs);
	if (!vecs)
		return NULL;

	unsigned int i = 0;
	fgets(line, LINE_LEN, filePtr);
	while (!feof(filePtr))
	{
		vecs[i] = strtof(line, NULL);
		i++;
		fgets(line, LINE_LEN, filePtr);
	}

	*size = numVecs;
	fclose(filePtr);

	return vecs;
}

float *combineVecs(const float *const v1, const float *const v2,
	const unsigned int size)
{
	float *combined = malloc(sizeof(float) * 2 * size);
	if (!combined)
		return NULL;

	for (size_t i = 0; i < size; i += 3)
	{
		float v10 = v1[i];
		float v11 = v1[i+1];
		float v12 = v1[i+2];
		float v20 = v2[i];
		float v21 = v2[i+1];
		float v22 = v2[i+2];
		combined[(i*2)] = v10; 
		combined[(i*2)+1] = v11; 
		combined[(i*2)+2] = v12; 
		combined[(i*2)+3] = v20; 
		combined[(i*2)+4] = v21; 
		combined[(i*2)+5] = v22; 
	}

	return combined;
}

Light *readLights(const char *const filename, unsigned int *size)
{
	FILE *filePtr = fopen(filename, "r");
	if (!filePtr)
		return NULL;

	char line[LINE_LEN];
	fgets(line, LINE_LEN, filePtr);
	unsigned int numLights = strtol(line, NULL, 10);

	Light *lights = malloc(sizeof(Light) * numLights);
	if (!lights)
		return NULL;

	for (unsigned int i = 0; i < numLights; i++)
	{
		float x, y, z;
		fscanf(filePtr, "%f%f%f", &x, &y, &z);
		lights[i].origin[0] = x;
		lights[i].origin[1] = z;
		lights[i].origin[2] = -y;
		lights[i].constant  = 1.0f;
		lights[i].linear    = 0.7f;
		lights[i].quad      = 1.8f;
	}

	*size = numLights;
	fclose(filePtr);

	return lights;
}
