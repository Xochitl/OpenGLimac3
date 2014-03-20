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

	uv = VertexTexCoord;
	normal = vec3(Object * vec4(VertexNormal, 1.0));
	position = vec3(Object * vec4(VertexPosition, 1.0));

	float brakeX = Time;
	float brakeY = Time;

	float pi = 3.141592654;

	mat3 rotZ = mat3(
					cos(pi*Time*0.2), -sin(pi*Time*0.2), 0,
					sin(pi*Time*0.2), cos(pi*Time*0.2), 0,
					0, 0, 1 
					);

	if(Time>50.){
		if(brakeX/(Time*0.05)<0.01)  brakeX = Time*0.01;
		else brakeX/=(Time*0.05);
		if(brakeY/(Time*0.05)<0.001) brakeY = Time*0.001;
		else brakeY/=(Time*0.05);
	}

		mat3 rotX = mat3(
						1, 0, 0,
						0, cos(pi*brakeX),-sin(pi*brakeX),
						0, sin(pi*brakeX), cos(pi*brakeX) 
						);


		mat3 rotY = mat3(
						cos(pi*brakeY), 0, sin(pi*brakeY),
						0, 1, 0,
						-sin(pi*brakeY), 0, cos(pi*brakeY) 
					);


	if(Time<24.9){
		position.x += CameraPosition.x;
		position.y += CameraPosition.y;
		position.z -= Time*200;

		position*=(mat3(InverseViewProjection)*0.00008);
		position*=rotZ;
	}
	else{
		//position*=rotX*rotY;
		position.x += CameraPosition.x;
		position.y += CameraPosition.y;
		position.z += CameraPosition.z;
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
