/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Uniforms
uniform sampler2D sky;


// Shader Input
in vec3 frag_Position;
in vec2 frag_TexCoord;


// Shader Output
out vec4 frag_Color;


// Constansts
const vec2 fog_col		= vec2(0.6, 1.0);
const float log2_fog_den= -0.01201122392;


//------------------------------------------------------------------------------
void main()
{
	// Variables
	float fogY, fogFactor;

	// Fog controls
	fogY		= min(0, log2_fog_den * frag_Position.y - log2_fog_den * 10.0);
	fogFactor	= exp2(-fogY * fogY);

	// Mix to get the final color
	frag_Color = mix(texture(sky, frag_TexCoord), fog_col.xxxy, fogFactor);
}
