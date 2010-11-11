#version 150

in vec2 vert_Position;

out vec2 position;

const vec3 cc = vec3(1.0, .0, -1.0);

void main(){
	position 	  = vert_Position;
	gl_Position   = vert_Position.xyxx * cc.xxyy + cc.yyyx;
}
