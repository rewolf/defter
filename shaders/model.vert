#version 150

uniform mat4 	projection;
uniform mat4 	view;
uniform mat4	modelview;
uniform mat4	mvp;

in 	vec3 vert_Position;
in	vec3 vert_Normal;
in	vec2 vert_TexCoord;

out	vec3 frag_Normal;
out	vec2 frag_TexCoord;
out vec4 frag_Pos;

void main(){
	frag_Normal		= normalize(vert_Normal);
	frag_TexCoord	= vert_TexCoord;
	frag_Pos		= modelview * vec4(vert_Position, 1.0);
	gl_Position		= mvp * vec4(vert_Position, 1.0);
};
