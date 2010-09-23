
#ifndef _SKYBOX_H_
#define _SKYBOX_H_


class Skybox{
public:
	Skybox();
	~Skybox();

	void		render			(matrix4& transform);


public:
	ShaderProg*		m_shSky;
	bool			m_no_error;

	GLuint			m_tex;
	GLuint			m_vao;
	GLuint			m_vbo[3];
};



#endif
