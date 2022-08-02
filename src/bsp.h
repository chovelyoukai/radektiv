#ifndef BSP_H
#define BSP_H

#include "alloc.h"
#include "bspfile.h"

BSPLevel *loadMap(Stack *s, const char *const filename);
void convertMapVerts(BSPLevel *level);
void getRealMapIndices(Stack *s, BSPLevel *level);
BSPLevel *destroyMap(BSPLevel *level);

#endif
