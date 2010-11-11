/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Uniforms
uniform vec4 color;


// Shader Output
out vec4 frag_Color;


//------------------------------------------------------------------------------
void main(){
	frag_Color = color;
}
