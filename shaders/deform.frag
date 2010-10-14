
#version 150 core

uniform sampler2D in_heightmap;
uniform sampler2D stamp;
uniform vec2 thingy;
uniform float height_scale;
uniform float falloff;

in vec2 in_texCoord;
in vec2 stamp_texCoord;

out float new_height;

void main(){
	vec2 dist;
	float in_height, stamp_height;
	float left, right, top, bottom;
	vec3 binormal, tangent;
	
	in_height = texture(in_heightmap, in_texCoord).r;

	dist = fract(thingy) - in_texCoord;

	in_height += exp(dot(dist, dist)*-5000*falloff)*.3 * height_scale;

	new_height = in_height;
}
