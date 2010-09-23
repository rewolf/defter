
#version 150 core

uniform sampler2D in_heightmap;
uniform sampler2D stamp;
uniform vec2 thingy;
uniform float tc_delta;
uniform float scale;

in vec2 in_texCoord;
in vec2 stamp_texCoord;

out float new_height;
out vec3 normalmap;

void main(){
	float in_height, stamp_height, dist;
	float left, right, top, bottom;
	vec3 binormal, tangent;
	
	in_height = texture(in_heightmap, in_texCoord).r;

	left 	= texture(in_heightmap, vec2(in_texCoord.s - tc_delta, in_texCoord.t)).r;
	right 	= texture(in_heightmap, vec2(in_texCoord.s + tc_delta, in_texCoord.t)).r;
	top 	= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t-tc_delta)).r;
	bottom 	= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t+tc_delta)).r;

	tangent.x = .0;
	tangent.y = bottom-top;
	tangent.z = .06;
	binormal.x= .06;
	binormal.y= right - left;
	binormal.z= .0;

	normalmap = (normalize(cross(tangent, binormal).rbg)+1.0) * .5;

	dist = distance(thingy, in_texCoord);

	in_height += exp(-dist*dist*10000)*.1 * scale;

	new_height = in_height;
}
