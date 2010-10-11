#version 150 core

uniform sampler2D colormap;
uniform sampler2D normalmap;
uniform vec2 click_pos;
uniform vec2 scales;
uniform mat4 view;
uniform vec4 cam_and_shift;
uniform float hd_aura;
uniform float is_hd_stamp;

in vec3 frag_View;
in vec2 frag_TexCoord;
in vec3 position;

out vec4 frag_Color;

// GLOBALS
//--------
const vec4 light		= normalize(vec4(.0, 4.0, -10.0, 0.0));
const vec4 fog_col		= vec4(0.6, 0.6, 0.6, 1.0);
const float log2		= 1.442695;
const float fog_density	= 0.01;

vec4 light_Ambient	= vec4(0.2, 0.2, 0.2, 1.0);
vec4 light_Diffuse	= vec4(0.8, 0.8, 0.8, 1.0);
vec4 light_Specular	= vec4(0.9, 0.9, 0.9, 1.0);

//------------------------------------------------------------------------------
void main()
{
	// Draw in the clicked position
	if (distance(click_pos, frag_TexCoord)< .5 * scales.y)
	{
		frag_Color = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	// Variables
	vec3 normal, lightDir, viewVec, reflec;
	vec4 color, ambient, diffuse, specular;
	float diffuseIntensity, specularIntensity, fogZ, fogFactor;

	// Read in the noaml from the normal map and calculate in view space
	normal = normalize(mat3(view) * (texture2D(normalmap, frag_TexCoord).rbg * 2.0 - 1.0));

	// Calculate the light in view space and normalize
	lightDir = normalize((view * light).xyz);

	//Normalize the incomming view vector
	viewVec = normalize(frag_View);

	// Get the colour value
	color = texture(colormap, frag_TexCoord * 100.0);

	// Initial variables and settings
	ambient = light_Ambient;
	diffuse = light_Diffuse;
	specular = light_Specular;

	// Calculate the reflec vector
	reflec = normalize(-reflect(lightDir, normal));

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
	if (distance((cam_and_shift.xy + 0.5), frag_TexCoord) < hd_aura * scales.y)
		frag_Color += 2.0 * is_hd_stamp * (color * vec4(0.0, 0.0, 1.0, 1.0));

	// Fog controls
	fogZ		= gl_FragCoord.z / gl_FragCoord.w;
	fogFactor	= exp2(-fog_density * fog_density * fogZ * fogZ * log2);
	fogFactor	= clamp(fogFactor, 0.0, 1.0);

	// Mix to get the final color
	frag_Color = mix(fog_col, frag_Color, fogFactor);
}
