#version 150

uniform vec3		diffuseC;
uniform vec3		ambientC;
uniform	vec4		specularC;
uniform mat4		view;
uniform sampler2D	colormap;

in	vec3 	frag_Normal;
in	vec2	frag_TexCoord;
in	vec4	frag_Pos;

out vec4	out_Color;

// Constants
const vec3 lightW		= normalize(vec3(1.0, 4.0, -10.0));

void main(){
	vec3 	normal;
	vec3	light;
	vec3	reflec;
	vec3	eye;
	vec3 	diffuse;
	vec3	specular;
	vec3	ambient;
	vec3	texC;

	light  	= normalize(mat3(view) * lightW);
	normal	= normalize(frag_Normal);
	reflec	= -reflect(light, normal);
	eye		= - normalize(frag_Pos.xyz);

	texC	= texture(colormap, frag_TexCoord).rgb;
	diffuse	= diffuseC * max(0, dot(normal, light)) * texC;
	specular= specularC.rgb * pow(max(0, dot(reflec, eye)), 32);
	ambient = .2* ambientC;

	out_Color = vec4(ambient + diffuse + specular, 1.0);
}
