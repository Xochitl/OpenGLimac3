#if defined(VERTEX)

in vec2 VertexPosition;

out vec2 uv;

void main(void)
{   
	uv = VertexPosition * 0.5 + 0.5;

    gl_Position = vec4(VertexPosition.xy, 0.0, 1.0);
}

#endif

#if defined(FRAGMENT)

uniform sampler2D Texture1;
uniform float Time;

in vec2 uv;

out vec4  OutColor;

void main(void)
{
	float gamma = 1;
	vec3 colorCorrected = texture(Texture1, uv).rgb;
	if(Time>=17 && Time<=22.8)
		gamma = Time-16;
	else if(Time>22.8 && Time<24)
		gamma = 2-(Time-22.8);
	else gamma = 1;
    OutColor = vec4(pow(colorCorrected.r, 1.0f/gamma),pow(colorCorrected.g, 1.0f/gamma), pow(colorCorrected.b, 1.0f/gamma), 1.0);
}

#endif
