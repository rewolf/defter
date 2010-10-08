#version 150 core

uniform sampler2D normalmap;
uniform sampler2D colormap;
uniform vec2 click_pos;
uniform vec2 scales;
uniform vec4 cam_and_shift;
uniform float hd_aura;
uniform float is_hd_stamp;

in vec2 hmap_texCoord;
in vec3 position;

out vec4 frag_Color;

// GLOBALS
//--------
const vec4 const_fog_col= vec4(.3, .3, .3, 1.0);
const vec3 light		= -normalize(vec3(.86, -1.4, .5));

//------------------------------------------------------------------------------
void main(){
	vec3 normal, color;
	float fogZ, fogFactor;

	// Set the color tex
	color = texture(colormap, hmap_texCoord).rgb;

	// Add in an overlay for an aura that allows the HD defs in
	if (distance((cam_and_shift.xy + 0.5), hmap_texCoord) < hd_aura * scales.y)
		color	+= is_hd_stamp * (color * vec3(0.0, 0.0, 1.0));

	if (distance(click_pos, hmap_texCoord)< .5 * scales.y)
		color	 = vec3(1.0, 0.0, 0.0);

	// worldspace normal
	normal = normalize(texture(normalmap, hmap_texCoord).rbg*2.0 - 1.0);

	frag_Color = vec4(color * dot(light, normal), 1.0);

	fogZ = dot(position, position);
	fogFactor = 1.0 - exp(-fogZ*.00003);

	frag_Color = mix (frag_Color, const_fog_col, fogFactor);
}
