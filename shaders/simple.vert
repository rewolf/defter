#version 150 core
#pragma optionNV unroll all
#pragma optionNV inline all


// UNIFORMS
//---------
uniform sampler2D heightmap;
// texToMetre = .x  ; metreToTex = .y
uniform vec2 scales;
// cam = .xy  ; shift = .zw   interleaving
uniform vec4 cam_and_shift;
uniform mat4 projection;
uniform mat4 view;


// VARYINGS
//---------
in vec3 in_Position;
in vec2 in_TexCoord;

out vec2 geom_TexCoord;

// GLOBALS
//--------
const vec2  const_list	= vec2(1.0,  .0);
const float HEIGHT 		= 20.0;

//------------------------------------------------------------------------------
// NB:
// Clipmap is centred on origin but the origin texel has coordinate 0.5,0.5
// the camera, starts at 0,0
void main()
{
	float height, lod, texToMetre, metreToTex;
	vec2 texCoord, camera_world, shift, camera_tex;
	float camera_height;

	camera_tex	= cam_and_shift.xy;
	shift	 	= cam_and_shift.zw;
	texToMetre	= scales.x;
	metreToTex	= scales.y;

	// Convert camera position to world space
	camera_world = camera_tex * texToMetre;

	// Compute texture coordinates for vertex and lookup height
	texCoord = in_TexCoord + camera_tex + shift*metreToTex;

	// Get the height of vertex, and the height at the camera position
	// Vertex height samples the mipmap level corresponding to this clipmap level
	height 	= texture(heightmap, texCoord).r;
	camera_height = -texture(heightmap, 0.5+camera_tex, .0).r * HEIGHT - 2.5;

	// Set vertex position and height from heightmap
	gl_Position = vec4(in_Position.x, height*HEIGHT, in_Position.y, 1.0);

	// Shift the roaming mesh so that vertices maintain same heights
	// The following MAD instruction shifts the x and z coordinates by s and t
	gl_Position = gl_Position + const_list.xyxy * shift.sttt;
	// So gl_Position contains the basic heightfield worldspace coordinate

	// Pos contains the transformed coordinate in eye-space.
	gl_Position = projection * view * (const_list.yxyy * camera_height + gl_Position);
	
	// Send through the texcoord
	geom_TexCoord = texCoord;
}
