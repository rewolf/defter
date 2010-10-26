/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core
#pragma optionNV inline all

// Uniforms
uniform sampler2D heightmap;
uniform sampler2D colormap;
uniform sampler2D pdmap;

uniform sampler2D detail0;
uniform sampler2D detail1;
uniform sampler2D detail2;
uniform sampler2D detail3;

uniform sampler2D detail0N;
uniform sampler2D detail1N;
uniform sampler2D detail2N;
uniform sampler2D detail3N;

uniform float tc_delta;
uniform float parallaxBias;
uniform float parallaxScale;
uniform int parallaxItr;

uniform vec2 click_pos;
uniform vec2 scales;
uniform mat4 view;
uniform vec4 cam_and_shift;
uniform float is_hd_stamp;
uniform ivec2 tileOffset;
// .x = hd_aura_sq ; .y = inv_tile_size
uniform vec2 hdasq_its;


// Shader Input
in vec3 frag_View;
in vec2 frag_TexCoord;


// Shader Ouput
out vec4 frag_Color;


// Constansts
const vec4 light		= normalize(vec4(0.0, 4.0, -10.0, 0.0));
const vec4 fog_col		= vec4(0.6, 0.6, 0.6, 1.0);
const float log2_fog_den= -0.0001442695;
const vec4 cc			= vec4(1.0, 0.0, -1.0, 2.0);

const vec4 light_Ambient	= vec4(0.2, 0.2, 0.2, 1.0);
const vec4 light_Diffuse	= vec4(0.8, 0.8, 0.8, 1.0);
const vec4 light_Specular	= vec4(0.9, 0.9, 0.9, 1.0);

//const float parallaxScale = 0.0006;
//const float parallaxBias = -0.0;
//const int parallaxItrNum = 1;

//------------------------------------------------------------------------------
void main()
{
	// Draw in the clicked position
	vec2 dist 		= click_pos - frag_TexCoord;
	float scale_sq 	= scales.y * scales.y;
	if (dot(dist, dist) < .25 * scale_sq)
	{
		frag_Color = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	// Variables
	vec3 normal, lightDir, viewVec, reflec;
	vec3 pdn;
	vec4 color, ambient, diffuse, specular;
	float diffuseIntensity, specularIntensity, fogZ, fogFactor;

	// Calculate the light in view space and normalize
	lightDir = normalize(mat3(view) * light.xyz);

	//Normalize the incomming view vector
	viewVec = normalize(frag_View);

	normal = normalize(mat3(view) * vec3(0.0, 1.0, 0.0));
	vec3 tangent = normalize(mat3(view) * vec3(1.0, 0.0, 0.0));
	vec3 binormal = normalize(cross(normal, tangent));

	mat3 tbn = mat3(tangent.x, binormal.x, normal.x,
					tangent.y, binormal.y, normal.y,
					tangent.z, binormal.z, normal.z);

	lightDir = tbn * lightDir;
	viewVec = tbn * viewVec;
	
	vec3 parallaxTexcoords = vec3(frag_TexCoord, 0.0);
	//vec2 parallaxTexcoords = frag_TexCoord;

	parallaxTexcoords.y = 1.0 - parallaxTexcoords.y;

	//float height2 = (texture(heightmap, parallaxTexcoords.xy).r * parallaxScale) + parallaxBias;
	//parallaxTexcoords += height2 * viewVec.xy;

	for (int i = 0; i < parallaxItr; i++)
	{
		vec4 hmap = texture(heightmap, parallaxTexcoords.xy);
		float height2 = (hmap.r * parallaxScale) + parallaxBias;
		parallaxTexcoords += (height2 - parallaxTexcoords.z) * 1.0 * viewVec;
	}

	// creates the vector  <dhdu, 1.0, dhdv> in range [-1,1]
	pdn = texture(pdmap, parallaxTexcoords.xy).rrg * cc.wyw + cc.zxz;


	// Read in the noaml from the normal map and calculate in view space
	//normal = normalize(mat3(view) * pdn);
	normal = normalize(pdn);

	frag_Color = vec4(normal, 1.0);
	//return;

	// Get the colour value
	parallaxTexcoords.y = 1.0 - parallaxTexcoords.y;
	color = texture(colormap, parallaxTexcoords.xy * 100.0);

	// Initial variables and settings
	ambient		= light_Ambient;
	diffuse		= light_Diffuse;
	specular	= light_Specular;

	// Calculate the reflec vector
	reflec = -reflect(lightDir, normal);

	// Calculate the diffuse intensity
	diffuseIntensity = max(0.0, dot(normal, lightDir));

	// Calculate the specular intensity
	specularIntensity = pow(max(0.0, dot(reflec, viewVec)), 32);

	// Factor in the intensities to the diffuse and specular amounts
	diffuse	   *= diffuseIntensity;
	specular   *= specularIntensity * 0.3;

	// Calculate the frag color
	frag_Color = (ambient + diffuse + specular) * color;
}
