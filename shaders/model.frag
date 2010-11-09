#version 150


in	vec3 	frag_Normal;
in	vec2	frag_TexCoord;

out vec4	out_Color;

// Constants
const vec4 light		= normalize(vec4(1.0, 4.0, -10.0, 0.0));

void main(){
	vec3 	normal;
	float	NdotL;

	normal = normalize(frag_Normal);
	NdotL  = max(0, dot(normal, light.xyz));

	out_Color = vec4(1.0, .0, .0, 1.0) * NdotL;
}
