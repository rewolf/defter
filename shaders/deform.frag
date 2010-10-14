
#version 150 core

uniform sampler2D in_heightmap;
uniform sampler2D stamp;
uniform vec2 thingy;
uniform float height_scale;
uniform float falloff;

in vec2 frag_texCoord;
in vec2 stamp_texCoord;

out float new_height;

void main(){
	vec2 dist;
	float in_height, stamp_height;
	
	in_height = texture(in_heightmap, frag_texCoord).r;

	dist = fract(thingy) - frag_texCoord;

	in_height += exp( - dot(dist, dist) * falloff) * height_scale;

	new_height = in_height;
}
