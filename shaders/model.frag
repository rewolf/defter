#version 150

uniform vec3	diffuseC;
uniform	vec4	specularC;
uniform mat4 	view;

in	vec3 	frag_Normal;
in	vec2	frag_TexCoord;
in	vec4	frag_Pos;

out vec4	out_Color;

// Constants
const vec3 lightW		= normalize(vec3(1.0, 4.0, -10.0));

void main(){
	vec3 	normal;
	vec3	light;
	vec3 	diffuse;
	vec3	reflec;
	vec3	specular;
	vec3	eye;

	light  	= normalize(mat3(view) * lightW);
	normal	= normalize(mat3(view) * frag_Normal);
	reflec	= -reflect(light, normal);
	eye		= - normalize(frag_Pos.xyz);

	diffuse	= diffuseC * max(0, dot(normal, light));
	specular= specularC.rgb * pow(max(0, dot(reflec, eye)), 32);

	out_Color = vec4(diffuse + specular, 1.0);
}