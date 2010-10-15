
#ifndef _DEFORM_H_
#define _DEFORM_H_

struct  Stamp {
	bool			m_isTexStamp;
	GLuint			m_texture;
	ShaderProg*		m_shader;

	void(*initShader) (Stamp stamp, vector2 clickPos, float scale, float intensity);

	Stamp()
	{
		m_isTexStamp	= false;
		m_texture		= 0;
		m_shader		= NULL;

		initShader		= NULL;
	}

	bool SetupShader(string vertPath, string fragPath)
	{
		m_isTexStamp = false;

		m_shader = new ShaderProg(vertPath, "", fragPath);
		glBindAttribLocation(m_shader->m_programID, 0, "vert_Position");
		glBindAttribLocation(m_shader->m_programID, 0, "vert_TexCoord");

		return (m_shader->CompileAndLink());
	}

	bool LoadTexture(string textureName)
	{
		m_isTexStamp = true;
		return (LoadPNG(&m_texture, textureName));
	}
};

class Deform{
public:
	Deform					(int coarseDim, int highDim, float metre_to_tex, float metre_to_detail_tex);
	~Deform();

	void displace_heightmap	(TexData texdata, vector2 clickPos, string stampName, float scale,
							float intensity, bool isCoarse, GLuint copySrcTex=0);
	void create_pdmap		(TexData texdata, bool isCoarse);

	bool			m_no_error;

private:
	void calculate_pdmap	(TexData texdata, vector2 clickPos, float scale, bool isCoarse);
	
	ShaderProg*		m_shTexStamp;
	ShaderProg*		m_shPDMapper;

	// FBOs
	GLuint			m_fbo_heightmap;

	// VAOs & VBOs
	GLuint			m_vao;
	GLuint			m_vbo;

	int				m_coarseDim;
	int				m_highDim;
	float			m_metre_to_tex;
	float			m_metre_to_detail_tex;
	GLuint			m_coarseBackup;
	bool			m_initialised;

	map<string, Stamp> stampCollection;
};

// Functional Stamp setup callbacks
void setupGaussian(Stamp stamp, vector2 clickPos, float scale, float intensity);

#endif
