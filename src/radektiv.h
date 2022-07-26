#ifndef RADEKTIV_H
#define RADEKTIV_H

#include "linmath.h"

void updateViewAngles(float xDelta, float yDelta, float *pitch, float *yaw,
	vec3 forward, vec3 right, vec3 up);
void updateViewPos(unsigned int inputs, vec3 forward, vec3 right, vec3 eye);

#endif
