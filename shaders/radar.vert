#version 150 core

uniform int pass;

in vec2 quad_Position;
in vec2 vert_texCoord;

out vec2 frag_texCoord;

void main()
{
	gl_Position = vec4(quad_Position, 0.0f, 1.0f);
	frag_texCoord = vert_texCoord;
}
