#ifndef RENDER_H
#define RENDER_H

#include <GL/glew.h>
#include <stdbool.h>

#include "linmath.h"

#define SHADER_SIZE 4096

typedef struct
{
	float *verts;
	unsigned int vertCount;
	unsigned int vao;
	vec3 origin;
	vec3 rotation;
	float scale;
} RenderObject;

typedef struct
{
	vec3 origin;
	float constant;
	float linear;
	float quad;
} Light;

bool initRenderer(void);
void setupAoBuffer(void);
void setupGBuffer(void);
unsigned int generateTexture(GLint internalFormat, int width, int height,
	GLenum format, GLenum type, GLint min, GLint mag, GLint wrapS, GLint wrapT);
RenderObject makeRenderObject(float *verts, unsigned int vertCount);
void drawScene(RenderObject *scene, unsigned int objects, Light *lights,
	unsigned int lightCount, mat4x4 view, mat4x4 proj);
void drawRenderObject(RenderObject ro);
void drawSSAO(mat4x4 view, mat4x4 proj);
void blurSSAO(void);
void drawAmbient(void);
void drawLights(Light *lights, unsigned int lightCount);
bool loadShaderProgram(const char *const vsName, const char *const fsName,
	unsigned int *prog);
bool loadShader(const char *const filename, const GLenum type,
	unsigned int *shader);
bool readFile(const char *const filename, char *buffer, size_t size);

#endif
