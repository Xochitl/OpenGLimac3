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
	float teta = pi/2000;
	uv = VertexTexCoord;
	normal = vec3(Object * vec4(VertexNormal, 1.0));; 
	position = vec3(Object * vec4(VertexPosition, 1.0)); 

		mat3 rotX = mat3(
					1, 0, 0,
					0, cos(gl_InstanceID+pi*Time*0.05),-sin(gl_InstanceID+pi*Time*0.05),
					0, sin(gl_InstanceID+pi*Time*0.05), cos(gl_InstanceID+pi*Time*0.05) 
					);

	mat3 rotY = mat3(
					cos(pi/2*Time), 0, sin(pi/2*Time),
					0, 1, 0,
					-sin(pi/2*Time), 0, cos(pi/2*Time) 
					);

	mat3 rotZ = mat3(
					cos(pi*Time*0.01), -sin(pi*Time*0.01), 0,
					sin(pi*Time*0.01), cos(pi*Time*0.01), 0,
					0, 0, 1 
					);

	//Faire continuer les 32 avec 33 et ...avec TIme pour le déploiement
	position.x += 0.2*pow(2, teta*gl_InstanceID)*cos(-teta*gl_InstanceID*32*0.1);
	position.y += 0.2*pow(2, teta*gl_InstanceID)*sin(-teta*gl_InstanceID*32*0.1);
	position.z += cos(32*0.1)*sin(32*0.1)*gl_InstanceID*0.01;


	position *=rotX;
	

		position.x += 10;
	position.y += 10;


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


		vec3 mean = vec3(0.f, 0.f, 0.f);

	for (int i=-2; i<2; i++){
    	for (int j=-2; j<2; j++) {
            mean += texelFetch(Spec, ivec2(gl_FragCoord) + ivec2(i,j), 0).rgb;
        }
    }
    mean /= 4*4;
 
	vec3 diffuse = texture(Diffuse, uv).rgb;
	float spec = texture(Spec, uv).r;
	Color = vec4(diffuse*10, 1);
	Normal = vec4(normal, spec);
}

#endif
