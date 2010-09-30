
#version 150 core

uniform float tc_delta;

in vec2 vert_Position;
in vec2 vert_in_texCoord;
in vec2 vert_stamp_texCoord;

out vec2 in_texCoord;
out vec2 stamp_texCoord;

void main(){
	gl_Position = vec4(vert_Position,1.0f,1.0f);
	in_texCoord = (vert_Position + 1.0f ) * .5f;
	stamp_texCoord = vert_stamp_texCoord;
}
