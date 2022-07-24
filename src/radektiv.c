#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "inputs.h"
#include "model.h"
#include "radektiv.h"
#include "render.h"
#include "window.h"

int main(int argc, char **argv)
{
	initEnvGlobals();
	initWindowSystem();
	createWindow(eg.winWidth, eg.winHeight, eg.gameName);

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

	vec3 forward;
	vec3 right;
	vec3 up = {0.0f, 1.0f, 0.0f};
	vec3 eye = {128.0f, 128.0f, 128.0f};
	float pitch, yaw;
	pitch = yaw = 0.0f;

	while (!shouldWindowClose())
	{
		updateViewAngles(&pitch, &yaw, forward, right);
		updateViewPos(forward, right, eye);

		mat4x4 view;
		vec3 center;
		vec3_add(center, eye, forward);
		mat4x4_look_at(view, eye, center, up);

		mat4x4 proj;
		mat4x4_perspective(proj, 1.5f, getAspectRatio(), 0.1f, 2048.0f);

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

const float MAX_PITCH = (M_PI / 2.0f) - 0.1f;
const float YAW_MOD = M_PI * 2.0f;

void updateViewAngles(float *pitch, float *yaw, vec3 forward, vec3 right)
{
	// update view angles
	float xDelta, yDelta;
	getMousePos(&xDelta, &yDelta);

	float newYaw = *yaw + xDelta / eg.winWidth;
	while (newYaw > YAW_MOD) {newYaw -= YAW_MOD;}
	while (newYaw < 0.0f) {newYaw += YAW_MOD;}

	float newPitch = *pitch - yDelta / eg.winHeight;
	if (newPitch >= MAX_PITCH)
		newPitch = MAX_PITCH;
	if (newPitch <= -MAX_PITCH)
		newPitch = -MAX_PITCH;

	forward[0] = cosf(newYaw) * cosf(newPitch);
	forward[2] = sinf(newYaw) * cosf(newPitch);
	forward[1] = sinf(newPitch);
	vec3_norm(forward, forward);
	vec3 up = {0.0f, 1.0f, 0.0f};
	vec3_mul_cross(right, forward, up);

	*yaw = newYaw;
	*pitch = newPitch;
}

void updateViewPos(vec3 forward, vec3 right, vec3 eye)
{
		vec3 moveForward, moveRight;
		vec3_scale(moveForward, forward, 3.0f);
		vec3_scale(moveRight, right, 3.0f);

		// update eye pos
		unsigned int inputs = getInputs();
		if (inputs & IN_FORWARD)
			vec3_add(eye, eye, moveForward);
		if (inputs & IN_BACKWARD)
			vec3_sub(eye, eye, moveForward);
		if (inputs & IN_LEFT)
			vec3_sub(eye, eye, moveRight);
		if (inputs & IN_RIGHT)
			vec3_add(eye, eye, moveRight);
}
