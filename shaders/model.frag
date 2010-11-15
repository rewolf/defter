#version 150

uniform vec3		diffuseC;
uniform vec3		ambientC;
uniform	vec4		specularC;
uniform mat4		view;
uniform sampler2D	colormap;
uniform bool		useTex;

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
	
	if (useTex){
		texC	= texture(colormap, frag_TexCoord).rgb;
		diffuse	= vec3(1.0, 1.0, 1.0);
		ambient	= vec3(.1, .1, .1);
	}else{
		texC	= vec3(1.0, 1.0, 1.0);
		diffuse	= diffuseC;
		ambient	= diffuse*.1;
	}
	diffuse	= diffuse * max(0, dot(normal, light));
	specular= specularC.rgb * pow(max(0, dot(reflec, eye)), 32);

	out_Color = vec4((ambient + diffuse) * texC + specular, 1.0);
}
