#version 150

in vec2 vert_Position;

out vec2 frag_TexCoord;

const vec3 cc = vec3(1.0, .0, -1.0);

void main(){
	frag_TexCoord = vert_Position * .5 + .5;
	gl_Position   = vert_Position.xyxx * cc.xxyy + cc.yyyx;
}
