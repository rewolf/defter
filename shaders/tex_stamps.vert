#version 150 core

// Uniforms
uniform vec2 stamp_scale;
uniform vec2 clickPos;

// Shader Input
in vec2 vert_Position;

// Shader Output
out vec2 frag_TexCoord;
out vec2 stamp_TexCoord;

void main()
{
	vec2 clickFract	= fract(clickPos);
	vec2 dif 		= 0.5 - clickFract;

	vec2 texCoord 	= vert_Position * stamp_scale;

	gl_Position 	= vec4(2 * (texCoord - dif), 1.0, 1.0);
	frag_TexCoord 	= texCoord  + clickFract;

	stamp_TexCoord	= vert_Position * 0.5 + 0.5;
}