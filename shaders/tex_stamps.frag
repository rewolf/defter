/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Uniforms
uniform sampler2D in_heightmap;
uniform sampler2D in_stampmap;
uniform float intensity;


// Shader Input
in vec2 frag_TexCoord;
in vec2 stamp_TexCoord;


// Shader Output
out float height;


//------------------------------------------------------------------------------
void main()
{
	height	 = texture(in_heightmap, frag_TexCoord).r;
	height	+= (texture(in_stampmap, stamp_TexCoord).r * 0.1 * intensity);
}
