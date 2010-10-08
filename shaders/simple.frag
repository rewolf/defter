#version 150 core

uniform sampler2D normalmap;
uniform sampler2D colormap;
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
const vec4 const_fog_col= vec4(.3, .3, .3, 1.0);

vec4 light_Ambient	= vec4(0.2, 0.2, 0.2, 1.0);
vec4 light_Diffuse	= vec4(0.8, 0.8, 0.8, 1.0);
vec4 light_Specular	= vec4(0.9, 0.9, 0.9, 1.0);

//------------------------------------------------------------------------------
void main()
{
	if (distance(click_pos, frag_TexCoord)< .5 * scales.y)
	{
		frag_Color = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	// Read in the noaml from the normal map and calculate in view space
	vec3 normal = normalize(mat3(view) * (texture2D(normalmap, frag_TexCoord).rbg * 2.0 - 1.0));

	// Calculate the light in view space and normalize
	vec3 lightDir = normalize((view * light).xyz);

	//Normalize the incomming view vector
	vec3 viewVec = normalize(frag_View);

	// Get the colour value
	vec4 color = texture(colormap, frag_TexCoord);//vec4(vec3(1.0, 1.0, 1.0) * 0.5, 1.0);//

	// Add in an overlay for an aura that allows the HD defs in
	if (distance((cam_and_shift.xy + 0.5), frag_TexCoord) < hd_aura * scales.y)
		color	+= 2.0 * is_hd_stamp * (color * vec4(0.0, 0.0, 1.0, 1.0));
    
	// Initial variables and settings
	vec4 ambient = light_Ambient;
	vec4 diffuse = light_Diffuse;
	vec4 specular = light_Specular;

	// Calculate the reflec vector
	vec3 reflec = normalize(-reflect(lightDir, normal));

	// Calculate the diffuse intensity
	float diffuseIntensity = max(0.0, dot(normal, lightDir));

	// Calculate the specular intensity
	float specularIntensity = pow(max(0.0, dot(reflec, viewVec)), 64);

	// Factor in the intensities to the diffuse and specular amounts
	diffuse *= diffuseIntensity;
	specular *= specularIntensity;

	// Calculate the final color
	frag_Color = (ambient + diffuse + specular) * color;

	// Fog controls
	float fogZ		= dot(position, position);
	float fogFactor	= 1.0 - exp(-fogZ * .00003);
	frag_Color = mix (frag_Color, const_fog_col, fogFactor);
}
