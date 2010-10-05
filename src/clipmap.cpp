

#include "regl3.h"
#include "re_math.h"
#include <vector>
using namespace reMath;
#include "re_shader.h"
#include "util.h"
#include "deform.h"
#include "clipmap.h"

#ifdef _WIN32
	float log2(float n){
		return logf(n)/logf(2);
	};
#endif

//--------------------------------------------------------
Clipmap::Clipmap(int nVerts, float quad_size, int nLevels, int heightmap_dim){
	if ( ((nVerts + 1) & nVerts ) != 0){
		int pot = 1<<int(ceil(log2(nVerts+1)));
		printf("\tWARNING: nVerts must be an integer  2^k - 1, Rounding up %d -> %d\n", nVerts, pot-1);
		nVerts = pot-1;
	}

	m_enabled		= true;
	m_N				= nVerts;
	m_nLevels		= nLevels;
	m_quad_size		= quad_size;
	m_heightmap_dim	= heightmap_dim;
	m_min_draw_count	= 0;

	//////////////////////////////////////////////////
	// Just some scaling settings
	// 1 unit = 1 metre
	// NB The finest clip level quads should match texel resolution
	// => texelsize = 1.0/texture_dimension
	// The size of the finest quads will determine the physical area a heightmap will represent
	
	m_texel_size	= 1.0f/(heightmap_dim);
	m_M				= (m_N + 1) / 4;	// block size

	m_metre_to_tex 	= m_texel_size/quad_size;
	m_tex_to_metre 	= 1.0f/m_metre_to_tex;

	// Output settings to stdout
	printf("\tClipmap levels:\t\t\t%d\n", 		m_nLevels);
	printf("\tFinest quad size:\t\t%.3fm\n", 		m_quad_size);
	printf("\tVertices per ring-side:\t\t%d\n",	m_N);
	printf("\tSample distance:\t\t%.3f\n",		m_texel_size);
	printf("\tEffective heightmap size:\t%.2f\n", heightmap_dim * quad_size);

}


//--------------------------------------------------------
Clipmap::~Clipmap(){
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(3, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
}

//--------------------------------------------------------
bool
Clipmap::init(){
	int i,j;
	float quad_size, texel_size, left, ffar;
	int vmarker, vcount;
	std::vector <VERTEX>	vertices;
	std::vector <TEXCOORD>	texcoords;
	std::vector <INDEX>		indices;
	std::vector <INDEX>		cullable;

	quad_size  = m_quad_size;
	texel_size = m_texel_size;

	vcount = 0;  // The index of the next vertex (ie the number of vertices so far)

	// Create the inner grid
	left = - (m_N-1) * .5f;
	ffar  = - (m_N-1) * .5f;
	for (i = 0; i < m_N;i++){
		for (j = 0; j < m_N; j++){
			VERTEX v;
			TEXCOORD tc;

			v.x = left*quad_size + j*quad_size;
			v.y = ffar*quad_size + i*quad_size;
			tc.u= 0.5f + left*texel_size + j*texel_size;
			tc.v= 0.5f + ffar*texel_size + i*texel_size;;

			vertices.push_back(v);
			texcoords.push_back(tc);
			if (i>0 && j>0){
				indices.push_back( vcount-1 );
				indices.push_back( vcount-m_N );
				indices.push_back( vcount-m_N-1 );

				indices.push_back( vcount-m_N );
				indices.push_back( vcount-1 );
				indices.push_back( vcount );
			}
			vcount++;
		}
	}

	// Create the LOD regions
	//////////////////////////////////
	for (i=0; i < m_nLevels; i++){
		quad_size*=2;
		texel_size*=2;

		// Create degnerate triangles around the centre top
		for (int j = 0; j < (m_N-1)/2+1; j++){
			VERTEX v;
			TEXCOORD tc;

			v.x  = left*quad_size/2 + j * quad_size;
			v.y  = ffar*quad_size/2;
			tc.u = 0.5f + left*texel_size/2 + j*texel_size;
			tc.v = 0.5f + ffar*texel_size/2;

			if (j > 0){
				VERTEX vm = v;
				vm.x -= 0.5f*quad_size;
	//			vm.y += 0.25f*quad_size;	// delete this line
				TEXCOORD tcm = tc;
				tcm.u-= 0.5f*texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back( vcount );
				indices.push_back( vcount + 1);
				indices.push_back( vcount - 1);
				vcount ++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Create degnerate triangles around the centre bottom
		for (int j = 0; j < (m_N-1)/2+1; j++){
			VERTEX v;
			TEXCOORD tc;

			v.x  = left*quad_size/2 + j * quad_size;
			v.y  = ffar*quad_size/2 + (m_N-1) * quad_size/2;
			tc.u = 0.5f + left*texel_size/2 + j*texel_size;
			tc.v = 0.5f + ffar*texel_size/2 + (m_N-1)*texel_size/2;

			if (j > 0){
				VERTEX vm = v;
				vm.x -= 0.5f*quad_size;
	//			vm.y -= 0.25f*quad_size;	// delete this line
				TEXCOORD tcm = tc;
				tcm.u-= 0.5f*texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back( vcount );
				indices.push_back( vcount - 1);
				indices.push_back( vcount + 1);
				vcount ++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Create degnerate triangles around the centre left
		for (int j = 0; j < (m_N-1)/2+1; j++){
			VERTEX v;
			TEXCOORD tc;

			v.x  = left*quad_size/2;
			v.y  = ffar*quad_size/2 + j * quad_size;
			tc.u = 0.5f + left*texel_size/2;
			tc.v = 0.5f + ffar*texel_size/2 + j * texel_size;

			if (j > 0){
				VERTEX vm = v;
				vm.y -= 0.5f*quad_size;
	//			vm.x += 0.25f*quad_size;	// delete this line
				TEXCOORD tcm = tc;
				tcm.v-= 0.5f*texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back( vcount );
				indices.push_back( vcount - 1);
				indices.push_back( vcount + 1);
				vcount ++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Create degnerate triangles around the centre right
		for (int j = 0; j < (m_N-1)/2+1; j++){
			VERTEX v;
			TEXCOORD tc;

			v.x  = left*quad_size/2 + (m_N-1)*quad_size/2;
			v.y  = ffar*quad_size/2 + j * quad_size;
			tc.u = 0.5f + left*texel_size/2 + (m_N-1)*texel_size/2;
			tc.v = 0.5f + ffar*texel_size/2 + j * texel_size;

			if (j > 0){
				VERTEX vm = v;
				vm.y -= 0.5f*quad_size;
	//			vm.x -= 0.25f*quad_size;	// delete this line
				TEXCOORD tcm = tc;
				tcm.v-= 0.5f*texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back( vcount );
				indices.push_back( vcount + 1);
				indices.push_back( vcount - 1);
				vcount ++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Construct the L shape top/bottom strip
		for (int j = 0; j < (m_N-1)/2+1; j++){
			VERTEX v1,v2;
			TEXCOORD tc1,tc2;

			v1.x = left*quad_size/2 + j*quad_size;;
			v1.y = ffar*quad_size/2 - quad_size + (i%2==1? (m_N+1)*quad_size/2 : 0);
			v2 = v1;
			v2.y += quad_size;
			tc1.u = 0.5f + left*texel_size/2 + j*texel_size;
			tc1.v = 0.5f + ffar*texel_size/2 - texel_size + (i%2==1? (m_N+1)*texel_size/2 : 0);
			tc2 = tc1;
			tc2.v += texel_size;

			if (j>0){
				indices.push_back( vcount - 1);
				indices.push_back( vcount );
				indices.push_back( vcount - 2);
				indices.push_back( vcount - 1);
				indices.push_back( vcount + 1);
				indices.push_back( vcount );
			}
			vertices.push_back(v1);
			vertices.push_back(v2);
			texcoords.push_back(tc1);
			texcoords.push_back(tc2);
			vcount+=2;
		}
		// Construct the L shape left/right strip
		for (int j = 0; j < (m_N-1)/2+2; j++){
			VERTEX v1,v2;
			TEXCOORD tc1,tc2;

			v1.x = left*quad_size/2 - quad_size + (i%2==0? (m_N+1)*quad_size/2 : 0);
			v1.y = ffar*quad_size/2 + j*quad_size - (i%2==0? quad_size: 0);
			v2 = v1;
			v2.x += quad_size;
			tc1.u = 0.5f + left*texel_size/2 - texel_size + (i%2==0? (m_N+1)*texel_size/2 : 0);
			tc1.v = 0.5f + ffar*texel_size/2 + j*texel_size - (i%2==0? texel_size: 0);
			tc2 = tc1;
			tc2.u += texel_size;

			if (j>0){
				indices.push_back( vcount );
				indices.push_back( vcount - 1);
				indices.push_back( vcount - 2);
				indices.push_back( vcount );
				indices.push_back( vcount + 1);
				indices.push_back( vcount - 1);
			}
			vertices.push_back(v1);
			vertices.push_back(v2);
			texcoords.push_back(tc1);
			texcoords.push_back(tc2);
			vcount+=2;
		}


		// Now we're gonna work on the left of this level, so update these variables
		left = left*.5f - (m_M - 1 + i%2);
		ffar  = ffar*.5f - (m_M - 1 + (i+1)%2);

		// Create left column of blocks
		vmarker = vcount;
		for (int j = 0; j < m_N; j++){
			for (int k = 0; k < m_M; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ k * quad_size;
				v.y		= ffar * quad_size 	+ j * quad_size;
				tc.u	= 0.5f + left * texel_size + k * texel_size;
				tc.v	= 0.5f + ffar * texel_size + j * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}		
		create_block(vmarker + m_M * (m_M-1) * 0,   m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M-1) * 1,   m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M-1) * 2,   m_M,   3, vertices, cullable); // fixup
		create_block(vmarker + m_M * (m_M-1) * 2+2*m_M, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M-1) * 3+2*m_M, m_M, m_M, vertices, cullable);

		// Create right column of blocks
		vmarker = vcount;
		for (int j = 0; j < m_N; j++){
			for (int k = 0; k < m_M; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + m_N - m_M) * quad_size;
				v.y		= ffar * quad_size 	+ j * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + m_N - m_M) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + j * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}		
		create_block(vmarker + m_M * (m_M-1) * 0,   m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M-1) * 1,   m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M-1) * 2,   m_M,   3, vertices, cullable); // fixup
		create_block(vmarker + m_M * (m_M-1) * 2+2*m_M, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M-1) * 3+2*m_M, m_M, m_M, vertices, cullable);

		// Create top row of blocks
		vmarker = vcount;
		for (int j = 0; j < m_M; j++){
			for (int k = 0; k < m_M; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + m_M - 1) * quad_size;
				v.y		= ffar * quad_size 	+ j * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + m_M - 1) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + j * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);
		vmarker = vcount;		
		for (int j = 0; j < m_M; j++){
			for (int k = 0; k < 3; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + 2*m_M - 2) * quad_size;
				v.y		= ffar * quad_size 	+ j * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + 2*m_M - 2) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + j * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}		
		create_block(vmarker, 3, m_M, vertices, cullable);
		vmarker = vcount;
		for (int j = 0; j < m_M; j++){
			for (int k = 0; k < m_M; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + 2*m_M ) * quad_size;
				v.y		= ffar * quad_size 	+ j * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + 2*m_M) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + j * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);

		// Create bottom row of blocks
		vmarker = vcount;
		for (int j = 0; j < m_M; j++){
			for (int k = 0; k < m_M; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + m_M - 1) * quad_size;
				v.y		= ffar * quad_size 	+ (j + m_N - m_M) * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + m_M - 1) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + (j + m_N - m_M) * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);
		vmarker = vcount;		
		for (int j = 0; j < m_M; j++){
			for (int k = 0; k < 3; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + 2*m_M - 2) * quad_size;
				v.y		= ffar * quad_size 	+ (j + m_N - m_M) * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + 2*m_M - 2) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + (j + m_N - m_M) * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}		
		create_block(vmarker, 3, m_M, vertices, cullable);
		vmarker = vcount;
		for (int j = 0; j < m_M; j++){
			for (int k = 0; k < m_M; k++){
				VERTEX v;
				TEXCOORD tc;

				v.x 	= left * quad_size 	+ (k + 2*m_M ) * quad_size;
				v.y		= ffar * quad_size 	+ (j + m_N - m_M) * quad_size;
				tc.u	= 0.5f + left * texel_size + (k + 2*m_M) * texel_size;
				tc.v	= 0.5f + ffar * texel_size + (j + m_N - m_M) * texel_size;
				
				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);
	}

	m_min_draw_count = indices.size();
	// Add this offset to all blocks' start indices
	for (int i = 0; i < blocks.size(); i++)
		blocks[i].start_index += m_min_draw_count * sizeof(INDEX);
	// Append the cullable block indices to the base indices
	indices.insert(indices.end(), cullable.begin(), cullable.end());

	// Add the base indices to the draw list
	m_draw_count[0]	 = m_min_draw_count;
	m_draw_starts[0] = 0;

	printf("\tFurthest point:\t\t\t%.2f\n",	-ffar*quad_size);
	printf("\tVertex count:\t\t\t%d\n",(int)vertices.size());
	printf("\tIndex count:\t\t\t%d\n",(int)indices.size());
	printf("\tPrimitive count:\t\t%d\n",(int)indices.size()/3);
	printf("\tClipmap Memory:\t\t\t%.2fMiB\n", (texcoords.size()*2 + vertices.size()*3 + indices.size())*4 /1024.0f/1024.0f);

	// OpenGL STUFF
	/////////////////////////////////////

	// Create the vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(3, m_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TEXCOORD) * vertices.size(), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDEX) * indices.size(), &indices[0], GL_STATIC_DRAW);
	return true;
}

//--------------------------------------------------------
void
Clipmap::create_block(int vertstart, int width, int height, 
					  std::vector<VERTEX> &vertices, std::vector<INDEX> &indices){
	cull_block block;
	int idx;

	idx 				= indices.size();	
	block.start_index	= indices.size() * sizeof(INDEX);	// offset into indexbuffer
	block.count			= (width-1)*(height-1) * 6; 		// number of indices

	// Form the triangles and push them onto the index list
	for (int y = 0; y < height-1; y++){
		for (int x = 0; x < width-1; x++){
			indices.push_back((y + 0) * width + (x + 0)  + vertstart);
			indices.push_back((y + 1) * width + (x + 0)  + vertstart);
			indices.push_back((y + 0) * width + (x + 1)  + vertstart);

			indices.push_back((y + 0) * width + (x + 1)  + vertstart);
			indices.push_back((y + 1) * width + (x + 0)  + vertstart);
			indices.push_back((y + 1) * width + (x + 1)  + vertstart);
		}
	}

	block.bound[0] = vertices[indices[idx]];
	block.bound[1] = vertices[indices[idx + (width-1)*6 - 3]];
	block.bound[2] = vertices[indices[idx + block.count + 1 - (width-1)*6]];
	block.bound[3] = vertices[indices[idx + block.count-1]];


	blocks.push_back(block);
}

//--------------------------------------------------------
void 
Clipmap::cull(matrix4& mvp, vector2 shift){
	// initialise primcount to 1 for base indices
	m_primcount = 1;
	for (int i = 0; i < blocks.size(); i++){
		cull_block& block = blocks[i];

		if (!m_enabled){
			m_draw_count[m_primcount] 	= blocks[i].count;
			m_draw_starts[m_primcount]	= blocks[i].start_index;
			m_primcount++;
		}
		else{
			for (int j = 0; j < 4; j++){
				VERTEX& v = block.bound[j];
				vector4 frag = mvp * vector4(v.x+shift.x, -20.0f, v.y+shift.y, 1.0f);
				vector4 NDC  = frag * (1.0f / frag.w);

				// if screen x is neither > 1.0 nor < -1.0
				// remember v.y is the z coordinate, the frag's x and y would still need to be divided
				// by the w-coordinate (which is w=-z for perspective) so just mult both sides
				if (!(NDC.z < -1.0f || NDC.z > 1.0f 
						|| NDC.x < -1.2f || NDC.x > 1.2f)){
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
Clipmap::render(){
	glBindVertexArray(m_vao);
	for (int i = 0; i < m_primcount; i++)
		glDrawElements(GL_TRIANGLES, m_draw_count[i], GL_UNSIGNED_INT, (GLvoid*)(m_draw_starts[i]));

	//glMultiDrawElements(GL_TRIANGLES, ( GLsizei*)(m_draw_count), 
	//	GL_UNSIGNED_INT, (const GLvoid**)(m_draw_starts), m_primcount);
}

