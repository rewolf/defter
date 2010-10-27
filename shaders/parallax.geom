/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core

// Declare the incoming primitive type
layout(triangles) in;

// Declare the 3esulting primitive type
layout(triangle_strip, max_vertices=170)  out;


// Incoming from vertex shader
in vec3 geom_View[3];
in vec2 geom_TexCoord[3];
in vec3 geom_Normal[3];


// Outgoing per-vertex information
out vec3 frag_View;
out vec2 frag_TexCoord;


//------------------------------------------------------------------------------
void main()
{
	vec3 x;
	vec3 y;
	vec3 z;
	vec3 w;
	vec4 vertex[3];
	vertex[0] = gl_in[0].gl_Position;
	vertex[1] = gl_in[1].gl_Position;
	vertex[2] = gl_in[2].gl_Position;

	// Gather components for frustum culling below
	x = abs(vec3(vertex[0].x, vertex[1].x, vertex[2].x));
	y = abs(vec3(vertex[0].y, vertex[1].y, vertex[2].y));
	z = -vec3(vertex[0].z, vertex[1].z, vertex[2].z);
	w = vec3(vertex[0].w, vertex[1].w, vertex[2].w);


	if (!any(lessThan(x, w)))
		return;
	if (!any(lessThan(y, w)))
		return;
	if (!any(lessThan(z, w)))
		return;

	gl_Position = vertex[0];
	frag_View = geom_View[0];
	frag_TexCoord = geom_TexCoord[0];
	EmitVertex();

	gl_Position = vertex[1];
	frag_View = geom_View[1];
	frag_TexCoord = geom_TexCoord[1];
	EmitVertex();

	gl_Position = vertex[2];
	frag_View = geom_View[2];
	frag_TexCoord = geom_TexCoord[2];
	EmitVertex();

	EndPrimitive();
}
