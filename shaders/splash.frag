#version 150 core

// Textures for rendering the splash
uniform sampler2D splashmap;

in vec2 frag_TexCoord;
out vec4 frag_Color;

void main()
{
	frag_Color = texture(splashmap, frag_TexCoord);
}
