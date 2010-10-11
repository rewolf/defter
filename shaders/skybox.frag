#version 150 core

uniform sampler2D sky;

in vec2 frag_TexCoord;
in vec3 frag_pos;

out vec4 frag_Color;

const vec4 fog_col		= vec4(0.6, 0.6, 0.6, 1.0);
const float log2		= 1.442695;
const float fog_density	= 0.01;

void main(){
	float fogY, fogFactor;

	// Fog controls
	fogY		= frag_pos.y - 10.0;
	fogFactor	= exp2(-fog_density * fog_density * fogY * fogY * log2);
	fogFactor	= clamp(fogFactor, 0.0, 1.0);

	// Mix to get the final color
	frag_Color = mix(texture(sky, frag_TexCoord), fog_col, fogFactor);
}
