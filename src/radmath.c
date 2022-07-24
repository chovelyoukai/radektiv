#include <stdio.h>
#include <stdlib.h>

#include "radmath.h"

float randFloat()
{
	float r = (float)rand() / (float)RAND_MAX;
	return r;
}
