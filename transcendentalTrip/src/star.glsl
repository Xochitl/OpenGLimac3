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
	float bigbang = 1/500;


float lambda = pi/20000;
float phi = 2*pi/20000;

	uv = VertexTexCoord;
	normal = vec3(Object * vec4(VertexNormal, 1.0));
	position = vec3(Object * vec4(VertexPosition, 1.0));
	mat3 rotX = mat3(
					1, 0, 0,
					0, cos(gl_InstanceID+pi*Time*0.005),-sin(gl_InstanceID+pi*Time*0.005),
					0, sin(gl_InstanceID+pi*Time*0.005), cos(gl_InstanceID+pi*Time*0.005) 
					);

	mat3 rotY = mat3(
					cos(pi*Time*0.02), 0, sin(pi*Time*0.02),
					0, 1, 0,
					-sin(pi*Time*0.02), 0, cos(pi*Time*0.02) 
					);

	mat3 rotZ = mat3(
					cos(pi*Time*0.1), -sin(pi*Time*0.1), 0,
					sin(pi*Time*0.1), cos(pi*Time*0.1), 0,
					0, 0, 1 
					);


	//Spirate d'entrée
	if(Time<25 && gl_InstanceID<2000){
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

	//Champ d'étoiles
	/*if(gl_InstanceID>2000){
		position.x += 1*cos(-0.5*pi+teta*gl_InstanceID*(gl_InstanceID-500))*cos(-pi+phi*gl_InstanceID);
		position.y += 1*cos(-0.5*pi+teta*gl_InstanceID*(gl_InstanceID-100))*sin(-pi+phi*gl_InstanceID);
		position.z += 1*sin(-0.5*pi+teta*gl_InstanceID);


		position *= rotX*rotY;
	}*/


	/*if(gl_InstanceID>2000){
		position.x += 5*cos(-0.5*pi+teta*gl_InstanceID*Time)*cos(-pi+phi*gl_InstanceID);
		position.y += 5*cos(-0.5*pi+teta*gl_InstanceID*Time)*sin(-pi+phi*gl_InstanceID);
		position.z += 5*sin(-0.5*pi+teta*gl_InstanceID*Time);
	}*/


	if(gl_InstanceID>1999){
		position.x += 2*cos(pi+lambda*(gl_InstanceID)*(Time-25)*0.4)*cos(pi+phi*(gl_InstanceID));
		position.y += 2*cos(pi+lambda*(gl_InstanceID)*(Time-25)*0.4)*sin(pi+phi*(gl_InstanceID));
		position.z += 2*sin(pi+lambda*(gl_InstanceID)*(Time-25)*0.4);

		position *= -rotZ;
	}


	//Expansion univers, big bang
	/*if(Time>2.0f && (Time-2.f)/2.f<7) position *= (Time-2.f)/2.f;
	else if(Time<2.0f) position *= 0.f;
	else position *= 7.f;*/

	//Onde de choc
	/*if(gl_InstanceID>1){
		if(Time>2.0f) position *= (Time-2.f)/2.f*2;
		else position *= 0;
	}*/

	//position *= rotY*rotZ;


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
