#if defined(VERTEX)
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Object;
uniform float Time;

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;


out vec2 uv;
out vec3 normal;
out vec3 position;

void main(void)
{	

	float pi = 3.141592654;
	float sigma = pi*2/300;

	uv = VertexTexCoord;
	normal = vec3(Object * vec4(VertexNormal, 1.0));
	position = vec3(Object * vec4(VertexPosition, 1.0));
	mat3 rotX = mat3(
					1, 0, 0,
					0, cos(pi*Time*0.1),-sin(pi*Time*0.1),
					0, sin(pi*Time*0.1), cos(pi*Time*0.1) 
					);

	mat3 rotY = mat3(
					cos(pi*Time*0.1), 0, sin(pi*Time*0.1),
					0, 1, 0,
					-sin(pi*Time*0.1), 0, cos(pi*Time*0.1) 
					);

	mat3 rotZ = mat3(
					cos(pi*Time*0.1), -sin(pi*Time*0.1), 0,
					sin(pi*Time*0.1), cos(pi*Time*0.1), 0,
					0, 0, 1 
					);


	if(gl_InstanceID<=300){
		position.x += 1.1*cos(sigma*gl_InstanceID);
		position.y += 1.1*sin(sigma*gl_InstanceID);
		position*=rotX;
	}
	else if(gl_InstanceID>300 && gl_InstanceID<=600){
		position.z += 1.1*cos(sigma*gl_InstanceID);
		position.y += 1.1*sin(sigma*gl_InstanceID);
		position*=rotY;
	}
	else if(gl_InstanceID>600 && gl_InstanceID<=900){
		position.z += 1.1*cos(sigma*gl_InstanceID);
		position.x += 1.1*sin(sigma*gl_InstanceID);
		position*=rotZ;
	}

	position.y += 4;

	position*=rotZ;

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
	Color = vec4(0.7, diffuse.bg*0.5, spec);
	Normal = vec4(0.8,0.5,0.5, spec);
}

#endif
