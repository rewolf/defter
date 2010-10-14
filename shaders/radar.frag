#version 150 core

// Textures for rendering the background
uniform sampler2D heightmap;
uniform sampler2D colormap;

// Values to offset the texture lookups
uniform vec2 offset;
uniform float scale;
uniform float aspectRatio;

// Variable to decide what is been drawn currently
uniform int pass;

// Variables to draw the current position as a dot
uniform vec2 currentPos;
uniform float dotRadius;

// Variable to shade in the given area
uniform vec4 tileBounds;
uniform vec4 cellColor;

// Variables for gradient of vision cone
uniform mat2 viewRotation;

// Variable to draw a line at this position
uniform vec2 linePos;

in vec2 frag_TexCoord;
out vec4 frag_Color;

const vec2 const_list = vec2(1.0, 0.0);

void main()
{
	vec2 dist, texCoord;
	vec4 color;
	vec4 height;
	float r;

	// Run a specified pass to complete different tasks
	switch (pass)
	{
		case 0:
			// Read the colour and height values and set it
			texCoord	= (frag_TexCoord * scale) + offset;
			color		= texture(colormap, texCoord);
			height		= texture(heightmap, texCoord).rrrr * .9 + .1;
			color		= color + height * const_list.xxxy + const_list.yyyx;
		break;

		case 1:
			// Draw a quad on the screen
			if (frag_TexCoord.x > tileBounds.x && frag_TexCoord.x < tileBounds.z &&
				frag_TexCoord.y > tileBounds.y && frag_TexCoord.y < tileBounds.w)
			{
				color = cellColor;
			}
		break;

		case 2:
			// Draw the vision cone
			dist = frag_TexCoord - currentPos;
			dist = viewRotation * dist;
			if (-dist.t * aspectRatio > abs(dist.s))
			{
				color = vec4(1.0, 0.0, 1.0, 0.3);
			}
		break;

		case 3:
			// Draw verticle line
			if (frag_TexCoord.x >= linePos.x && frag_TexCoord.x <= linePos.y)
				color = vec4(0.0, 0.0, 0.0, 0.4);

			// Draw horizontal line
			if (frag_TexCoord.y >= linePos.x && frag_TexCoord.y <= linePos.y)
				color = vec4(0.0, 0.0, 0.0, 0.4);
		break;

		case 4:
			// Draw the dot on the radar
			dist = frag_TexCoord - currentPos;
			r = dot(dist, dist);
			if (r < dotRadius)
				color = vec4(0.0, 1.0, 1.0, 1.0);

		break;
	}

	frag_Color	= color;
}
