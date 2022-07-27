#version 330 core

in vec4 fs_worldCoords;

uniform vec3 lightPos;
uniform float far;

void main()
{
	float distance = length(fs_worldCoords.xyz - lightPos);
	distance /= far;
	gl_FragDepth = distance;
}
