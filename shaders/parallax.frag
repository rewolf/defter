/*
/ Copyright © 2010
/ Andrew Flower & Justin Crause
/ Honours Project - Deformable Terrain
*/

#version 150 core
#pragma optionNV inline all

// Uniforms
uniform sampler2D colormap;
uniform sampler2D pdmap;

uniform sampler2D detail0;
uniform sampler2D detail1;
uniform sampler2D detail2;
uniform sampler2D detail3;

uniform sampler2D detail0N;
uniform sampler2D detail1N;
uniform sampler2D detail2N;
uniform sampler2D detail3N;

uniform sampler2D curStamp;

uniform float parallaxBias;
uniform float parallaxScale;
uniform int parallaxItr;

uniform vec2 click_pos;
uniform vec2 scales;
uniform mat4 view;
uniform vec4 cam_and_shift;
uniform float is_hd_stamp;
uniform ivec2 tileOffset;
// .x = hd_aura_sq ; .y = inv_tile_size
uniform vec2 hdasq_its;


// Shader Input
in vec3 frag_View;
in vec3 frag_Tangent;
in vec2 frag_TexCoord;
in vec2 frag_StampTexCoord;


// Shader Ouput
out vec4 frag_Color;


// Constansts
const vec4 light			= normalize(vec4(0.0, 4.0, -10.0, 0.0));
const vec4 fog_col			= vec4(0.6, 0.6, 0.6, 1.0);
const float log2_fog_den	= -0.0000942695;
const vec4 cc				= vec4(1.0, 0.0, -1.0, 2.0);

const vec4 light_Ambient	= vec4(0.2, 0.2, 0.2, 1.0);
const vec4 light_Diffuse	= vec4(0.8, 0.8, 0.8, 1.0);
const vec4 light_Specular	= vec4(0.9, 0.9, 0.9, 1.0);


//------------------------------------------------------------------------------
void main()
{
	// Draw in the clicked position
	vec2 dist 		= click_pos - frag_TexCoord;
	float scale_sq 	= scales.y * scales.y;
	if (dot(dist, dist) < .25 * scale_sq)
	{
		frag_Color = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	// Variables
	vec3 normal, lightDir, viewVec, reflec;
	vec3 pdn;
	vec4 color, ambient, diffuse, specular;
	float diffuseIntensity, specularIntensity, fogZ, fogFactor;

	// creates the vector  <dhdu, 1.0, dhdv> in range [-1,1]
	pdn = texture(pdmap, frag_TexCoord).rrg * cc.wyw + cc.zxz;

	// Read in the noaml from the normal map and calculate in view space
	normal = normalize(mat3(view) * normalize(pdn));

	// Calculate the light in view space and normalize
	lightDir = (mat3(view) * normalize(light.xyz));

	//Normalize the incomming view vector
	viewVec = normalize(frag_View);

	// Get the colour value
	color = texture(colormap, frag_TexCoord * 100.0)
		  + texture(curStamp, frag_StampTexCoord).r * cc.xyyy;

	// Initial variables and settings
	ambient		= light_Ambient;
	diffuse		= light_Diffuse;
	specular	= light_Specular;

	// Calculate the reflec vector
	reflec = -reflect(lightDir, normal);

	// Calculate the diffuse intensity
	diffuseIntensity = max(0.0, dot(normal, lightDir));

	// Calculate the specular intensity
	specularIntensity = pow(max(0.0, dot(reflec, viewVec)), 32);

	// Factor in the intensities to the diffuse and specular amounts
	diffuse	   *= diffuseIntensity;
	specular   *= specularIntensity * 0.3;

	// Calculate the frag color
	frag_Color = (ambient + diffuse + specular) * color;
	

	// Fog controls
	fogZ		= gl_FragCoord.z * (1.0 / gl_FragCoord.w);
	fogFactor	= exp2(log2_fog_den * fogZ * fogZ);
	fogFactor	= clamp(fogFactor, 0.0, 1.0);


	// High-detail maps
	vec2 tile	= frag_TexCoord * hdasq_its.y - tileOffset;
	vec2 tc		= fract(tile);
	int t		= int(floor(tile.x) + floor(tile.y) * 6);
	tile		-= tc;
	float factor= clamp(0.5 + fogZ * 0.018, 0.0, 1.0);
	if (factor > .001){
		vec4 detail;
		vec3 hdnormal;

		vec3 tangent = normalize(mat3(view) * frag_Tangent);
		vec3 binormal = (cross(normal, tangent));

		mat3 tbn = mat3(tangent.x, binormal.x, normal.x,
						tangent.y, binormal.y, normal.y,
						tangent.z, binormal.z, normal.z);
		
		lightDir	= normalize(tbn * lightDir);
		viewVec		= normalize(tbn * viewVec);
		viewVec.y	*= -1.0;

		vec3 parallaxTexcoords	= vec3(tc, 0.0);
		switch(t)
		{
			case 0:
				for (int i = 0; i < parallaxItr; i++)
				{
					vec4 hmap = texture(detail0, parallaxTexcoords.xy);
					float height2 = (hmap.r * parallaxScale) + parallaxBias;
					parallaxTexcoords += (height2 - parallaxTexcoords.z) * viewVec;
				}
				
				hdnormal = texture(detail0N, parallaxTexcoords.xy).rrg * cc.wyw + cc.zxz;
				hdnormal.z = -hdnormal.z;

				normal = normalize(hdnormal);

				color = texture(colormap, (tile + parallaxTexcoords.xy + tileOffset) / hdasq_its.y * 100.0) + texture(curStamp, frag_StampTexCoord).r * cc.xyyy;

				// Calculate the diffuse intensity
				diffuseIntensity = max(0.0, dot(normal, lightDir));
				
				break;
			case 1:
				for (int i = 0; i < parallaxItr; i++)
				{
					vec4 hmap = texture(detail1, parallaxTexcoords.xy);
					float height2 = (hmap.r * parallaxScale) + parallaxBias;
					parallaxTexcoords += (height2 - parallaxTexcoords.z) * viewVec;
				}

				hdnormal = texture(detail1N, parallaxTexcoords.xy).rrg * cc.wyw + cc.zxz;
				hdnormal.z = -hdnormal.z;

				normal = normalize(hdnormal);

				color = texture(colormap, (tile + parallaxTexcoords.xy + tileOffset) / hdasq_its.y * 100.0) + texture(curStamp, frag_StampTexCoord).r * cc.xyyy;

				// Calculate the diffuse intensity
				diffuseIntensity = max(0.0, dot(normal, lightDir));

				break;
			case 6:
				for (int i = 0; i < parallaxItr; i++)
				{
					vec4 hmap = texture(detail2, parallaxTexcoords.xy);
					float height2 = (hmap.r * parallaxScale) + parallaxBias;
					parallaxTexcoords += (height2 - parallaxTexcoords.z) * viewVec;
				}

				hdnormal = texture(detail2N, parallaxTexcoords.xy).rrg * cc.wyw + cc.zxz;
				hdnormal.z = -hdnormal.z;

				normal = normalize(hdnormal);

				color = texture(colormap, (tile + parallaxTexcoords.xy + tileOffset) / hdasq_its.y * 100.0) + texture(curStamp, frag_StampTexCoord).r * cc.xyyy;

				// Calculate the diffuse intensity
				diffuseIntensity = max(0.0, dot(normal, lightDir));

				break;
			case 7:
				for (int i = 0; i < parallaxItr; i++)
				{
					vec4 hmap = texture(detail3, parallaxTexcoords.xy);
					float height2 = (hmap.r * parallaxScale) + parallaxBias;
					parallaxTexcoords += (height2 - parallaxTexcoords.z) * viewVec;
				}

				hdnormal = texture(detail3N, parallaxTexcoords.xy).rrg * cc.wyw + cc.zxz;
				hdnormal.z = -hdnormal.z;


				normal = normalize(hdnormal);
				
				color = texture(colormap, (tile + parallaxTexcoords.xy + tileOffset) / hdasq_its.y * 100.0) + texture(curStamp, frag_StampTexCoord).r * cc.xyyy;

				// Calculate the diffuse intensity
				diffuseIntensity = max(0.0, dot(normal, lightDir));

				break;
		};
		// Factor in the intensities to the diffuse and specular amounts
		diffuse	   *= diffuseIntensity;
		specular   *= 0.0;

		// Calculate the frag color
		detail = (ambient + diffuse + specular) * color;
		frag_Color=  detail;//mix(detail, frag_Color, factor) * cc.xxxy + cc.yyyx;
	}

	// Add in an overlay for an aura that allows the HD defs in
	dist = cam_and_shift.xy + 0.5 - frag_TexCoord;
	if (dot(dist, dist) < hdasq_its.x)
		frag_Color += 2.0 * is_hd_stamp * (color * vec4(0.0, 0.0, 1.0, 1.0));


	// Mix fog to get the final color
	frag_Color = mix(fog_col, frag_Color, fogFactor);
}
