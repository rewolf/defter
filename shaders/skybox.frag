#version 150 core

uniform sampler2D sky;

in vec2 frag_TexCoord;
in vec3 frag_pos;

out vec4 frag_Color;

const vec2 fog_col		= vec2(0.6, 1.0);
const float log2_fog_density	= -0.01201122392;;

void main(){
	float fogY, fogFactor;

	// Fog controls
	fogY		= log2_fog_density * frag_pos.y - log2_fog_density * 10.0;
	fogFactor	= exp2(-fogY * fogY);

	// Mix to get the final color
	frag_Color = mix(texture(sky, frag_TexCoord), fog_col.xxxy, fogFactor);
}
