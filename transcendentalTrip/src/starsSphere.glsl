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
	float lambda = pi/4000;
	float phi = 2*pi/4000;
	float teta = pi/6000;


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

	if(gl_InstanceID<4000){
		position.x += 2*cos(pi+lambda*(gl_InstanceID)*(Time-25)*0.4)*cos(pi+phi*(gl_InstanceID));
		position.y += 2*cos(pi+lambda*(gl_InstanceID)*(Time-25)*0.4)*sin(pi+phi*(gl_InstanceID));
		position.z += 2*sin(pi+lambda*(gl_InstanceID)*(Time-25)*0.4);

		position *= -rotZ;
	}
	else{
		
		position.x += 10*cos(-0.5*pi+teta*gl_InstanceID*(gl_InstanceID-500))*cos(-pi+phi*gl_InstanceID);
		position.y += 10*cos(-0.5*pi+teta*gl_InstanceID*(gl_InstanceID-100))*sin(-pi+phi*gl_InstanceID);
		position.z += 10*sin(-0.5*pi+teta*gl_InstanceID);
		position *= rotX*rotY;
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
