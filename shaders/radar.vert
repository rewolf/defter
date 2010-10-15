/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Shader Input
in vec2 vert_Position;
in vec2 vert_TexCoord;


// Shader Output
out vec2 frag_TexCoord;


//------------------------------------------------------------------------------
void main()
{
	gl_Position = vec4(vert_Position, 0.0f, 1.0f);
	frag_TexCoord = vert_TexCoord;
}
