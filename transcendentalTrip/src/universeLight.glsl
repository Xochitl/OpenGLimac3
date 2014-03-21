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

uniform sampler2D Material;
uniform sampler2D Normal;
uniform sampler2D Depth;
uniform sampler2D Skybox;

uniform int NumLight;
uniform float Time;
uniform float LightIntensity;

uniform vec3 CameraPosition;
uniform vec3  LightPosition;
uniform vec3  LightColor;

uniform mat4 InverseViewProjection;

out vec4  Color;

vec3 pointLight(in vec3 lcolor, in float intensity, in vec3 lpos, in vec3 n, in vec3 fpos, vec3 diffuse, float spec, vec3 cpos)
{
	vec3 l =  lpos - fpos;
	vec3 v = fpos - cpos;
	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);
	float d = distance(l, fpos);
	float att = clamp(  1.0 /  (d*d) , 0.0, 50.0);
	vec3 color = lcolor * intensity * att * (diffuse * n_dot_l + spec * vec3(1.0, 1.0, 1.0) *  pow(n_dot_h, spec * 100.0));
	return color;
}


void main(void)
{
	vec4  material = texture(Material, uv).rgba;
	vec3  normal = texture(Normal, uv).rgb;
	float depth = texture(Depth, uv).r;
	vec3 skybox = texture(Skybox, uv).rgb;

	vec2  xy = uv * 2.0 -1.0;
	vec4  wPosition =  vec4(xy, depth * 2.0 -1.0, 1.0) * InverseViewProjection;
	vec3  position = vec3(wPosition/wPosition.w);

	vec3 diffuse = material.rgb;
	float spec = material.a;

	vec3 color = vec3(1.0);
	if(diffuse == vec3(0.) && NumLight==0){
		color = skybox;
	}
	else{
	vec3 n = normalize(normal);
	vec3 l =  LightPosition - position;

	vec3 v = position - CameraPosition;
	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);

	float d = distance(LightPosition, position);
	float att = clamp(  1.0 / ( 1.0 + 1.0 * (d*d)), 0.0, 1.0);


	if(NumLight==0)
		color = LightColor * LightIntensity  * (diffuse * n_dot_l + spec * vec3(1.0, 1.0, 1.0) *  pow(n_dot_h, spec * 100.0));
	else
		color = pointLight(LightColor,  LightIntensity, LightPosition, normal, position, diffuse, spec, CameraPosition);

	}

	Color = vec4(color, 1.0);
}

#endif
