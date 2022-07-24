#version 330 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vec4 newPos = vec4(vpos, 1.0f);
	gl_Position = newPos;
}
