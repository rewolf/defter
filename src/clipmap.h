
#ifndef _CLIPMAP_H_
#define _CLIPMAP_H_

// define new types for easy changing
typedef GLuint 	INDEX;
typedef vector2	VERTEX;
typedef float2	TEXCOORD;

struct cull_block{
	int		count;
	int		start_index;
	VERTEX	bound[4];
};


class Clipmap{
public:
	Clipmap (int nVerts, float quad_size, int levels, int heightmap_dim);
	~Clipmap ();

	bool		init			();
	void		cull			(matrix4& mvp, vector2 shift);
	void		render			();
private:
	void		create_block	(int vertstart, int width, int height, 
									std::vector<VERTEX> &vertices,
									std::vector<INDEX> &indices);

public:
	vector<cull_block> blocks;

	GLuint		m_vbo[3];
	GLuint		m_vao;

	int			m_min_draw_count;
	int			m_primcount;
	GLsizei		m_draw_count[128];
	GLuint		m_draw_starts[128];

	int			m_N;		// clipmap dim (verts)
	int			m_M;		// block size
	int			m_nLevels;
	int			m_heightmap_dim;

	float		m_quad_size;
	float		m_texel_size;
	float		m_tex_to_metre;
	float		m_metre_to_tex;

	bool 		m_enabled;
};


#endif
