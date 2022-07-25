#version 330 core

out vec4 fragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform vec2 screenSize;
uniform vec3 lightPos;
uniform float constant;
uniform float linear;
uniform float quad;

const vec3 lightColor = vec3(1.0f, 0.9f, 0.7f);

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / screenSize.x;
	uv.y = gl_FragCoord.y / screenSize.y;
	vec3 position = vec3(texture(gPosition, uv));
	vec3 normal = vec3(texture(gNormal, uv));
	vec3 albedo = vec3(texture(gAlbedo, uv));

	vec3 lightDir = lightPos - position;
	float lightDist = length(lightDir);
	float intensity = 1.0f / (constant + linear * lightDist + quad *
		(lightDist * lightDist));

	lightDir = normalize(lightDir);
	vec3 light = max(dot(lightDir, normal), 0.0f) * lightColor;

	vec3 result = intensity * light * albedo;
	fragColor = vec4(result, 1.0f);
}
