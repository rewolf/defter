
#version 150 core

uniform sampler2D in_heightmap;
uniform float tc_delta;

in vec2 in_texCoord;

out vec3 normalmap;
out vec3 tangentmap;

void main(){
	float dist;
	float left, right, top, bottom;
	float ll, rr, tt, bb;
	float dhdu, dhdv;
	vec3 binormal, tangent;
	
	left 	= texture(in_heightmap, vec2(in_texCoord.s - tc_delta, in_texCoord.t)).r;
	right 	= texture(in_heightmap, vec2(in_texCoord.s + tc_delta, in_texCoord.t)).r;
	top 	= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t - tc_delta)).r;
	bottom 	= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t + tc_delta)).r;

	ll 		= texture(in_heightmap, vec2(in_texCoord.s - 2 * tc_delta, in_texCoord.t)).r;
	rr	 	= texture(in_heightmap, vec2(in_texCoord.s + 2 * tc_delta, in_texCoord.t)).r;
	tt		= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t - 2 * tc_delta)).r;
	bb	 	= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t + 2 * tc_delta)).r;

	tangent.x = .0;
	tangent.y = bottom-top;
	tangent.z = .06;
	binormal.x= .06;
	binormal.y= right - left;
	binormal.z= .0;

	dhdv = - ( 8 * (bottom - top)  - (bb - tt)) / (12 * 0.1) * 20;
	dhdu = - ( 8 * (right  - left) - (rr - ll)) / (12 * 0.1) * 20;

	normalmap = (normalize(vec3(dhdu, dhdv, 1.0)) + 1.0) * .5;

	//normalmap = (normalize(cross(tangent, binormal).rbg) + 1.0) * .5;
	tangentmap = (normalize(tangent).rbg + 1.0) * .5;

}
