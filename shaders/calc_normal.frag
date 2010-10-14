
#version 150 core

uniform sampler2D in_heightmap;
uniform float tc_delta;

in vec2 in_texCoord;

out vec2 pdmap;

const vec2 factor = vec2(-8 * 10 / 1.2, -10/1.2);

void main(){
	vec2 dhd;

	vec2 dst1, dst2, dst;

	// horiz
	dst1.s 	=  texture(in_heightmap, vec2(in_texCoord.s + tc_delta, in_texCoord.t)).r;	//right
	dst1.s 	-= texture(in_heightmap, vec2(in_texCoord.s - tc_delta, in_texCoord.t)).r;	//left
	// vert
	dst1.t 	=  texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t + tc_delta)).r;	//bot
	dst1.t 	-= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t - tc_delta)).r;	//top

	// horiz
	dst2.s	=  texture(in_heightmap, vec2(in_texCoord.s - 2 * tc_delta, in_texCoord.t)).r;
	dst2.s	-= texture(in_heightmap, vec2(in_texCoord.s + 2 * tc_delta, in_texCoord.t)).r;
	// vert
	dst2.t	=  texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t - 2 * tc_delta)).r;
	dst2.t	-= texture(in_heightmap, vec2(in_texCoord.s, in_texCoord.t + 2 * tc_delta)).r;

	dhd = clamp( factor.x * dst1.st + factor.y*dst2.st , -1.0, 1.0);
	pdmap = dhd * .5 + .5;
}
