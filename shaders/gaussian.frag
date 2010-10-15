
#version 150 core

// Uniforms
uniform sampler2D in_heightmap;
uniform vec2 clickPos;
uniform float intensity;
uniform vec2 stamp_scale;

uniform float falloff;

// Shader Input
in vec2 frag_TexCoord;

// Shader Output
out float height;

void main()
{
	vec2 dist;
	
	height = texture(in_heightmap, frag_TexCoord).r;

	dist = fract(clickPos) - frag_TexCoord;

	height += exp( - dot(dist, dist)  * falloff) * intensity;
}
