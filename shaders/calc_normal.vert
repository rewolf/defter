
#version 150 core

uniform float tc_delta;
uniform vec2 stamp_size_scale;
uniform vec2 thingy;

in vec2 vert_Position;
in vec2 vert_in_texCoord;

out vec2 in_texCoord;

void main(){
	vec2 dif = vec2(0.5, 0.5) - fract(thingy);
	vec2 posScaled 	= vert_Position * stamp_size_scale;
	gl_Position = vec4(posScaled - 2 * dif,1.0f,1.0f);
	in_texCoord = (posScaled + 1.0f ) * .5f - dif;
}
