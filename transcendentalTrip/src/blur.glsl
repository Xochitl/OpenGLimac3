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

in vec2 uv;

uniform sampler2D Texture1;
uniform float Time;

out vec4  OutColor;

void main ()
{
	vec3 color = texture(Texture1, uv).rgb;
    vec3 mean = texture(Texture1, uv).rgb;
    if(Time>13 && Time<=22.8){

        int kernel = int(floor(Time))-12;
        if(kernel>8) kernel = 8;

    	for (int i=-kernel; i<kernel; i++){
        	for (int j=-kernel; j<kernel; j++) {
                mean += texelFetch(Texture1, ivec2(gl_FragCoord) + ivec2(i,j), 0).rgb;
            }
        }
        mean /= kernel*kernel;
    }
    OutColor = vec4(mix(color, mean, 0.15), 1.0);
}
#endif

