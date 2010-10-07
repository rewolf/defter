#version 150 core

uniform sampler2D sky;

in vec2 frag_TexCoord;
in vec3 frag_pos;

out vec4 frag_Color;

const vec4 const_fog_col= vec4(.3, .3, .3, 1.0);

void main(){
	float fogZ, fogFactor;

	frag_Color = texture(sky, frag_TexCoord);
	fogFactor  = clamp(-.06 * frag_pos.y + 1, .0, 1.0);
	frag_Color = fogFactor * const_fog_col + (1-fogFactor)*frag_Color;
}
