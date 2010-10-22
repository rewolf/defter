/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core
#pragma optionNV unroll all
#pragma optionNV inline all

// Uniforms
uniform sampler2D heightmap;
// texToMetre = .x  ; metreToTex = .y
uniform vec2 scales;
// cam = .xy  ; shift = .zw   interleaving
uniform vec4 cam_and_shift;
uniform float cam_height;
uniform mat4 projection;
uniform mat4 view;


// Shader Input
in vec3 vert_Position;
in vec2 vert_TexCoord;


// Shader Output
out vec3 geom_View;
out vec2 geom_TexCoord;


// Constansts
const vec2  const_list	= vec2(1.0,  .0);
const float HEIGHT 		= 40.0;


//------------------------------------------------------------------------------
// NB:
// Clipmap is centred on origin but the origin texel has coordinate 0.5, 0.5
// the camera, starts at 0, 0
//------------------------------------------------------------------------------
void main()
{
	// Variables
	float height, lod, texToMetre, metreToTex;
	vec2 texCoord, camera_world, shift, camera_tex;
	float camera_height;

	// Extract data passed in
	camera_tex	= cam_and_shift.xy;
	shift	 	= cam_and_shift.zw;
	texToMetre	= scales.x;
	metreToTex	= scales.y;

	// Convert camera position to world space
	camera_world = camera_tex * texToMetre;

	// Compute texture coordinates for vertex and lookup height
	texCoord = vert_TexCoord + camera_tex + shift * metreToTex;

	// Get the height of vertex, and the height at the camera position
	// Vertex height samples the mipmap level corresponding to this clipmap level
	height 	= texture(heightmap, texCoord).r*0;
	camera_height = -20;//-cam_height;

	// Set vertex position and height from heightmap
	vec4 pos = vec4(vert_Position.x, height * HEIGHT, vert_Position.y, 1.0);

	// Shift the roaming mesh so that vertices maintain same heights
	// The following MAD instruction shifts the x and z coordinates by s and t
	pos = pos + const_list.xyxy * shift.sttt;
	// So gl_Position contains the basic heightfield worldspace coordinate
	pos = pos + const_list.yxyy * camera_height;

	// Pos contains the transformed coordinate in eye-space.
	pos = view * pos;

	// Calculate the view vector
	geom_View = -pos.xyz * (1.0 / pos.w);
	
	// Save out the gl_Position
	gl_Position = projection * pos;
	
	// Save out the texCoord
	geom_TexCoord = texCoord;
}
