/*****************************************************************************
 * Header: skybox
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _SKYBOX_H_
#define _SKYBOX_H_


class Skybox
{
public:
	Skybox();
	~Skybox();

	bool HasError		(void);
	void render			(matrix4& transform);


private:
	ShaderProg*		m_shSky;
	bool			m_error;

	GLuint			m_tex;
	GLuint			m_vao;
	GLuint			m_vbo[3];
};

#endif
