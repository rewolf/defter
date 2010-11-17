/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Uniforms
uniform sampler2D in_heightmap;
uniform float tc_delta;
uniform float height_scale;


// Shader Input
in vec2 frag_TexCoord;


// Shader Output
out vec2 pdmap;

// Constansts
const vec2 factor	= vec2(-8 * 10 / 1.2, -10/1.2);
const vec3 cc 		= vec3(1.0, .0, -1.0);
const vec3 cc2 		= vec3(2.0, .0, -2.0);


//------------------------------------------------------------------------------
void main()
{
	vec2 dhdst;
	vec2 dh1, dh2;
	vec2 dst = vec2(tc_delta);

	// Horizontal
	dh1.s 	 = texture(in_heightmap, frag_TexCoord + dst * cc.xy).r;	// Right
	dh1.s 	-= texture(in_heightmap, frag_TexCoord + dst * cc.zy).r;	// Left
	// Vertical
	dh1.t 	 = texture(in_heightmap, frag_TexCoord + dst * cc.yx).r;	// Bottom
	dh1.t 	-= texture(in_heightmap, frag_TexCoord + dst * cc.yz).r;	// Top

	// Horizontal
	dh2.s	 = texture(in_heightmap, frag_TexCoord + dst * cc2.zy).r;	// Right
	dh2.s	-= texture(in_heightmap, frag_TexCoord + dst * cc2.xy).r;	// Left
	// Vertical
	dh2.t	 = texture(in_heightmap, frag_TexCoord + dst * cc2.yz).r;	// Bottom
	dh2.t	-= texture(in_heightmap, frag_TexCoord + dst * cc2.yx).r;	// Top

	//dhdst = clamp(factor.x * dh1.st + factor.y * dh2.st , -1.0, 1.0);
	//pdmap = dhdst * 0.5 + 0.5;

	vec2 pd = height_scale*(factor.x * dh1.st + factor.y * dh2.st);
	vec3 normal = normalize(vec3(pd.s, 1.0, pd.t));
	pdmap = normal.xz/normal.y * 0.5 + 0.5;
}
