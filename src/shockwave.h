
#ifndef _SHOCKWAVE_H_
#define _SHOCKWAVE_H_

enum State {ACTIVE, IDLE};

class Shockwave{
public:
	Shockwave	(TexData coarsemap, int dimension);
	~Shockwave	(void);


	void		update			(float dt);
	void		create			(vector3 position);

public:
	ShaderProg* m_shWave;
	GLuint		m_fbo;
	GLuint		m_vao;
	GLuint		m_vbo;
	GLuint		m_stampTex;
	TexData		m_coarsemap;
	State		m_state;
	bool		m_no_error;
	int			m_dimension;
	float		m_radius;
	float		m_height;
};


#endif
