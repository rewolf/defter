#version 150 core

// Variable to decide what is been drawn currently
uniform int pass;

// Variable to draw a line at this position
uniform vec2 linePos;

// Variable to shade in the given area
uniform vec4 tileBounds;
uniform vec4 cellColor;

in vec2 frag_texCoord;
out vec4 frag_Color;

void main()
{
	vec4 color;

	switch (pass)
	{
		case 0:
			color = vec4(0.0, 0.0, 0.0, 1.0);

		case 1:
			// Draw a quad on the screen
			if (frag_texCoord.x > tileBounds.x && frag_texCoord.x < tileBounds.z &&
				frag_texCoord.y > tileBounds.y && frag_texCoord.y < tileBounds.w)
			{
				color = cellColor;
			}
		break;

		case 2:
			// Draw verticle line
			if (frag_texCoord.x >= linePos.x && frag_texCoord.x <= linePos.y)
				color = vec4(0.0, 0.0, 1.0, 1.0);

			// Draw horizontal line
			if (frag_texCoord.y >= linePos.x && frag_texCoord.y <= linePos.y)
				color = vec4(0.0, 0.0, 1.0, 1.0);
		break;
	}

	frag_Color	= color;
}
