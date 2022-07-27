#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 lightView[6];
uniform mat4 lightProj;

out vec4 fs_worldCoords;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; i++)
		{
			fs_worldCoords = gl_in[i].gl_Position;
			gl_Position = lightProj * lightView[face] * fs_worldCoords;
			EmitVertex();
		}
		EndPrimitive();
	}
}
