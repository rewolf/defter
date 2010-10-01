
#version 150 core

uniform float tc_delta;
uniform vec2  stamp_size_scale;
uniform vec2 thingy;

in vec2 vert_Position;
in vec2 vert_in_texCoord;
in vec2 vert_stamp_texCoord;

out vec2 in_texCoord;
out vec2 stamp_texCoord;

void main(){
	vec2 dif = vec2(0.5, 0.5) - fract(thingy);
	gl_Position = vec4(vert_Position * stamp_size_scale -dif,1.0f,1.0f);
	in_texCoord = (vert_Position * stamp_size_scale -dif + 1.0f ) * .5f;
	stamp_texCoord = vert_stamp_texCoord;
}
