#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "model.h"
#include "radmath.h"
#include "render.h"

unsigned int geomProg;
unsigned int lightProg;
unsigned int ambientProg;
unsigned int ssaoProg;
unsigned int blurProg;
unsigned int currentProg;
unsigned int gBuffer;
unsigned int gPosition, gNormal, gAlbedo;
unsigned int ssaoBuffer;
unsigned int ssaoTex, ssaoNoiseTex;
unsigned int ssaoBlurBuffer;
unsigned int ssaoBlurTex;

#define SSAO_KERN_SIZE 6
vec3 ssaoKern[SSAO_KERN_SIZE];
vec3 ssaoNoise[16];

RenderObject screen;

float squareVerts[] =
{
	 // positions        // normals
	 1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	 1.0f, -1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f,
	 1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f
};

bool initRenderer(void)
{
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
		return false;

	if (!loadShaderProgram("shaders/geom_vs.glsl", "shaders/geom_fs.glsl",
		&geomProg))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", "shaders/ssao_fs.glsl",
		&ssaoProg))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", "shaders/light_fs.glsl",
		&lightProg))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", "shaders/ambient_fs.glsl",
		&ambientProg))
		return false;

	if (!loadShaderProgram("shaders/screen_vs.glsl", "shaders/blur_fs.glsl",
		&blurProg))
		return false;

	setupGBuffer();
	setupAoBuffer();

	screen = makeRenderObject(squareVerts, sizeof squareVerts);

	return true;
}

void setupAoBuffer(void)
{
	for (int i = 0; i < SSAO_KERN_SIZE; i++)
	{
		ssaoKern[i][0] = randFloat() * 2.0f - 1.0f;
		ssaoKern[i][1] = randFloat() * 2.0f - 1.0f;
		ssaoKern[i][2] = randFloat() * 2.0f - 1.0f;

		vec3_norm(ssaoKern[i], ssaoKern[i]);
		vec3_scale(ssaoKern[i], ssaoKern[i], randFloat());
	}

	for (int i = 0; i < 16; i++)
	{
		ssaoNoise[i][0] = randFloat() * 2.0f - 1.0f;
		ssaoNoise[i][1] = randFloat() * 2.0f - 1.0f;
		ssaoNoise[i][2] = 0.0f;
	}
	
	glGenFramebuffers(1, &ssaoBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);
	ssaoTex = generateTexture(GL_DEPTH_COMPONENT, eg.winWidth, eg.winHeight,
		GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		ssaoTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenFramebuffers(1, &ssaoBlurBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurBuffer);
	ssaoBlurTex = generateTexture(GL_DEPTH_COMPONENT, eg.winWidth, eg.winHeight,
		GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		ssaoBlurTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// 4x4 noise texture for smoother AO
	glGenTextures(1, &ssaoNoiseTex);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT,
		&ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
}

void setupGBuffer(void)
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// generate textures for pos, normal, color 
	gPosition = generateTexture(GL_RGBA16F, eg.winWidth, eg.winHeight,
		GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		gPosition, 0);

	gNormal = generateTexture(GL_RGBA16F, eg.winWidth, eg.winHeight,
		GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		gNormal, 0);

	gAlbedo = generateTexture(GL_RGBA, eg.winWidth, eg.winHeight, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
		gAlbedo, 0);

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

	ro.origin[0] = ro.origin[1] = ro.origin[2] = 0.0f;
	ro.rotation[0] = ro.rotation[1] = ro.rotation[2] = 0.0f;
	ro.scale = 1.0f;

	return ro;
}

void drawScene(RenderObject *scene, unsigned int objects, Light *lights,
	unsigned int lightCount, mat4x4 view, mat4x4 proj)
{
	// first draw all ROs to g buffer
	glUseProgram(geomProg);
	currentProg = geomProg;

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glDepthMask(GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);

	const float *projPtr = &proj[0][0];
	glUniformMatrix4fv(glGetUniformLocation(currentProg, "proj"),
		1, GL_FALSE, projPtr);
	const float *viewPtr = &view[0][0];
	glUniformMatrix4fv(glGetUniformLocation(currentProg, "view"),
		1, GL_FALSE, viewPtr);

	for (int i = 0; i < objects; i++)
	{
		drawRenderObject(scene[i]);
	}

	// setup for SSAO
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// calculate AO
	if (eg.doSSAO)
	{
		drawSSAO(view, proj);
		blurSSAO();
	}

	// set up for lighting passes
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glDepthMask(GL_FALSE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	// do ambient lighting
	drawAmbient();

	// then draw lights
	if (eg.doLight)
	{
		drawLights(lights, lightCount);
	}
}

void drawRenderObject(RenderObject ro)
{
	glBindVertexArray(ro.vao);

	mat4x4 model;
	mat4x4_identity(model);
	mat4x4_scale_aniso(model, model, ro.scale, ro.scale, ro.scale);
	mat4x4_rotate(model, model, 1.0f, 0.0f, 0.0f, ro.rotation[0]);
	mat4x4_rotate(model, model, 0.0f, 1.0f, 0.0f, ro.rotation[1]);
	mat4x4_rotate(model, model, 0.0f, 0.0f, 1.0f, ro.rotation[2]);
	mat4x4_translate_in_place(model, ro.origin[0], ro.origin[1], ro.origin[2]);
	const float *modelPtr = &model[0][0];
	glUniformMatrix4fv(glGetUniformLocation(currentProg, "model"),
		1, GL_FALSE, modelPtr);

	mat4x4 modelInverted;
	mat4x4_invert(modelInverted, model);
	mat4x4 normal;
	mat4x4_transpose(normal, modelInverted);
	const float *normalPtr = &normal[0][0];
	glUniformMatrix4fv(glGetUniformLocation(currentProg, "normal"),
		1, GL_FALSE, normalPtr);

	glDrawArrays(GL_TRIANGLES, 0, ro.vertCount);
}

void drawSSAO(mat4x4 view, mat4x4 proj)
{
	const float *projPtr = &proj[0][0];
	const float *viewPtr = &view[0][0];

	glUseProgram(ssaoProg);
	currentProg = ssaoProg;
	glUniform1i(glGetUniformLocation(currentProg, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(currentProg, "ssaoNoise"), 1);
	glUniform2f(glGetUniformLocation(currentProg, "screenSize"),
		eg.winWidth, eg.winHeight);
	glUniformMatrix4fv(glGetUniformLocation(currentProg, "proj"),
		1, GL_FALSE, projPtr);
	glUniformMatrix4fv(glGetUniformLocation(currentProg, "view"),
		1, GL_FALSE, viewPtr);
	const float *ssaoKernPtr = &ssaoKern[0][0];
	glUniform3fv(glGetUniformLocation(currentProg, "samples"),
			SSAO_KERN_SIZE, ssaoKernPtr);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);

	drawRenderObject(screen);
}

void blurSSAO(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurBuffer);
	glViewport(0, 0, eg.winWidth, eg.winHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(blurProg);
	currentProg = blurProg;
	glUniform1i(glGetUniformLocation(currentProg, "ssaoTex"), 0);
	glUniform2f(glGetUniformLocation(currentProg, "screenSize"),
		eg.winWidth, eg.winHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoTex);

	drawRenderObject(screen);
}

void drawAmbient(void)
{
	glUseProgram(ambientProg);
	currentProg = ambientProg;
	glUniform1i(glGetUniformLocation(currentProg, "ssaoTex"), 0);
	glUniform1i(glGetUniformLocation(currentProg, "gAlbedo"), 1);
	glUniform2f(glGetUniformLocation(currentProg, "screenSize"),
		eg.winWidth, eg.winHeight);
	glUniform1f(glGetUniformLocation(currentProg, "defaultAo"),
		eg.doSSAO ? 1.0f : -1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);

	drawRenderObject(screen);
}

void drawLights(Light *lights, unsigned int lightCount)
{
	glUseProgram(lightProg);
	currentProg = lightProg;
	glUniform1i(glGetUniformLocation(currentProg, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(currentProg, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(currentProg, "gAlbedo"), 2);
	glUniform2f(glGetUniformLocation(currentProg, "screenSize"),
		eg.winWidth, eg.winHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);

	for (int n = 0; n < lightCount; n++)
	{
		glUniform3fv(glGetUniformLocation(currentProg, "lightPos"), 1,
			lights[n].origin);
		glUniform1f(glGetUniformLocation(currentProg, "constant"),
			lights[n].constant);
		glUniform1f(glGetUniformLocation(currentProg, "linear"),
			lights[n].linear);
		glUniform1f(glGetUniformLocation(currentProg, "quad"),
			lights[n].quad);

		drawRenderObject(screen);
	}
}

bool loadShaderProgram(const char *const vsName, const char *const fsName,
	unsigned int *prog)
{
	unsigned int vs, fs;
	if (!loadShader(vsName, GL_VERTEX_SHADER, &vs) ||
		!loadShader(fsName, GL_FRAGMENT_SHADER, &fs))
	{
		return false;
	}

	*prog = glCreateProgram();
	glAttachShader(*prog, vs);
	glAttachShader(*prog, fs);
	glLinkProgram(*prog);

	glDeleteShader(vs);
	glDeleteShader(fs);

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
