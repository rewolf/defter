#version 150 core
#pragma optionNV inline all

uniform sampler2D colormap;
uniform sampler2D pdmap;
uniform vec2 click_pos;
uniform vec2 scales;
uniform mat4 view;
uniform vec4 cam_and_shift;
uniform float hd_aura_sq;
uniform float is_hd_stamp;
uniform float inv_tile_size;
uniform ivec2 tileOffset;

in vec3 frag_View;
in vec2 frag_TexCoord;
in vec3 position;

out vec4 frag_Color;

// GLOBALS
//--------
const vec4 light		= normalize(vec4(.0, 4.0, -10.0, 0.0));
const vec4 fog_col		= vec4(0.6, 0.6, 0.6, 1.0);
const float log2_fog_density	= -0.0001442695;

vec4 light_Ambient	= vec4(0.2, 0.2, 0.2, 1.0);
vec4 light_Diffuse	= vec4(0.8, 0.8, 0.8, 1.0);
vec4 light_Specular	= vec4(0.9, 0.9, 0.9, 1.0);

//------------------------------------------------------------------------------
void main()
{
	// Draw in the clicked position
	vec2 dist 		= click_pos - frag_TexCoord;
	float scale_sq 	= scales.y*scales.y;
	if (dot(dist,dist) < .25 * scale_sq)
	{
		frag_Color = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	// Variables
	vec3 normal, lightDir, viewVec, reflec;
	vec3 pdn;
	vec4 color, ambient, diffuse, specular;
	float diffuseIntensity, specularIntensity, fogZ, fogFactor;

	pdn = vec3(texture(pdmap, frag_TexCoord).xy * 2 - 1, 1.0).rbg;

	// Read in the noaml from the normal map and calculate in view space
	normal = normalize(mat3(view) * pdn);

	// Calculate the light in view space and normalize
	lightDir = (view * light).xyz;

	//Normalize the incomming view vector
	viewVec = normalize(frag_View);

	// Get the colour value
	color = texture(colormap, frag_TexCoord * 100.0);

	// Initial variables and settings
	ambient = light_Ambient;
	diffuse = light_Diffuse;
	specular = light_Specular;

	// Calculate the reflec vector
	reflec = -reflect(lightDir, normal);

	// Calculate the diffuse intensity
	diffuseIntensity = max(0.0, dot(normal, lightDir));

	// Calculate the specular intensity
	specularIntensity = pow(max(0.0, dot(reflec, viewVec)), 32);

	// Factor in the intensities to the diffuse and specular amounts
	diffuse *= diffuseIntensity;
	specular *= specularIntensity * 0.3;

	// Calculate the final color
	frag_Color = (ambient + diffuse + specular) * color;

	// Add in an overlay for an aura that allows the HD defs in
	dist = cam_and_shift.xy + 0.5 - frag_TexCoord;
	if (dot(dist, dist) < hd_aura_sq)
		frag_Color += 2.0 * is_hd_stamp * (color * vec4(0.0, 0.0, 1.0, 1.0));

	// Fog controls
	fogZ		= gl_FragCoord.z * (1.0/gl_FragCoord.w);
	fogFactor	= exp2(log2_fog_density * fogZ * fogZ);
	fogFactor	= clamp(fogFactor, 0.0, 1.0);

	// Mix to get the final color
	frag_Color = mix(fog_col, frag_Color, fogFactor);

	/*

	ivec2 tile = ivec2((frag_TexCoord )* inv_tile_size) - tileOffset;
	int t = tile.x + tile.y*6;
	switch(t){
		case 0:
			frag_Color=  vec4(.0, .0, 1.0, 1.0);
			break;
		case 1:
			frag_Color=  vec4(.0, 1.0, .0, 1.0);
			break;
		case 6:
			frag_Color=  vec4(.0, 1.0, 1.0, 1.0);
			break;
		case 7:
			frag_Color=  vec4(1.0, .0, .0, 1.0);
			break;
	};
	*/
}

