/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Uniforms
uniform sampler2D HUDelem;

// Shader Input
in vec2 frag_TexCoord;

// Shader Output
out vec4 frag_Color;


//------------------------------------------------------------------------------
void main(){
	frag_Color = texture(HUDelem, frag_TexCoord).rgbb * vec4(1.0,1.0,1.0,.0) + vec4(.0, .0, .0, 1.0);
}
