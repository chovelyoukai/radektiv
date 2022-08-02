#version 330 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec2 lmuv;
layout(location = 2) in vec2 texuv;
layout(location = 3) in vec3 vnorm;
layout(location = 4) in uvec4 vclr;

out vec4 fs_normal;
out vec4 fs_worldCoords;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 normal;

void main()
{
	vec4 newPos = vec4(vpos, 1.0);
	fs_worldCoords = model * newPos;
	gl_Position = proj * view * model * newPos;
	fs_normal = vec4(mat3(normal) * normalize(vnorm), 0.0f);
}
