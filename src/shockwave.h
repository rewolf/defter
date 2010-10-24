
#ifndef _SHOCKWAVE_H_
#define _SHOCKWAVE_H_

enum State {INIT, SECOND, ACTIVE, IDLE};

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
	GLuint		m_nextTex;
	GLuint		m_currentTex;
	GLuint		m_previousTex;
	GLuint		m_initialTex;
	TexData		m_coarsemap;
	State		m_state;
	bool		m_no_error;
	int			m_dimension;

};


#endif
