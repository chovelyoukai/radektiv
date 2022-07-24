#version 330 core

uniform sampler2D gPosition;
uniform sampler2D ssaoNoise;

#define SSAO_KERN_SIZE 6
#define RADIUS 32.0f
#define BIAS 0.025f

uniform vec2 screenSize;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 samples[SSAO_KERN_SIZE];

float calculateAO(vec2 uv)
{
	vec3 position = (view * texture(gPosition, uv)).xyz;
	vec2 noiseScale = screenSize / 4.0f;
	vec3 random = texture(ssaoNoise, uv * noiseScale).xyz;

	float occlusion = 0.0f;
	for (int i = 0; i < SSAO_KERN_SIZE; i++)
	{
		vec3 samplePos = samples[i] + random;
		samplePos = position + samplePos * RADIUS;

		vec4 sampleOff = vec4(samplePos, 1.0f);
		sampleOff = proj * sampleOff;
		sampleOff.xyz /= sampleOff.w;
		sampleOff.xyz = sampleOff.xyz * 0.5f + 0.5f;

		float sampleDepth = (view * texture(gPosition, sampleOff.xy)).z;

		float rangeCheck = smoothstep(0.0f, 1.0f,
			RADIUS / abs(position.z - sampleDepth));
		occlusion += step(0.0f, sampleDepth - (samplePos.z + BIAS)) * rangeCheck;
	}
	occlusion = 1.2f - (occlusion / SSAO_KERN_SIZE);
	return occlusion;
}

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / screenSize.x;
	uv.y = gl_FragCoord.y / screenSize.y;

	gl_FragDepth = calculateAO(uv);
}
