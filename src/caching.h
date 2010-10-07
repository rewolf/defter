

#ifndef _CACHING_H_
#define _CACHING_H_

struct	Tile {
	TexData	m_texdata;
	bool	m_LoadedPrevious;
	bool	m_LoadedCurrent;
	int		m_texID;
};

class Caching{
public:
	Caching					(Deform* pDeform, int clipDim, int coarseDim, float clipRes, int highDim, float highRes);
	~Caching				(void);
	void Update				(vector2 worldPos);

	string					m_caching_stats;

private:
	void UpdateLoadStatus	(bool newStatus, int region, vector2 TileIndex);
	void DrawRadar			(void);

	SDL_Thread*		m_loadThread;

	Deform*			m_pDeform;
	int				m_GridSize;
	Tile*			m_Grid;
	float			m_TileSize;
	float			m_BandWidth;
	float			m_BandPercent;
	float			m_CoarseOffset;
	int				m_RegionCurrent;
	int				m_RegionPrevious;
	vector2			m_TileIndexCurrent;
	vector2			m_TileIndexPrevious;
};

#endif
