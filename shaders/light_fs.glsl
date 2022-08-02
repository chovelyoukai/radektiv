#version 330 core

out vec4 fragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform samplerCube shadowMap;

uniform vec2 screenSize;
uniform vec3 lightPos;
uniform float constant;
uniform float linear;
uniform float quad;
uniform float far;

vec3 sampleOffsets[14] = vec3[]
(
	vec3( 1, 1, 1), vec3( 1, 1,-1), vec3( 1,-1, 1), vec3( 1,-1,-1),
	vec3(-1, 1, 1), vec3(-1, 1,-1), vec3(-1,-1, 1), vec3(-1,-1,-1),
	vec3( 1, 0, 0), vec3( 0, 1, 0), vec3( 0, 0, 1),
	vec3(-1, 0, 0), vec3( 0,-1, 0), vec3( 0,-0, 1)
);

float shadowAmount(vec3 position, vec3 normal)
{
	vec3 lightVec = position - lightPos;
	float currentDepth = length(lightVec);
	float closestDepth = far * texture(shadowMap, lightVec).r;

	float radius = (currentDepth - closestDepth) * 1.5f / closestDepth;

	float shadow = 0.0f;
	float bias = max(0.2 * (1.0 - dot(normal, lightVec)), 0.05);
	for (int i = 0; i < 14; i++)
	{
		float depth = far * texture(shadowMap, lightVec + sampleOffsets[i] * radius).r;
		shadow += currentDepth - bias > depth ? 1.0f : 0.0f;
	}
	shadow /= 14.0f;
	return shadow;
}

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

	float shadow = 1.0f - shadowAmount(position, normal);

	vec3 result = shadow * intensity * light * albedo;
	fragColor = vec4(result, 1.0f);
}
