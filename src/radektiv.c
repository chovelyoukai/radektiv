#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "linmath.h"
#include "model.h"
#include "render.h"
#include "window.h"

int main(int argc, char **argv)
{
	initEnvGlobals();
	initWindowSystem();
	createWindow(eg.winWidth, eg.winHeight, eg.gameName);
	releaseMouse();

	if (!initRenderer())
		return -1;

	RenderObject scene[1];

	unsigned int vertSize;
	float *mapVerts = readVecs("models/map.verts", &vertSize);
	unsigned int normSize;
	float *mapNorms = readVecs("models/map.norms", &normSize);
	if (!mapVerts || !mapNorms)
	{
		return -1;
	}
	float *mapModel = combineVecs(mapVerts, mapNorms, vertSize);
	free(mapVerts);
	free(mapNorms);
	scene[0] = makeRenderObject(mapModel, vertSize + normSize);

	unsigned int numLights;
	Light *lights = readLights("models/map.lights", &numLights);

	while (!shouldWindowClose())
	{
		mat4x4 view;
		vec3 eye = {128.0f, 128.0f, 128.0f};
		vec3 center = {0.0f, 0.0f, 0.0f};
		vec3 up = {0.0f, 1.0f, 0.0f};
		mat4x4_look_at(view, eye, center, up);

		mat4x4 proj;
		mat4x4_perspective(proj, 1.5f, getAspectRatio(), 0.1f, 500.0f);

		for (int i = 0; i < numLights; i++)
		{
			lights[i].origin[0] += 0.5f * cosf(getTime());
			lights[i].origin[1] += 1.0f * sinf(getTime());
			lights[i].origin[2] += 0.5f * sinf(getTime());
		}

		drawScene(scene, 1, lights, numLights, view, proj);

		updateWindow();
	}

	free(scene[0].verts);
	destroyWindow();
	destroyEnvGlobals();
	return 0;
}

