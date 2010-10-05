

#ifndef _CACHING_H_
#define _CACHING_H_

struct	Tile {
	GLuint	m_TexID;
	bool	m_LoadedPrevious;
	bool	m_LoadedCurrent;
};

class Caching{
public:
	Caching					(int coarseDim, float clipRes, int highDim, float highRes);
	~Caching				(void);
	void Update				(vector2 worldPos);

private:
	void UpdateLoadStatus	(bool newStatus, int region);

	int		m_SizeOfGrid;
	Tile*	m_Grid;
	float	m_TileSize;
	float	m_BandWidth;
	float	m_BandPercent;
	float	m_CoarseOffset;
	int		m_RegionPrevious;
	int		m_RegionCurrent;
};

#endif
