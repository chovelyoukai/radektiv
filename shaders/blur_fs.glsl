#version 330 core

uniform sampler2D ssaoTex;
uniform vec2 screenSize;

#define BLUR_SIZE 2

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / screenSize.x;
	uv.y = gl_FragCoord.y / screenSize.y;
	vec2 texelSize = 1.0f / vec2(textureSize(ssaoTex, 0));
	
	float result = 0.0f;
	for (float x = -BLUR_SIZE; x < BLUR_SIZE; x++)
	{
		for (float y = -BLUR_SIZE; y < BLUR_SIZE; y++)
		{
			result += texture(ssaoTex, uv + vec2(x, y)
				* texelSize).r;
		}
	}
	gl_FragDepth = result / (4 * BLUR_SIZE * BLUR_SIZE);
}
