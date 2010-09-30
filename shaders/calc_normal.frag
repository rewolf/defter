
#version 150 core

uniform sampler2D in_heightmap;
uniform float tc_delta;

in vec2 in_texCoord;

out vec3 normalmap;
out vec3 tangentmap;

void main(){
	float in_height, dist;
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

	normalmap = (normalize(cross(tangent, binormal).rbg) + 1.0) * .5;
	tangentmap = (normalize(tangent).rbg + 1.0) * .5;

}
