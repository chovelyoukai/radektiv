#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec4 fs_normal;
in vec4 fs_worldCoords;
in vec2 fs_uv;
in vec4 fs_clr;

out vec4 fragColor;

uniform sampler2D albedo;

void main()
{
	gPosition = fs_worldCoords;
	gNormal = normalize(fs_normal);
	gAlbedo = texture(albedo, fs_uv);
}
