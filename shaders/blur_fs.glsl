#version 330 core

uniform sampler2D ssaoTex;
uniform vec2 screenSize;

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / screenSize.x;
	uv.y = gl_FragCoord.y / screenSize.y;
	vec2 texelSize = 1.0f / vec2(textureSize(ssaoTex, 0));
	
	float result = 0.0f;
	for (int x = -2; x < 2; x++)
	{
		for (int y = -2; y < 2; y++)
		{
			result += texture(ssaoTex, uv + vec2(float(x), float(y))
				* texelSize).r;
		}
	}
	gl_FragDepth = result / 16.0f;
}
