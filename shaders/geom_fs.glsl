#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec4 fs_normal;
in vec4 fs_worldCoords;

out vec4 fragColor;

void main()
{
	gPosition = fs_worldCoords;
	gNormal = normalize(fs_normal);
	gAlbedo = vec4(0.5, 0.5, 0.5, 1.0f);
}
