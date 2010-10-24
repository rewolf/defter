#version 150

uniform sampler2D 	current;
uniform sampler2D 	previous;
// factors.x = wave eqn parameter   ;  factors.y = dx
uniform vec2		factors;

in 	vec2 	frag_TexCoord;
out float 	new_height;

const vec3 cc = vec3(1.0, .0, -1.0);

void main(){
	float top, bottom, left, right, centre, old;

	old		= texture(previous, frag_TexCoord).r;
	centre 	= texture(current,  frag_TexCoord).r;
	left 	= texture(current,  frag_TexCoord + factors.y * cc.zy).r;
	right 	= texture(current,  frag_TexCoord + factors.y * cc.xy).r;
	top		= texture(current,  frag_TexCoord + factors.y * cc.yx).r;
	bottom	= texture(current,  frag_TexCoord + factors.y * cc.yz).r;

	new_height = 2 * centre - old 
		+ factors.x * ( right + left + top + bottom - 4 * centre);
}
