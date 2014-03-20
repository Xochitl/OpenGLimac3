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
	if(Time>=19 && Time<24.8)
		gamma = Time-18;
	else if(Time>24.8 && Time<25.2)
		gamma = -5;//(Time-24.8);
	else if(Time>25.2 && gamma<1.0)
		gamma += (Time-25.2);
	//else
		//gamma = 1;

    OutColor = vec4(pow(colorCorrected.r, 1.0f/gamma),pow(colorCorrected.g, 1.0f/gamma), pow(colorCorrected.b, 1.0f/gamma), 1.0);
}

#endif
