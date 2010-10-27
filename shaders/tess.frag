/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core
#pragma optionNV inline all

// Uniforms
uniform sampler2D colormap;

uniform sampler2D detail0;
uniform sampler2D detail1;
uniform sampler2D detail2;
uniform sampler2D detail3;

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
in vec3 frag_Normal;

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
	vec4 color, ambient, diffuse, specular;
	float diffuseIntensity, specularIntensity, fogZ, fogFactor;

	// Read in the noaml from the normal map and calculate in view space
	normal = normalize(mat3(view) * frag_Normal);

	// Calculate the light in view space and normalize
	lightDir = (view * light).xyz;

	//Normalize the incomming view vector
	viewVec = normalize(frag_View);

	// Get the colour value
	color = texture(colormap, frag_TexCoord * 100.0);

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

	// Add in an overlay for an aura that allows the HD defs in
	dist = cam_and_shift.xy + 0.5 - frag_TexCoord;
	if (dot(dist, dist) < hdasq_its.x)
		frag_Color += 2.0 * is_hd_stamp * (color * vec4(0.0, 0.0, 1.0, 1.0));


	// Fog controls
	fogZ		= gl_FragCoord.z * (1.0 / gl_FragCoord.w);
	fogFactor	= exp2(log2_fog_den * fogZ * fogZ);
	fogFactor	= clamp(fogFactor, 0.0, 1.0);
	
	/*

	// High-detail maps
	vec3 detailN = vec4(.0, 1.0, .0);
	switch(tileIndex)
	{
		case 0:
			detail =  texture(detail0N, tc).rrrr;
			break;                   
		case 1:                     
			detail =  texture(detail1N, tc).rrrr;
			break;                   
		case 6:                      
			detail =  texture(detail2N, tc).rrrr;
			break;                   
		case 7:                      
			detail =  texture(detail3N, tc).rrrr;
			break;
	};
*/

	// Mix fog to get the final color
	frag_Color = mix(fog_col, frag_Color, fogFactor);
}
