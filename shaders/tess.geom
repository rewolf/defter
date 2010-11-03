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
	frag_Normal		= out_norms[idx];	\
	frag_View		= -out_verts[idx].xyz * (1.0/out_verts[idx].w); \
	gl_Position     = projection * out_verts[idx];	\
	EmitVertex();


// Declare the incoming primitive type
layout(triangles) in;

// Declare the 3esulting primitive type
layout(triangle_strip, max_vertices=170)  out;

// Uniforms
uniform sampler2D heightmap;
uniform sampler2D pdmap;
uniform mat4 projection;
uniform mat4 view;
uniform float camera_height;
uniform vec2 hdasq_its;
uniform ivec2 tileOffset;

// detail maps
uniform sampler2D detail0;
uniform sampler2D detail1;
uniform sampler2D detail2;
uniform sampler2D detail3;
uniform sampler2D detail0N;
uniform sampler2D detail1N;
uniform sampler2D detail2N;
uniform sampler2D detail3N;

// Incoming from vertex shader
in vec2 geom_TexCoord[3];
in vec4 geom_ProjPos[3];
in float mustTess[3];


// Outgoing per-vertex information
out vec3 frag_View;
out vec2 frag_TexCoord;
out vec3 frag_Normal;

int prepareVert(in int idx);
void refine_with_pattern();

// Globals
const vec4 cc = vec4(1.0, .0, -1.0, 2.0);
const float HEIGHT = 40.0;

mat3x4 stuff;
mat3x2 stuff2;

vec4 out_verts   [10];
vec3 barycentric [10];
vec2 out_tex[10];
vec3 out_norms[10];
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

	stuff = view*mat3x4(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
	stuff2= mat3x2(geom_TexCoord[0], geom_TexCoord[1], geom_TexCoord[2]);

	// Gather components for frustum culling below
	x = abs(vec3(vertex[0].x, vertex[1].x, vertex[2].x));
	y = abs(vec3(vertex[0].y, vertex[1].y, vertex[2].y));
	z = -vec3(vertex[0].z, vertex[1].z, vertex[2].z);
	w = vec3(vertex[0].w, vertex[1].w, vertex[2].w);


	if (!any(lessThan(x, w*1.1)))
		return;
	if (!any(lessThan(y, w*1.1)))
		return;
	if (!any(lessThan(z, w*1.1)))
		return;

	refine_with_pattern();
}

//--------------------------------------------------------
void refine_with_pattern(){
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
	int mustTess[3];
	mustTess[0]=prepareVert(9);
	mustTess[1]=prepareVert(0);
	mustTess[2]=prepareVert(3);
	int index = (mustTess[0] << 0) | 
				(mustTess[1] << 1) | 
				(mustTess[2] << 2);
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
			emitVert(9);
			prepareVert(7);
			emitVert(7);
			emitVert(3);
			prepareVert(4);
			emitVert(4);
			emitVert(0);
			EndPrimitive();
			break;
		case 5:
			emitVert(3);
			prepareVert(6);
			emitVert(6);
			emitVert(0);
			prepareVert(8);
			emitVert(8);
			emitVert(9);
			EndPrimitive();
			break;
		case 6:
			emitVert(0);
			prepareVert(1);
			emitVert(1);
			emitVert(9);
			prepareVert(2);
			emitVert(2);
			emitVert(3);
			EndPrimitive();
			break;
		case 7:
			emitVert(3);
			prepareVert(6);
			emitVert(6);
			prepareVert(2);
			emitVert(2);
			prepareVert(5);
			emitVert(5);
			prepareVert(1);
			emitVert(1);
			prepareVert(4);
			emitVert(4);
			emitVert(0);
			EndPrimitive();
			emitVert(6);
			prepareVert(8);
			emitVert(8);
			emitVert(5);
			prepareVert(7);
			emitVert(7);
			emitVert(4);
			EndPrimitive();
			emitVert(8);
			emitVert(9);
			emitVert(7);
			EndPrimitive();
	};
}
uniform vec4 cam_and_shift;
//--------------------------------------------------------
int prepareVert(in int idx){
	vec4 temp;
	// Interpolat texcoords and vertex position
	out_tex[idx] = stuff2 * barycentric[idx].xyz;
	temp		 = stuff * barycentric[idx].xyz;

	// High-detail maps
	vec2 tile	= out_tex[idx] * hdasq_its.y - tileOffset;
	vec2 tc		= fract(tile);
	int t		= int(floor(tile.x) + floor(tile.y) * 6);

	// Read coarsemap normal
	out_norms[idx] = texture(pdmap, out_tex[idx]).rgg*cc.wyw + cc.zxz;

	float dist  = barycentric[idx].x * mustTess[0]
				+ barycentric[idx].y * mustTess[1]
				+ barycentric[idx].z * mustTess[2];
	float factor= 1.0;//clamp(1.0 - .1*dist, .0, 1.0);
	float detail = .0;
	vec3 add = vec3(.0, .0, .0);
	switch(t)
	{
		case 0:
			detail =  texture(detail0, tc).r;
			add.xz = (texture(detail0N, tc).rg*2 - 1);
			break;
		case 1:
			detail =  texture(detail1, tc).r;
			add.xz = (texture(detail1N, tc).rg*2 - 1);
			break;
		case 6:
			detail =  texture(detail2, tc).r;
			add.xz = (texture(detail2N, tc).rg*2 - 1);
			break;
		case 7:
			detail =  texture(detail3, tc).r;
			add.xz = (texture(detail3N, tc).rg*2 - 1);
			break;
	};
	out_norms[idx] += add * factor;
	out_verts[idx] = temp + cc.yxyy * detail * .5 *factor;
	//out_verts[idx].xyz = temp.xyz + normalize(out_norms[idx]) * detail * .5;
	//out_verts[idx].w = 1.0;
	return int(detail > .0f);
}

