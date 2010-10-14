
#version 150 core

uniform float tc_delta;
uniform vec2 size_scale;
uniform vec2 thingy;

in vec2 vert_Position;
in vec2 vert_in_texCoord;
in vec2 vert_stamp_texCoord;

out vec2 frag_texCoord;
out vec2 stamp_texCoord;

void main(){
	vec2 ff 		= fract(thingy);
	vec2 dif 		= 0.5 - ff;

	vec2 posScaled 	= vert_Position * size_scale;

	gl_Position 	= vec4(2*(posScaled - dif), 1.0f,1.0f);
	frag_texCoord 	= posScaled  + ff;
	stamp_texCoord 	= vert_stamp_texCoord;
}
