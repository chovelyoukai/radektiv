#version 330 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnorm;

uniform mat4 model;

void main()
{
	vec4 newPos = vec4(vpos, 1.0);
	gl_Position = model * newPos;
}
