#ifndef RADEKTIV_H
#define RADEKTIV_H

#include "linmath.h"

void updateViewAngles(float *pitch, float *yaw, vec3 forward, vec3 right);
void updateViewPos(vec3 forward, vec3 right, vec3 eye);

#endif
