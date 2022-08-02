#version 330 core

out vec4 fragColor;

uniform sampler2D ssaoTex;
uniform sampler2D gAlbedo;
uniform vec2 screenSize;
uniform float defaultAo;

const float ambient = 0.4f;

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / screenSize.x;
	uv.y = gl_FragCoord.y / screenSize.y;
	vec3 albedo = texture(gAlbedo, uv).xyz;

	float ao = abs(min(texture(ssaoTex, uv).r, defaultAo));
	vec3 result = ao * ambient * albedo;
	fragColor = vec4(result, 1.0f);
}
