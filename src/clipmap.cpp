/*****************************************************************************
 * clipmap: Provides a nLevel clipmap for LOD applications
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"
#include "re_math.h"
#include <vector>
using namespace reMath;
#include <map>
using namespace std;
#include "re_shader.h"
#include "util.h"
#include "deform.h"
#include "clipmap.h"

#ifdef _WIN32
	float log2(float n)
	{
		return logf(n)/logf(2);
	};
#endif

//--------------------------------------------------------
Clipmap::Clipmap(int nVerts, float quad_size, int nLevels, int heightmap_dim)
{
	if (((nVerts + 1) & nVerts) != 0)
	{
		int pot = 1 << int(ceil(log2((float)nVerts + 1.0f)));
		printf("Warning\n\tnVerts must be an integer  2^k - 1, Rounding up %d -> %d\n\t\t\t\t", nVerts, pot-1);
		nVerts = pot-1;
	}

	m_cullingEnabled			= true;
	m_N					= nVerts;
	m_nLevels			= nLevels;
	m_quad_size			= quad_size;
	m_heightmap_dim		= heightmap_dim;
	m_min_draw_count	= 0;

	//////////////////////////////////////////////////
	// Just some scaling settings
	// 1 unit = 1 metre
	// NB The finest clip level quads should match texel resolution
	// => texelsize = 1.0/texture_dimension
	// The size of the finest quads will determine the physical area a heightmap will represent
	
	m_texel_size	= 1.0f/(heightmap_dim);
	m_M				= (m_N + 1) / 4; // block size

	m_metre_to_tex 	= m_texel_size/quad_size;
	m_tex_to_metre 	= 1.0f/m_metre_to_tex;

	m_clipmap_stats = "";
	// Output settings to string
	stringstream sstr;
	sstr.setf(ios::fixed, ios::floatfield);
	sstr.precision(3);
	sstr << "Clipmap levels:\t\t\t"			<< m_nLevels				<< "\n";
	sstr << "Finest quad size:\t\t"			<< m_quad_size				<< "m\n";
	sstr << "Vertices per ring-side:\t\t"	<< m_N						<< "\n";
	sstr.precision(4);
	sstr << "Sampling distance:\t\t"		<< m_texel_size				<< "\n";
	sstr.precision(2);
	sstr << "Effective heightmap size:\t"	<< heightmap_dim * quad_size<< "m\n";

	m_clipmap_stats += sstr.str();
}


//--------------------------------------------------------
Clipmap::~Clipmap()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(6, m_vbo);
	glDeleteVertexArrays(2, m_vao);
}

//--------------------------------------------------------
bool
Clipmap::init()
{
	int i,j;
	float quad_size, texel_size, left, ffar;
	int vmarker, vcount;
	std::vector <vector2>	vertices,  vertices_inner;
	std::vector <vector2>	texcoords, texcoords_inner;
	std::vector <GLuint>	indices;
	std::vector <GLuint>	indices_inner;
	std::vector <GLuint>	cullable;

	quad_size  = m_quad_size;
	texel_size = m_texel_size;

	vcount = 0;  // The index of the next vertex (ie the number of vertices so far)

	// Create the inner grid
	left = -(m_N - 1) * 0.5f;
	ffar = -(m_N - 1) * 0.5f;
	for (i = 0; i < m_N; i++)
	{
		for (j = 0; j < m_N; j++)
		{
			vector2 v;
			vector2 tc;

			v.x = left * quad_size + j * quad_size;
			v.y = ffar * quad_size + i * quad_size;
			tc.x= 0.5f + left * texel_size + j * texel_size;
			tc.y= 0.5f + ffar * texel_size + i * texel_size;;

			vertices_inner.push_back(v);
			texcoords_inner.push_back(tc);
			if (i > 0 && j > 0)
			{
				indices_inner.push_back( vcount-1 );
				indices_inner.push_back( vcount-m_N );
				indices_inner.push_back( vcount-m_N-1 );

				indices_inner.push_back( vcount-m_N );
				indices_inner.push_back( vcount-1 );
				indices_inner.push_back( vcount );
			}
			vcount++;
		}
	}
	vcount= 0;

	// Create a footprint
	left = -(m_M - 1) * .5f;
	ffar = -(m_M - 1) * 0.5f;
	for (i = 0; i < m_M; i++)
	{
		for (j = 0; j < m_M; j++)
		{
			vector2 v;
			vector2 tc;

			v.x = left * quad_size + j * quad_size;
			v.y = ffar * quad_size + i * quad_size;
			tc.x= 0.5f + left * texel_size + j * texel_size;
			tc.y= 0.5f + ffar * texel_size + i * texel_size;;

			vertices.push_back(v);
			texcoords.push_back(tc);
			if (i > 0 && j > 0)
			{
				indices.push_back( vcount-1 );
				indices.push_back( vcount-m_M );
				indices.push_back( vcount-m_M-1 );

				indices.push_back( vcount-m_M );
				indices.push_back( vcount-1 );
				indices.push_back( vcount );
			}
			vcount++;
		}
	}


	m_min_draw_count = indices.size();

	int nVerts = vertices.size() + vertices_inner.size();
	int nTex   = texcoords.size() + texcoords_inner.size();
	int nInds  = indices.size() + indices_inner.size();
	int bytes = sizeof(vector2) * (nVerts + nTex)
			  + sizeof(GLuint) * nInds;

	stringstream sstr;
	sstr.setf(ios::fixed, ios::floatfield);
	sstr.precision(2);
	sstr << "Furthest point:\t\t\t" << -ffar*quad_size << "m\n";
	sstr << "Vertex count:\t\t\t" << nVerts << "\n";
	sstr << "Index count:\t\t\t" << nInds << "\n";
	sstr << "Clipmap Memory:\t\t\t" << bytes / 1024.0f / 1024.0f << "MiB\n";
	m_clipmap_stats += sstr.str();

	// OpenGL STUFF
	/////////////////////////////////////

	// Create the vertex array
	glGenVertexArrays(2, m_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(6, m_vbo);

	// OUTER LEVELS
	setup_vao(vertices, texcoords, indices, m_vao[0], m_vbo);

	// INNER GRID
	setup_vao(vertices_inner, texcoords_inner, indices_inner, m_vao[1], m_vbo+3);
	m_nInnerIndices = indices_inner.size();
	return true;
}

//--------------------------------------------------------
void 
Clipmap::setup_vao(std::vector<vector2>& verts, std::vector<vector2>& tcoords,  std::vector<GLuint>& indices, GLuint& vao, GLuint* vbo){
	int size;

	glBindVertexArray(vao);

	// Setup the vertex buffer
	size = sizeof(verts[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, size * verts.size(), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	size = sizeof(tcoords[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, size * tcoords.size(), &tcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	size = sizeof(indices[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * indices.size(), &indices[0], GL_STATIC_DRAW);
}



//--------------------------------------------------------
void 
Clipmap::cull(matrix4& mvp, vector2 shift)
{
	// initialise primcount to 1 for base indices
	m_primcount = 1;
	for (int i = 0; i < (int)blocks.size(); i++)
	{
		cull_block& block = blocks[i];

		if (!m_cullingEnabled)
		{
			m_draw_count[m_primcount] 	= blocks[i].count;
			m_draw_starts[m_primcount]	= blocks[i].start_index;
			m_primcount++;
		}
		else
		{
			for (int j = 0; j < 4; j++)
			{
				vector2& v = block.bound[j];
				vector4 frag = mvp * vector4(v.x + shift.x, -20.0f, v.y + shift.y, 1.0f);
				vector4 NDC  = frag * (1.0f / frag.w);

				// if screen x is neither > 1.0 nor < -1.0
				// remember v.y is the z coordinate, the frag's x and y would still need to be divided
				// by the w-coordinate (which is w=-z for perspective) so just mult both sides
				if (!(NDC.z < -1.0f || NDC.z > 1.0f || NDC.x < -1.2f || NDC.x > 1.2f))
				{
					m_draw_count[m_primcount] 	= blocks[i].count;
					m_draw_starts[m_primcount]	= blocks[i].start_index;
					m_primcount++;
					break;
				}
			}
		}
	}
}

//--------------------------------------------------------
void
Clipmap::render_levels(GLuint sh)
{
	float shiftX, left;
	float shiftZ, far;
	float scale;
	left = -m_quad_size * ((m_N-1)+ (m_M - 1)*2)*.5f;
	far  = -m_quad_size * ((m_N-1)+ (m_M - 1)*2)*.5f;
	scale  = 1;

	glBindVertexArray(m_vao[0]);
	for (int p = 0; p < m_nLevels; p++){
		scale *= 2.0f;

		if (p>0){
			// shift left and far
			left -= m_quad_size * (m_M-1 + (m_M-1)*.5f)*.5f*scale;
			far  -= m_quad_size * (m_M-1 + (m_M-1)*.5f)*.5f*scale;
		}
		// on odd levels put L on negative X,Z
		if (p & 0x1){ 
			left -= m_quad_size * scale;
			far  -= m_quad_size * scale;
		}

		shiftX = left;
		shiftZ = far;

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * (m_M-1) * scale;	// skip the block

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * (m_M-1 + 2) * scale; // skip the block and fixup

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * (m_M-1) * scale; // skip the block

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		// NEXT ROW
		shiftZ += m_quad_size * (m_M-1) * scale;
		shiftX = left;

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * ((m_M-1) * 3 + 2) * scale;	// skip the block

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		// NEXT ROW
		shiftZ += m_quad_size * (m_M-1 + 2) * scale; 			// skip block + fixup
		shiftX = left;

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * ((m_M-1) * 3 + 2) * scale;	// skip the block

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		// NEXT ROW
		shiftZ += m_quad_size * (m_M-1) * scale; 			// skip block
		shiftX = left;

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * (m_M-1) * scale;	// skip the block

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * (m_M-1 + 2) * scale; // skip the block and fixup

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);

		shiftX += m_quad_size * (m_M-1) * scale; // skip the block

		glUniform3f(glGetUniformLocation(sh, "st"), shiftX, shiftZ, scale);
		glDrawElements(GL_TRIANGLES, m_min_draw_count, GL_UNSIGNED_INT, 0);
	}
}

//--------------------------------------------------------
void
Clipmap::render_inner(){
	glBindVertexArray(m_vao[1]);
	glDrawElements(GL_TRIANGLES, m_nInnerIndices, GL_UNSIGNED_INT, 0);
}
