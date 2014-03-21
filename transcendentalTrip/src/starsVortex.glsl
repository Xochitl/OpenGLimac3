#if defined(VERTEX)
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Object;
uniform float Time;

uniform vec3  CameraPosition;
uniform mat4  InverseViewProjection;

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;


out vec2 uv;
out vec3 normal;
out vec3 position;

void main(void)
{	

	float pi = 3.141592654;
	float teta = pi/2000;
	float sigma = pi*2/200;

	uv = VertexTexCoord;
	normal = vec3(Object * vec4(VertexNormal, 1.0));
	position = vec3(Object * vec4(VertexPosition, 1.0));

	//Spirate d'entrée
	if(Time<22.8 && gl_InstanceID<2000){
		position.x += 0.15*pow(3, teta*gl_InstanceID)*cos(teta*gl_InstanceID*Time*10);
		position.y += 0.15*pow(3, teta*gl_InstanceID)*sin(teta*gl_InstanceID*Time*10);
		position.z -= gl_InstanceID*0.1 - Time*5;
	}
	else if(Time<35 && gl_InstanceID<2000){
		position.x += 0.15*pow(2, teta*gl_InstanceID)*cos(teta*gl_InstanceID*Time*5);
		position.y += 0.15*pow(2, teta*gl_InstanceID)*sin(teta*gl_InstanceID*Time*5);
		position.z += gl_InstanceID*0.01;
		position.z += Time*1.2-30;
	}
	else if(gl_InstanceID<2000){
		position.x += 0.15*pow(2, teta*gl_InstanceID)*cos(teta*gl_InstanceID*Time*5);
		position.y += 0.15*pow(2, teta*gl_InstanceID)*sin(teta*gl_InstanceID*Time*5);
		position.z += gl_InstanceID*0.01;
		position.z += 35*1.2-30;
	}


	gl_Position = Projection * View * vec4(position, 1.0);

}

#endif

#if defined(FRAGMENT)
uniform vec3 CameraPosition;
uniform float Time;

in vec2 uv;
in vec3 position;
in vec3 normal;

uniform sampler2D Diffuse;
uniform sampler2D Spec;

out vec4  Color;
out vec4  Normal;

void main(void)
{
	vec3 diffuse = texture(Diffuse, uv).rgb;
	float spec = texture(Spec, uv).r;
	Color = vec4(diffuse, spec);
	Normal = vec4(normal, spec);
}

#endif
