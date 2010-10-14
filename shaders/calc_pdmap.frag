
#version 150 core

uniform sampler2D in_heightmap;
uniform float tc_delta;

in vec2 in_texCoord;

out vec2 pdmap;

const vec2 factor	= vec2(-8 * 10 / 1.2, -10/1.2);
const vec3 cc 		= vec3(1.0, .0, -1.0);
const vec3 cc2 		= vec3(2.0, .0, -2.0);

void main(){
	vec2 dhdst;
	vec2 dh1, dh2;
	vec2 dst = vec2(tc_delta);

	// horiz
	dh1.s 	=  texture(in_heightmap, in_texCoord + dst * cc.xy).r;	//right
	dh1.s 	-= texture(in_heightmap, in_texCoord + dst * cc.zy).r;	//left
	// vert
	dh1.t 	=  texture(in_heightmap, in_texCoord + dst * cc.yx).r;	//bot
	dh1.t 	-= texture(in_heightmap, in_texCoord + dst * cc.yz).r;	//top

	// horiz
	dh2.s	=  texture(in_heightmap, in_texCoord + dst * cc2.zy).r;
	dh2.s	-= texture(in_heightmap, in_texCoord + dst * cc2.xy).r;
	// vert
	dh2.t	=  texture(in_heightmap, in_texCoord + dst * cc.yz).r;
	dh2.t	-= texture(in_heightmap, in_texCoord + dst * cc.yx).r;

	dhdst = clamp( factor.x * dh1.st + factor.y * dh2.st , -1.0, 1.0);
	pdmap = dhdst * .5 + .5;
}
