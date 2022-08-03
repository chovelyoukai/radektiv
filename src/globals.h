#ifndef GLOBALS_H
#define GLOBALS_H

#include "alloc.h"

typedef struct
{
	int winWidth;
	int winHeight;
	int maxFramerate;
	char *gameName;
	int doSSAO;
	int doLight;
} EnvGlobals;

extern EnvGlobals eg;

void initEnvGlobals(Stack *s);

#endif
