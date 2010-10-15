/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
/*

#version 150 core

// Uniforms
uniform mat4 mvpMatrix;


// Shader Input
in vec3 vert_Position;
in vec2 vert_TexCoord;


// Shader Output
out vec3 frag_Position;
out vec2 frag_TexCoord;


//------------------------------------------------------------------------------
void main()
{
	gl_Position 	= mvpMatrix * vec4(vert_Position, 1.0f);
	frag_Position 	= vert_Position;
	frag_TexCoord	= vert_TexCoord;
}
