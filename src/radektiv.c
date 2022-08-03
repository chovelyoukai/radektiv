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
	// long term and short term stacks;
	Stack *lts, *sts;
	lts = makeStack(DEFAULT_STACK_SIZE);
	sts = makeStack(DEFAULT_STACK_SIZE);

	initEnvGlobals(lts);
	initWindowSystem();
	Window win;
	makeWindow(&win, eg.winWidth, eg.winHeight, eg.gameName);

	Renderer r;
	if (!initRenderer(&r, lts))
		return -1;

	RenderObject scene[1];

	unsigned int vertSize;
	float *mapVerts = readVecs("models/map.verts", &vertSize, sts);
	unsigned int normSize;
	float *mapNorms = readVecs("models/map.norms", &normSize, sts);
	if (!mapVerts || !mapNorms)
	{
		return -1;
	}
	float *mapModel = combineVecs(mapVerts, mapNorms, vertSize, lts);
	scene[0] = makeRenderObject(mapModel, vertSize + normSize);

	unsigned int numLights;
	Light *lights = readLights("models/map.lights", &numLights, lts);

	mat4x4 proj;
	mat4x4_perspective(proj, 1.5f, getAspectRatio(&win), 0.1f, 2048.0f);
	initCamera(&(r.cam), 0.1f, 2048.0f, 1.5f);

	vec3 forward;
	vec3 right;
	vec3 up;
	vec3 eye = {128.0f, 128.0f, 128.0f};
	float pitch, yaw;
	pitch = yaw = 0.0f;

	while (!shouldWindowClose(&win))
	{
		float xDelta, yDelta;
		getMousePos(&win, &xDelta, &yDelta);
		updateViewAngles(xDelta, yDelta, &pitch, &yaw, forward, right, up);
		updateViewPos(getInputs(&win), forward, right, eye);

		mat4x4 view;
		vec3 center;
		vec3_add(center, eye, forward);
		mat4x4_look_at(view, eye, center, up);

		updateCamera(&(r.cam), eye, forward, right, up);
		drawScene(&r, scene, 1, lights, numLights, view, proj);

		stclear(sts);
		updateWindow(&win);
	}

	destroyWindow(&win);
	shutdownWindowSystem();
	destroyStack(sts);
	destroyStack(lts);
	return 0;
}

const float MAX_PITCH = (M_PI / 2.0f) - 0.1f;
const float YAW_MOD = M_PI * 2.0f;

void updateViewAngles(float xDelta, float yDelta, float *pitch, float *yaw,
	vec3 forward, vec3 right, vec3 up)
{
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
	vec3 absoluteUp = {0.0f, 1.0f, 0.0f};
	vec3_mul_cross(right, forward, absoluteUp);
	vec3_norm(right, right);
	vec3_mul_cross(up, right, forward);

	*yaw = newYaw;
	*pitch = newPitch;
}

void updateViewPos(unsigned int inputs, vec3 forward, vec3 right, vec3 eye)
{
		vec3 moveForward, moveRight;
		vec3_scale(moveForward, forward, 3.0f);
		vec3_scale(moveRight, right, 3.0f);

		if (inputs & IN_FORWARD)
			vec3_add(eye, eye, moveForward);
		if (inputs & IN_BACKWARD)
			vec3_sub(eye, eye, moveForward);
		if (inputs & IN_LEFT)
			vec3_sub(eye, eye, moveRight);
		if (inputs & IN_RIGHT)
			vec3_add(eye, eye, moveRight);
}
