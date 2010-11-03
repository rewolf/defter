#version 150

uniform float 	R;

in		vec2 	position;
out 	float 	new_height;

void main(){
	float r = length(position) - R;
	new_height = exp(-r*r*1600);
}
