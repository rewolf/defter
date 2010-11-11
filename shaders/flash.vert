/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Shader Input
in vec2 vert_Position;




//------------------------------------------------------------------------------
void main(){
	gl_Position 	= vec4(vert_Position, 1.0, 1.0);
}
