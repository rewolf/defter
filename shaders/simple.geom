#version 150 core

// Declare the incoming primitive type
layout(triangles) in;

// Declare the 3esulting primitive type
layout(triangles, max_vertices=170)  out;


// Incoming from vertex shader
in vec2 geom_TexCoord[3];

// Outgoing per-vertex information
out vec2 hmap_texCoord;
out vec3 position;

//--------------------------------------------------------
void main(){
	vec3 x;
	vec3 y;
	vec3 z;
	vec3 w;
	vec4 vertex[3];
	vertex[0] = gl_in[0].gl_Position;
	vertex[1] = gl_in[1].gl_Position;
	vertex[2] = gl_in[2].gl_Position;



	// gather components for frustum culling below
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
	position = vertex[0].xyz;
	hmap_texCoord = geom_TexCoord[0];
	EmitVertex();
	gl_Position = vertex[1];
	position = vertex[1].xyz;
	hmap_texCoord = geom_TexCoord[1];
	EmitVertex();
	gl_Position = vertex[2];
	position = vertex[2].xyz;
	hmap_texCoord = geom_TexCoord[2];
	EmitVertex();
	EndPrimitive();
}

