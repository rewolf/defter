/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core
#pragma optionNV unroll all
#pragma optionNV inline all


#define emitVert(idx)\
	frag_TexCoord   = out_tex[idx];     \
	/*frag_View		= -out_verts[idx].xyz * (1.0/out_verts[idx].w); */\
	gl_Position     = projection * out_verts[idx];	\
	EmitVertex();

#define prepareVert(idx)			\
/*	out_tex[idx] = barycentric[idx].x * geom_TexCoord[0]	\
			   + barycentric[idx].y * geom_TexCoord[1]		\
			   + barycentric[idx].z * geom_TexCoord[2];		\
	temp       = barycentric[idx].x * gl_in[0].gl_Position	\
			   + barycentric[idx].y * gl_in[1].gl_Position	\
			   + barycentric[idx].z * gl_in[2].gl_Position;	*/\
	out_tex[idx] = stuff2 * barycentric[idx].xyz;\
	temp		= stuff * barycentric[idx].xyz;\
	/*temp.y = texture(heightmap, out_tex[idx]).r * HEIGHT + camera_height;*/\
	temp.w = 1.0;											\
	out_verts[idx] = view * temp;

// Declare the incoming primitive type
layout(triangles) in;

// Declare the 3esulting primitive type
layout(triangle_strip, max_vertices=170)  out;

// Uniforms
uniform sampler2D heightmap;
uniform mat4 projection;
uniform mat4 view;
uniform float camera_height;

// Incoming from vertex shader
in vec2 geom_TexCoord[3];
in vec4 geom_ProjPos[3];
in int mustTess[3];


// Outgoing per-vertex information
out vec3 frag_View;
out vec2 frag_TexCoord;

void refine_with_pattern(in int index);

// Globals
const vec2 const_list = vec2(1.0, .0);
const float HEIGHT = 40.0;

mat3x4 stuff;
mat3x2 stuff2;

//------------------------------------------------------------------------------
void main()
{
	vec3 x;
	vec3 y;
	vec3 z;
	vec3 w;
	vec4 vertex[3];
	vertex[0] = geom_ProjPos[0];
	vertex[1] = geom_ProjPos[1];
	vertex[2] = geom_ProjPos[2];

	stuff = mat3x4(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
	stuff2= mat3x2(geom_TexCoord[0], geom_TexCoord[1], geom_TexCoord[2]);

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

	int index = (mustTess[0]<<0) | (mustTess[1] << 1) | (mustTess[2] << 2);
	refine_with_pattern(index);
}

//--------------------------------------------------------
void refine_with_pattern(in int index){
	vec4 out_verts   [10];
	vec3 barycentric [10];
	vec2 out_tex[10];
	vec4 temp;

	barycentric[0] = vec3( 0.0		, 1.0	, 0.0	);
	barycentric[1] = vec3( 0.0		, 0.667	, 0.333	);
	barycentric[2] = vec3( 0.0		, 0.333	, 0.667	);
	barycentric[3] = vec3( 0.0		, 0.0	, 1.0	);
	barycentric[4] = vec3( 0.333	, 0.667	, 0.0	);
	barycentric[5] = vec3( 0.3333	, 0.3333, 0.3334);
	barycentric[6] = vec3( 0.333	, 0.0	, 0.667	);
	barycentric[7] = vec3( 0.667	, 0.333	, 0.0	);
	barycentric[8] = vec3( 0.667	, 0.0	, 0.333	);
	barycentric[9] = vec3( 1.0		, 0.0	, 0.0	);

	// Interpolate positions and tex coords and then apply viewproj transform
	prepareVert(9);
	prepareVert(0);
	prepareVert(3);
	// Form the triangles
	switch(index){
		case 0:
		case 1:
		case 2:
		case 4:
			emitVert(9);
			emitVert(0);
			emitVert(3);
			EndPrimitive();
			break;
		case 3:
			prepareVert(7);
			prepareVert(4);
			emitVert(9);
			emitVert(7);
			emitVert(3);
			emitVert(4);
			emitVert(0);
			EndPrimitive();
			break;
		case 5:
			prepareVert(6);
			prepareVert(8);
			emitVert(3);
			emitVert(6);
			emitVert(0);
			emitVert(8);
			emitVert(9);
			EndPrimitive();
			break;
		case 6:
			prepareVert(1);
			prepareVert(2);
			emitVert(0);
			emitVert(1);
			emitVert(9);
			emitVert(2);
			emitVert(3);
			EndPrimitive();
			break;
		case 7:
			prepareVert(1);
			prepareVert(2);
			prepareVert(4);
			prepareVert(5);
			prepareVert(6);
			prepareVert(7);
			prepareVert(8);
			emitVert(3);
			emitVert(6);
			emitVert(2);
			emitVert(5);
			emitVert(1);
			emitVert(4);
			emitVert(0);
			EndPrimitive();
			emitVert(6);
			emitVert(8);
			emitVert(5);
			emitVert(7);
			emitVert(4);
			EndPrimitive();
			emitVert(8);
			emitVert(9);
			emitVert(7);
			EndPrimitive();
	};
}

//--------------------------------------------------------
void refine_with_pattern2(in int index){
	vec4 out_verts   [10];
	vec4 barycentric [10];
	vec2 out_tex[10];
	vec4 temp;

	barycentric[0] = vec4( 0.0	, 1.0	, 0.0	, 1.0	);
	barycentric[1] = vec4( 0.0	, 0.75	, 0.25	, 1.0	);
	barycentric[2] = vec4( 0.0	, 0.5	, 0.5	, 1.0	);
	barycentric[3] = vec4( 0.0	, 0.25	, 0.75	, 1.0	);
	barycentric[4] = vec4( 0.0	, 0.0	, 1.0	, 1.0	);

	barycentric[5] = vec4( 0.25	, 0.75	, 0.0	, 1.0	);
	barycentric[6] = vec4( 0.25	, 0.5	, 0.25	, 1.0	);
	barycentric[7] = vec4( 0.25	, 0.25	, 0.5	, 1.0	);
	barycentric[8] = vec4( 0.25	, 0.0	, 0.75	, 1.0	);

	barycentric[8] = vec4( 0.5	, 0.333	, 0.0	, 1.0	);
	barycentric[8] = vec4( 0.5	, 0.0	, 0.333	, 1.0	);
	barycentric[8] = vec4( 0.5	, 0.0	, 0.333	, 1.0	);
	barycentric[8] = vec4( 0.75	, 0.0	, 0.333	, 1.0	);
	barycentric[8] = vec4( 0.75	, 0.0	, 0.333	, 1.0	);
	barycentric[9] = vec4( 1.0	, 0.0	, 0.0	, 1.0	);

	// Interpolate positions and tex coords and then apply viewproj transform
	prepareVert(9);
	prepareVert(0);
	prepareVert(3);
	// Form the triangles
	switch(index){
		case 0:
		case 1:
		case 2:
		case 4:
			emitVert(9);
			emitVert(0);
			emitVert(3);
			EndPrimitive();
			break;
		case 3:
			prepareVert(7);
			prepareVert(4);
			emitVert(9);
			emitVert(7);
			emitVert(3);
			emitVert(4);
			emitVert(0);
			EndPrimitive();
			break;
		case 5:
			prepareVert(6);
			prepareVert(8);
			emitVert(3);
			emitVert(6);
			emitVert(0);
			emitVert(8);
			emitVert(9);
			EndPrimitive();
			break;
		case 6:
			prepareVert(1);
			prepareVert(2);
			emitVert(0);
			emitVert(1);
			emitVert(9);
			emitVert(2);
			emitVert(3);
			EndPrimitive();
			break;
		case 7:
			prepareVert(1);
			prepareVert(2);
			prepareVert(4);
			prepareVert(5);
			prepareVert(6);
			prepareVert(7);
			prepareVert(8);
			emitVert(3);
			emitVert(6);
			emitVert(2);
			emitVert(5);
			emitVert(1);
			emitVert(4);
			emitVert(0);
			EndPrimitive();
			emitVert(6);
			emitVert(8);
			emitVert(5);
			emitVert(7);
			emitVert(4);
			EndPrimitive();
			emitVert(8);
			emitVert(9);
			emitVert(7);
			EndPrimitive();
	};
}
