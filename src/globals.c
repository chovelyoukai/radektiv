#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int errno;

#include "globals.h"

EnvGlobals eg;

static int loadEnvVarI(const char *const name, const int def)
{
	char *str = getenv(name);
	int val = def;
	if (str)
	{
		val = strtol(str, NULL, 10);
		if (errno)
		{
			val = def;
		}
	}
	return val;
}

static char* loadEnvVarS(const char *const name, char *const def,
	char **var, Stack *s)
{
	char *str = getenv(name);
	if (!str)
	{
		str = def;
	}
	*var = stalloc(s, sizeof(char) * (strlen(str) + 1));
	strcpy(*var, str);
}

void initEnvGlobals(Stack *s)
{
	eg.winWidth     = loadEnvVarI("RAD_WIDTH",      800);
	eg.winHeight    = loadEnvVarI("RAD_HEIGHT",     800);
	eg.maxFramerate = loadEnvVarI("RAD_MAX_FRAMES", 60);
	eg.doSSAO       = loadEnvVarI("RAD_DO_SSAO",    1);
	eg.doLight      = loadEnvVarI("RAD_DO_LIGHT",   1);
	eg.doShadows    = loadEnvVarI("RAD_DO_SHADOWS", 1);
	loadEnvVarS("RAD_GAME_NAME", "Radektiv", &(eg.gameName), s);
}

