#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "model.h"
#include "radmath.h"
#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float squareVerts[] =
{
	 // positions        // normals
	 1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	 1.0f, -1.0f,  0.0f, 0.0f, 0.0f, -1.0f,

	-1.0f, -1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	 1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
};

float errTex[] =
{
	1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
};

bool initRenderer(Renderer *r, Stack *s)
{
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
		return false;

	if (!loadShaderProgram("shaders/geom_vs.glsl", NULL, "shaders/geom_fs.glsl",
		&(r->Progs.geom)))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", NULL, "shaders/ssao_fs.glsl",
		&(r->Progs.ssao)))
		return false;

	if (!loadShaderProgram("shaders/light_vs.glsl", NULL, "shaders/light_fs.glsl",
		&(r->Progs.light)))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", NULL, "shaders/ambient_fs.glsl",
		&(r->Progs.ambient)))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", NULL, "shaders/blur_fs.glsl",
		&(r->Progs.blur)))
		return false;

	if (!loadShaderProgram("shaders/shadow_vs.glsl", "shaders/shadow_gs.glsl",
		"shaders/shadow_fs.glsl", &(r->Progs.shadow)))
		return false;

	setupAoBuffer(r);
	setupGBuffer(r);
	setupShadowBuffer(r);

	r->Objects.screen = makeRenderObject(squareVerts, (sizeof squareVerts) / sizeof(float));
	printf("%lu\n", sizeof squareVerts);

	r->s = s;
	unsigned int lvc;
	float *lightVerts = readVecs("models/light.verts", &lvc, s);
	unsigned int lnc;
	float *lightNorms = readVecs("models/light.norms", &lnc, s);
	float *lightModel = combineVecs(lightVerts, lightNorms, lvc, s);
	r->Objects.light = makeRenderObject(lightModel, lvc + lnc);

	int w, h, nChan;
	unsigned char *data = stbi_load("textures/error.png", &w, &h, &nChan, 0);

	glGenTextures(1, &(r->Tex.error));
	glBindTexture(GL_TEXTURE_2D, r->Tex.error);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, errTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return true;
}

void initCamera(Camera *cam, float near, float far, float fov)
{
	cam->near = near;
	cam->far = far;
	cam->fov = fov;
	cam->tanFov = tanf(fov / 2.0f);
	cam->factorY = cosf(cam->fov / 2.0f);
	cam->factorX = cosf(atanf((cam->tanFov * eg.winWidth) / eg.winHeight));
}

void updateCamera(Camera *cam, vec3 eye, vec3 forward, vec3 right, vec3 up)
{
	vec3_dup(cam->eye, eye);
	vec3_dup(cam->forward, forward);
	vec3_dup(cam->right, right);
	vec3_dup(cam->up, up);
}

void setupAoBuffer(Renderer *r)
{
	for (int i = 0; i < SSAO_KERN_SIZE; i++)
	{
		r->ssaoKern[i][0] = randFloat() * 2.0f - 1.0f;
		r->ssaoKern[i][1] = randFloat() * 2.0f - 1.0f;
		r->ssaoKern[i][2] = randFloat() * 2.0f - 1.0f;

		vec3_norm(r->ssaoKern[i], r->ssaoKern[i]);
		vec3_scale(r->ssaoKern[i], r->ssaoKern[i], randFloat());
	}

	for (int i = 0; i < 16; i++)
	{
		r->ssaoNoise[i][0] = randFloat() * 2.0f - 1.0f;
		r->ssaoNoise[i][1] = randFloat() * 2.0f - 1.0f;
		r->ssaoNoise[i][2] = 0.0f;
	}
	
	glGenFramebuffers(1, &(r->FBOs.ssao));
	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.ssao);
	r->Tex.ssao = generateTexture(GL_DEPTH_COMPONENT, eg.winWidth, eg.winHeight,
		GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		r->Tex.ssao, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenFramebuffers(1, &(r->FBOs.ssaoBlur));
	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.ssaoBlur);
	r->Tex.ssaoBlur = generateTexture(GL_DEPTH_COMPONENT, eg.winWidth, eg.winHeight,
		GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		r->Tex.ssaoBlur, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// 4x4 noise texture for smoother AO
	glGenTextures(1, &(r->Tex.ssaoNoise));
	glBindTexture(GL_TEXTURE_2D, r->Tex.ssaoNoise);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT,
		&(r->ssaoNoise[0]));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
}

void setupGBuffer(Renderer *r)
{
	glGenFramebuffers(1, &(r->FBOs.g));
	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.g);

	// generate textures for pos, normal, color 
	r->Tex.gPosition = generateTexture(GL_RGBA16F, eg.winWidth, eg.winHeight,
		GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		r->Tex.gPosition, 0);

	r->Tex.gNormal = generateTexture(GL_RGBA16F, eg.winWidth, eg.winHeight,
		GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		r->Tex.gNormal, 0);

	r->Tex.gAlbedo = generateTexture(GL_RGBA, eg.winWidth, eg.winHeight, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
		r->Tex.gAlbedo, 0);

	unsigned int attachments[3] = {
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, attachments);

	// generate an RBO to store depth info
	unsigned int gRbo;
	glGenRenderbuffers(1, &gRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, gRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, eg.winWidth,
		eg.winHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, gRbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		fprintf(stderr, "FBO incomplete.\n");
}

static const int SHADOW_WIDTH = 1024;
static const int SHADOW_HEIGHT = 1024;

void setupShadowBuffer(Renderer *r)
{
	glGenFramebuffers(1, &(r->FBOs.shadow));
	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.shadow);

	glGenTextures(1, &(r->Tex.shadowMap));
	glBindTexture(GL_TEXTURE_CUBE_MAP, r->Tex.shadowMap);
	for (unsigned int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, r->Tex.shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}

unsigned int generateTexture(GLint internalFormat, int width, int height,
	GLenum format, GLenum type, GLint min, GLint mag, GLint wrapS, GLint wrapT)
{
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
		type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	return tex;
}

RenderObject makeRenderObject(float *verts, unsigned int size)
{
	RenderObject ro;
	ro.verts = verts;
	ro.vertCount = size / 6;

	glGenVertexArrays(1, &(ro.vao));
	glBindVertexArray(ro.vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), verts,
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
		(void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	ro.indexed = false;
	ro.origin[0] = ro.origin[1] = ro.origin[2] = 0.0f;
	ro.rotation[0] = ro.rotation[1] = ro.rotation[2] = 0.0f;
	ro.scale = 1.0f;

	return ro;
}

RenderObject makeRenderObjectBSP(BSPLevel *level)
{
	unsigned int numVerts = level->header->lumps[LUMP_VERTS].length /
		sizeof(BSPVert);
	unsigned int numIndices = level->realIndexCount;

	RenderObject ro;
	ro.verts = NULL;
	ro.vertCount = numVerts;
	ro.indexCount = numIndices;

	glGenVertexArrays(1, &(ro.vao));
	glBindVertexArray(ro.vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(BSPVert), level->verts,
		GL_STATIC_DRAW);

	unsigned int stride = sizeof(BSPVert);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *)(7 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 4, GL_BYTE, GL_FALSE, stride, (void *)(10 * sizeof(float)));
	glEnableVertexAttribArray(4);

	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(BSPIndex), level->realIndices,
		GL_STATIC_DRAW);

	ro.indexed = true;
	ro.origin[0] = ro.origin[1] = ro.origin[2] = 0.0f;
	ro.rotation[0] = ro.rotation[1] = ro.rotation[2] = 0.0f;
	ro.scale = 1.0f;

	return ro;
}

void useProg(Renderer *r, unsigned int prog)
{
	glUseProgram(prog);
	r->Progs.current = prog;
}

void drawScene(Renderer *r, RenderObject *scene, unsigned int objects,
	Light *lights, unsigned int lightCount, mat4x4 view, mat4x4 proj)
{
	// first draw all ROs to g buffer
	useProg(r, r->Progs.geom);

	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.g);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glDepthMask(GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);

	const float *projPtr = &proj[0][0];
	glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "proj"),
		1, GL_FALSE, projPtr);
	const float *viewPtr = &view[0][0];
	glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "view"),
		1, GL_FALSE, viewPtr);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r->Tex.error);

	for (int i = 0; i < objects; i++)
	{
		drawRenderObject(r, scene[i]);
	}

	// setup for SSAO
	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.ssao);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// calculate AO
	if (eg.doSSAO)
	{
		drawSSAO(r, view, proj);
		blurSSAO(r);
	}

	// set up for lighting passes
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glDepthMask(GL_FALSE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	// do ambient lighting
	drawAmbient(r);

	// then draw lights
	if (eg.doLight)
	{
		drawLights(r, scene, objects, lights, lightCount, view, proj);
	}
}

void makeModelMatrix(mat4x4 model, vec3 origin, vec3 rotation, float scale)
{
	mat4x4_identity(model);
	mat4x4_rotate(model, model, 1.0f, 0.0f, 0.0f, rotation[0]);
	mat4x4_rotate(model, model, 0.0f, 1.0f, 0.0f, rotation[1]);
	mat4x4_rotate(model, model, 0.0f, 0.0f, 1.0f, rotation[2]);
	mat4x4_translate_in_place(model, origin[0], origin[1], origin[2]);
	mat4x4_scale_aniso(model, model, scale, scale, scale);
}

void drawRenderObject(Renderer *r, RenderObject ro)
{
	glBindVertexArray(ro.vao);

	mat4x4 model;
	makeModelMatrix(model, ro.origin, ro.rotation, ro.scale);
	const float *modelPtr = &model[0][0];
	glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "model"),
		1, GL_FALSE, modelPtr);

	mat4x4 modelInverted;
	mat4x4_invert(modelInverted, model);
	mat4x4 normal;
	mat4x4_transpose(normal, modelInverted);
	const float *normalPtr = &normal[0][0];
	glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "normal"),
		1, GL_FALSE, normalPtr);

	if (ro.indexed)
	{
		glDrawElements(GL_TRIANGLES, ro.indexCount, GL_UNSIGNED_INT, NULL);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, ro.vertCount);
	}
}

void drawSSAO(Renderer *r, mat4x4 view, mat4x4 proj)
{
	const float *projPtr = &proj[0][0];
	const float *viewPtr = &view[0][0];

	useProg(r, r->Progs.ssao);
	glUniform1i(glGetUniformLocation(r->Progs.current, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(r->Progs.current, "ssaoNoise"), 1);
	glUniform2f(glGetUniformLocation(r->Progs.current, "screenSize"),
		eg.winWidth, eg.winHeight);
	glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "proj"),
		1, GL_FALSE, projPtr);
	glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "view"),
		1, GL_FALSE, viewPtr);
	const float *ssaoKernPtr = &(r->ssaoKern[0][0]);
	glUniform3fv(glGetUniformLocation(r->Progs.current, "samples"),
			SSAO_KERN_SIZE, ssaoKernPtr);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r->Tex.gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, r->Tex.ssaoNoise);

	drawRenderObject(r, r->Objects.screen);
}

void blurSSAO(Renderer *r)
{
	glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.ssaoBlur);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	useProg(r, r->Progs.blur);
	glUniform1i(glGetUniformLocation(r->Progs.current, "ssaoTex"), 0);
	glUniform2f(glGetUniformLocation(r->Progs.current, "screenSize"),
		eg.winWidth, eg.winHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r->Tex.ssao);

	drawRenderObject(r, r->Objects.screen);
}

void drawAmbient(Renderer *r)
{
	useProg(r, r->Progs.ambient);
	glUniform1i(glGetUniformLocation(r->Progs.current, "ssaoTex"), 0);
	glUniform1i(glGetUniformLocation(r->Progs.current, "gAlbedo"), 1);
	glUniform2f(glGetUniformLocation(r->Progs.current, "screenSize"),
		eg.winWidth, eg.winHeight);
	glUniform1f(glGetUniformLocation(r->Progs.current, "defaultAo"),
		eg.doSSAO ? 1.0f : -1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r->Tex.ssaoBlur);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, r->Tex.gAlbedo);

	drawRenderObject(r, r->Objects.screen);
}

bool isSphereVisible(Camera cam, vec3 origin, float radius, mat4x4 view, mat4x4 proj)
{
	vec3 toPoint;
	vec3_sub(toPoint, origin, cam.eye);

	float pointZ = vec3_mul_inner(toPoint, cam.forward);
	if (pointZ > cam.far + radius || pointZ < cam.near - radius)
		return false;

	float pointY = vec3_mul_inner(toPoint, cam.up);
	float aux = pointZ * cam.tanFov;
	float distance = radius / cam.factorY;
	if (pointY > aux + distance || pointY < -aux - distance)
		return false;

	float pointX = vec3_mul_inner(toPoint, cam.right);
	distance = radius / cam.factorX;
	aux = (aux * eg.winWidth) / eg.winHeight;
	if (pointX > aux + distance || pointX < -aux - distance)
		return false;

	return true;
}

void drawLights(Renderer *r, RenderObject *scene, unsigned int objects,
	Light *lights, unsigned int lightCount, mat4x4 view, mat4x4 proj)
{
	const float *projPtr = &proj[0][0];
	const float *viewPtr = &view[0][0];

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// if we aren't going to draw shadows, just set this up once
	if (!eg.doShadows)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.shadow);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

		glDepthMask(GL_TRUE);

		glClear(GL_DEPTH_BUFFER_BIT);
	}

	for (int n = 0; n < lightCount; n++)
	{
		float c = lights[n].constant / 1000.0f;
		float l = lights[n].linear / 1000.0f;
		float q = lights[n].quad / 1000.0f;
		float radius = -l + (sqrtf((l * l) - (4 * q * (c - 256.0f / 4.0f))) / (2 * q));

		if (!isSphereVisible(r->cam, lights[n].origin, radius, view, proj))
			continue;

		// first draw shadows
		if (eg.doShadows)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, r->FBOs.shadow);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

			glDepthMask(GL_TRUE);

			glClear(GL_DEPTH_BUFFER_BIT);

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			useProg(r, r->Progs.shadow);

			mat4x4 lightView[6];
			const float *lightViewPtr = &lightView[0][0][0];
			vec3 center;
			vec3_add(center, lights[n].origin, (vec3){1.0f, 0.0f, 0.0f});
			mat4x4_look_at(lightView[0], lights[n].origin, center, (vec3){0.0f, -1.0f, 0.0f});
			vec3_add(center, lights[n].origin, (vec3){-1.0f, 0.0f, 0.0f});
			mat4x4_look_at(lightView[1], lights[n].origin, center, (vec3){0.0f, -1.0f, 0.0f});
			vec3_add(center, lights[n].origin, (vec3){0.0f, 1.0f, 0.0f});
			mat4x4_look_at(lightView[2], lights[n].origin, center, (vec3){0.0f, 0.0f, 1.0f});
			vec3_add(center, lights[n].origin, (vec3){0.0f, -1.0f, 0.0f});
			mat4x4_look_at(lightView[3], lights[n].origin, center, (vec3){0.0f, 0.0f, -1.0f});
			vec3_add(center, lights[n].origin, (vec3){0.0f, 0.0f, 1.0f});
			mat4x4_look_at(lightView[4], lights[n].origin, center, (vec3){0.0f, -1.0f, 0.0f});
			vec3_add(center, lights[n].origin, (vec3){0.0f, 0.0f, -1.0f});
			mat4x4_look_at(lightView[5], lights[n].origin, center, (vec3){0.0f, -1.0f, 0.0f});

			mat4x4 lightProj;
			const float *lightProjPtr = &lightProj[0][0];
			mat4x4_perspective(lightProj, M_PI / 2.0f, 1.0f, r->cam.near, r->cam.far);

			glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "lightView"),
					6, GL_FALSE, lightViewPtr);
			glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "lightProj"),
					1, GL_FALSE, lightProjPtr);
			glUniform3fv(glGetUniformLocation(r->Progs.current, "lightPos"), 1,
					lights[n].origin);
			glUniform1f(glGetUniformLocation(r->Progs.current, "far"), r->cam.far);

			for (int i = 0; i < objects; i++)
			{
				drawRenderObject(r, scene[i]);
			}
		}

		// then draw lights
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, eg.winWidth, eg.winHeight);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		useProg(r, r->Progs.light);

		glUniform1i(glGetUniformLocation(r->Progs.current, "gPosition"), 0);
		glUniform1i(glGetUniformLocation(r->Progs.current, "gNormal"), 1);
		glUniform1i(glGetUniformLocation(r->Progs.current, "gAlbedo"), 2);
		glUniform1i(glGetUniformLocation(r->Progs.current, "shadowMap"), 3);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, r->Tex.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, r->Tex.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, r->Tex.gAlbedo);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, r->Tex.shadowMap);

		glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "proj"),
			1, GL_FALSE, projPtr);
		glUniformMatrix4fv(glGetUniformLocation(r->Progs.current, "view"),
			1, GL_FALSE, viewPtr);
		glUniform2f(glGetUniformLocation(r->Progs.current, "screenSize"),
			eg.winWidth, eg.winHeight);

		glUniform1f(glGetUniformLocation(r->Progs.current, "constant"), c);
		glUniform1f(glGetUniformLocation(r->Progs.current, "linear"), l);
		glUniform1f(glGetUniformLocation(r->Progs.current, "quad"), q);
		glUniform1f(glGetUniformLocation(r->Progs.current, "far"), r->cam.far);
		glUniform3fv(glGetUniformLocation(r->Progs.current, "lightPos"), 1,
			lights[n].origin);

		r->Objects.light.scale = radius;
		vec3_dup(r->Objects.light.origin, lights[n].origin);

		drawRenderObject(r, r->Objects.light);
	}
}

bool loadShaderProgram(const char *const vsName, const char *const gsName,
	const char *const fsName, unsigned int *prog)
{
	unsigned int vs, gs, fs;
	if (!loadShader(vsName, GL_VERTEX_SHADER, &vs) ||
		!loadShader(fsName, GL_FRAGMENT_SHADER, &fs))
	{
		return false;
	}

	if (gsName)
	{
		if (!loadShader(gsName, GL_GEOMETRY_SHADER, &gs))
			return false;
	}
	else
	{
		gs = 0;
	}

	*prog = glCreateProgram();
	glAttachShader(*prog, vs);
	glAttachShader(*prog, fs);
	if (gs)
	{
		glAttachShader(*prog, gs);
	}
	glLinkProgram(*prog);

	glDeleteShader(vs);
	glDeleteShader(fs);
	if (gs)
	{
		glDeleteShader(gs);
	}

	int success;
	glGetProgramiv(*prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		char log[SHADER_SIZE];
		glGetProgramInfoLog(*prog, SHADER_SIZE, NULL, log);
		fprintf(stderr, "Could not link program.\n%s", log);
		return false;
	}

	return true;
}

bool loadShader(const char *const filename, const GLenum type,
	unsigned int *shader)
{
	char text[SHADER_SIZE];
	if (!readFile(filename, text, SHADER_SIZE))
	{
		fprintf(stderr, "Could not load shader %s\n", filename);
	}

	*shader = glCreateShader(type);
	const char *const textPtr = text;
	glShaderSource(*shader, 1, &textPtr, NULL);
	glCompileShader(*shader);

	int success;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char log[SHADER_SIZE];
		glGetShaderInfoLog(*shader, SHADER_SIZE, NULL, log);
		fprintf(stderr, "Could not compile shader %s\n%s", filename, log);
		return false;
	}

	return true;
}

bool readFile(const char *const filename, char *buffer, size_t size)
{
	if (!filename || !buffer)
		return false;

	FILE *filePtr = fopen(filename, "r");
	if (!filePtr)
		return false;

	int read = fread(buffer, sizeof(char), size, filePtr);
	int err = ferror(filePtr);
	fclose(filePtr);

	if (read >= size || err)
		return false;

	buffer[read] = '\0';
	if (buffer[read-1] == '\n')
		buffer[read-1] = '\0';

	return true;
}
