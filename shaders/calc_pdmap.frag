
#version 150 core

// Uniforms
uniform sampler2D in_heightmap;
uniform float tc_delta;

// Shader Input
in vec2 frag_TexCoord;

// Shader Output
out vec2 pdmap;

// Constants
const vec2 factor	= vec2(-8 * 10 / 1.2, -10/1.2);
const vec3 cc 		= vec3(1.0, .0, -1.0);
const vec3 cc2 		= vec3(2.0, .0, -2.0);

void main()
{
	vec2 dhdst;
	vec2 dh1, dh2;
	vec2 dst = vec2(tc_delta);

	// Horizontal
	dh1.s 	 = texture(in_heightmap, frag_TexCoord + dst * cc.xy).r;	//Right
	dh1.s 	-= texture(in_heightmap, frag_TexCoord + dst * cc.zy).r;	//Left
	// Vertical
	dh1.t 	 = texture(in_heightmap, frag_TexCoord + dst * cc.yx).r;	//Bottom
	dh1.t 	-= texture(in_heightmap, frag_TexCoord + dst * cc.yz).r;	//Top

	// Horizontal
	dh2.s	 = texture(in_heightmap, frag_TexCoord + dst * cc2.zy).r;
	dh2.s	-= texture(in_heightmap, frag_TexCoord + dst * cc2.xy).r;
	// Vertical
	dh2.t	 = texture(in_heightmap, frag_TexCoord + dst * cc.yz).r;
	dh2.t	-= texture(in_heightmap, frag_TexCoord + dst * cc.yx).r;

	dhdst = clamp(factor.x * dh1.st + factor.y * dh2.st , -1.0, 1.0);
	pdmap = dhdst * 0.5 + 0.5;
}
