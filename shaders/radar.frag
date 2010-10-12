#version 150 core

// Textures for rendering the background
uniform sampler2D heightmap;
uniform sampler2D colormap;

// Variable to decide what is been drawn currently
uniform int pass;

// Variables to draw the current position as a dot
uniform vec2 currentPos;
uniform float dotRadius;

// Variable to shade in the given area
uniform vec4 tileBounds;
uniform vec4 cellColor;

// Variables for gradient of vision cone
uniform float coneGrad1;
uniform float coneGrad2;

// Variable to draw a line at this position
uniform vec2 linePos;

in vec2 frag_texCoord;
out vec4 frag_Color;

const vec2 const_list = vec2(1.0, .0);

void main()
{
	vec4 color;
	vec4 height;
	vec2 dist;
	float r;

	switch (pass)
	{
		case 0:
			// Read the colour and height values and set it
			color = texture(colormap, frag_texCoord); 
			height= texture(heightmap, frag_texCoord).rrrr * .9 + .1;
			color = color + height * const_list.xxxy + vec4(0.0, 0.0, 0.0, 1.0);
		break;

		case 1:
			// Draw a quad on the screen
			if (frag_texCoord.x > tileBounds.x && frag_texCoord.x < tileBounds.z &&
				frag_texCoord.y > tileBounds.y && frag_texCoord.y < tileBounds.w)
			{
				color = cellColor;
			}
		break;

		case 2:
			//
			dist = frag_texCoord - currentPos;
			if ( (dist.t/dist.s < coneGrad1 ^^ dist.s/dist.t < -coneGrad1) )
			{
				color = vec4(1.0, 0.0, 1.0, 0.3);
			}
		break;

		case 3:
			// Draw verticle line
			if (frag_texCoord.x >= linePos.x && frag_texCoord.x <= linePos.y)
				color = vec4(0.0, 0.0, 0.0, 0.4);

			// Draw horizontal line
			if (frag_texCoord.y >= linePos.x && frag_texCoord.y <= linePos.y)
				color = vec4(0.0, 0.0, 0.0, 0.4);
		break;

		case 4:
			// Draw the dot on the radar
			dist = frag_texCoord - currentPos;
			r = dot(dist, dist);
			if (r < dotRadius)
				color = vec4(0.0, 1.0, 1.0, 1.0);

		break;
	}

	frag_Color	= color;
}
