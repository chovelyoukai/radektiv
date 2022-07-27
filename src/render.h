#ifndef RENDER_H
#define RENDER_H

#include <GL/glew.h>
#include <stdbool.h>

#include "linmath.h"

#define SHADER_SIZE 4096
#define SSAO_KERN_SIZE 6

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

typedef struct
{
	vec3 eye;
	vec3 forward;
	vec3 right;
	vec3 up;
	float near;
	float far;
	float fov;
	float tanFov;
	float factorY;
	float factorX;
} Camera;

typedef struct
{
	struct
	{
		unsigned int geom;
		unsigned int light;
		unsigned int ambient;
		unsigned int ssao;
		unsigned int blur;
		unsigned int current;
		unsigned int shadow;
	} Progs;

	struct
	{
		unsigned int g;
		unsigned int ssao;
		unsigned int ssaoBlur;
		unsigned int shadow;
	} FBOs;

	struct
	{
		unsigned int gPosition;
		unsigned int gNormal;
		unsigned int gAlbedo;
		unsigned int ssao;
		unsigned int ssaoNoise;
		unsigned int ssaoBlur;
		unsigned int shadowMap;
	} Tex;

	struct
	{
		RenderObject screen;
		RenderObject light;
	} Objects;

	Camera cam;
	vec3 ssaoKern[SSAO_KERN_SIZE];
	vec3 ssaoNoise[16];
} Renderer;

bool initRenderer(Renderer *r);
void initCamera(Camera *cam, float near, float far, float fov);
void updateCamera(Camera *cam, vec3 eye, vec3 forward, vec3 right, vec3 up);
void setupAoBuffer(Renderer *r);
void setupGBuffer(Renderer *r);
void setupShadowBuffer(Renderer *r);
unsigned int generateTexture(GLint internalFormat, int width, int height,
	GLenum format, GLenum type, GLint min, GLint mag, GLint wrapS, GLint wrapT);
RenderObject makeRenderObject(float *verts, unsigned int vertCount);
void useProg(Renderer *r, unsigned int prog);
void drawScene(Renderer *r, RenderObject *scene, unsigned int objects,
	Light *lights, unsigned int lightCount, mat4x4 view, mat4x4 proj);
void makeModelMatrix(mat4x4 model, vec3 origin, vec3 rotation, float scale);
void drawRenderObject(Renderer *r, RenderObject ro);
void drawSSAO(Renderer *r, mat4x4 view, mat4x4 proj);
void blurSSAO(Renderer *r);
void drawAmbient(Renderer *r);
bool isSphereVisible(Camera cam, vec3 origin, float radius, mat4x4 view,
	mat4x4 proj);
void drawLights(Renderer *r, RenderObject *scene, unsigned int objects,
	Light *lights, unsigned int lightCount, mat4x4 view, mat4x4 proj);
bool loadShaderProgram(const char *const vsName, const char *const gsName,
	const char *const fsName, unsigned int *prog);
bool loadShader(const char *const filename, const GLenum type,
	unsigned int *shader);
bool readFile(const char *const filename, char *buffer, size_t size);

#endif
