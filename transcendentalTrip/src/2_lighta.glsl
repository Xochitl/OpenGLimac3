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

uniform vec3 CameraPosition;
uniform vec3  LightPosition;
uniform vec3  LightColor;
uniform float LightIntensity;
uniform mat4 InverseViewProjection;

out vec4  Color;

/*vec3 createPointLight(vec3 diffuse, float spec, vec3 lightColor, vec3 lightPosition) {
	vec4 material = texture(Material, uv).rgba;
	vec3 normal = texture(Normal, uv).rgb;
	float depth = texture(Depth, uv).r;

	vec2 xy = uv * 2.0 -1.0;
	vec4 wPosition = vec4(xy, depth * 2.0 -1.0, 1.0) * InverseViewProjection;
	vec3 position = vec3(wPosition/wPosition.w);
	vec3 n = normalize(normal);
	vec3 l =  lightPosition - position;

	vec3 v = position - CameraPosition;

	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);



	float lightIntensity = LightIntensity * 1.0/pow(n_dot_l*1.5, 2);

	return vec3(lightColor * lightIntensity * (diffuse * n_dot_l + spec * vec3(1.0, 1.0, 1.0) *  pow(n_dot_h, spec * 100.0)));

}*/

vec3 pointLight(in vec3 lcolor, in float intensity, in vec3 lpos, in vec3 n, in vec3 fpos, vec3 diffuse, float spec, vec3 cpos)
{
	vec3 l =  lpos - fpos;
	vec3 v = fpos - cpos;
	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);
	float d = distance(l, fpos);
	float att = clamp(  1.0 /  (d*d) , 0.0, 1000.0);
	vec3 color = lcolor * intensity /* att */* (diffuse * n_dot_l + spec * vec3(1.0, 1.0, 1.0) *  pow(n_dot_h, spec * 100.0));
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

	color = LightColor * LightIntensity * att * (diffuse * n_dot_l + spec * vec3(1.0, 1.0, 1.0) *  pow(n_dot_h, spec * 100.0));
	//color = createPointLight(diffuse, spec, LightColor, LightPosition);
	//color = pointLight(LightColor,  LightIntensity, LightPosition, normal, position, diffuse, spec, CameraPosition);
	}
	Color = vec4(color, 1.0);
	//Color = vec4(depth, 0.0 , 0.0, 1.0);
	//Color = vec4(normal, 1.0);







	//JIJI
	/*vec4  material = texture(Material, uv).rgba;


	vec3  normal = texture(Normal, uv).rgb;
	float depth = texture(Depth, uv).r;
	vec3 skybox = texture(Skybox, uv).rgb;

	vec2  xy = uv * 2.0 -1.0;
	vec4  wPosition =  vec4(xy, depth * 2.0 -1.0, 1.0) * InverseViewProjection;
	vec3  position = vec3(wPosition/wPosition.w);*/



/*vec3 lightPosition = vec3(-0.5, 2.f, -1.f);
vec3 lightColor = vec3(0.8, 1., 0.8);
float lightIntensity = 2.f;*/

/*vec3 diff = material.rgb;
float spec = material.a;

vec3 bumpedNormal = normalize(texture(uNormalMap, vec2(vUV.x,vUV.y) ).rgb*2. - 1.);
mat3 TBN = mat3(vTangent, vBitangent, vNormal);

vec3 n = normalize(TBN*bumpedNormal);
vec3 l = lightPosition - vPosition;
vec3 v = vPosition - uCameraPosition;
vec3 h = normalize(l-v);

float n_dot_l = clamp(dot(n, l), 0, 1.);
float n_dot_h = clamp(dot(n, h), 0, 1.);

float d = length(l);
float att = 1./(d*d);

vec3 color = lightColor * lightIntensity * att * (diff*n_dot_l + spec*vec3(1., 1., 1.)*pow(n_dot_h, spec*100.));

fragColor = vec4(color, 1.);*/


}

#endif

/*#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vUV;
in vec3 vTangent;
in vec3 vBitangent;

uniform vec3 uCameraPosition;
uniform sampler2D uDiffuse;
uniform sampler2D uSpec;
uniform sampler2D uNormalMap;

out vec4 fragColor;

void main() {
vec3 lightPosition = vec3(-0.5, 2.f, -1.f);
vec3 lightColor = vec3(0.8, 1., 0.8);
float lightIntensity = 2.f;

vec3 diff = texture(uDiffuse, vUV).rgb;
float spec = texture(uSpec, vUV).r;
vec3 bumpedNormal = normalize(texture(uNormalMap, vec2(vUV.x,vUV.y) ).rgb*2. - 1.);
mat3 TBN = mat3(vTangent, vBitangent, vNormal);

vec3 n = normalize(TBN*bumpedNormal);
vec3 l = lightPosition - vPosition;
vec3 v = vPosition - uCameraPosition;
vec3 h = normalize(l-v);

float n_dot_l = clamp(dot(n, l), 0, 1.);
float n_dot_h = clamp(dot(n, h), 0, 1.);

float d = length(l);
float att = 1./(d*d);

vec3 color = lightColor * lightIntensity * att * (diff*n_dot_l + spec*vec3(1., 1., 1.)*pow(n_dot_h, spec*100.));

fragColor = vec4(color, 1.);
}*/