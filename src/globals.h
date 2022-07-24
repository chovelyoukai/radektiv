#ifndef GLOBALS_H
#define GLOBALS_H

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

void initEnvGlobals(void);
void destroyEnvGlobals(void);

#endif
