

#ifndef _CACHING_H_
#define _CACHING_H_

struct	Tile {
	TexData	m_texdata;
	bool	m_modified;
	bool	m_LoadedPrevious;
	bool	m_LoadedCurrent;
	int		m_texID;
	int		m_row;
	int		m_col;
};

enum REQUEST_TYPE	{ LOAD, UNLOAD};
struct CacheRequest{
	REQUEST_TYPE	type;
	GLuint			pbo;
	GLubyte*		ptr;
	Tile*			tile;
};
	
// function for caching thread
int hdd_cacher(void* data);

#define PBO_POOL		(5)

class Caching{
public:
	Caching					(Deform* pDeform, int clipDim, int coarseDim, float clipRes, int highDim, float highRes);
	~Caching				(void);
	void Update				(vector2 worldPos);
	void DeformHighDetail	(TexData coarseMap, vector2 clickPos, float scale);

	friend int hdd_cacher 	(void* data);

	string					m_caching_stats;

private:
	void UpdateTiles		(bool newStatus, int region, vector2 TileIndex);
	void SetLoadStatus		(bool newStatus, vector2 TileIndex, vector2 size);
	void SetActiveStatus	(bool newStatus, vector2 TileIndex, vector2 size);
	void DrawRadar			(void);

	void Load				(Tile* tile);
	void Unload				(Tile* tile);
	bool LoadTextureData	(Tile* tile, GLubyte* data);
	bool SaveTextureData	(Tile* tile, GLubyte* data);
	void UpdatePBOs			();

	Deform*				m_pDeform;
	int					m_coarseDim;
	int					m_highDim;
	int					m_GridSize;
	Tile*				m_Grid;
	float				m_TileSize;
	float				m_BandWidth;
	float				m_BandPercent;
	float				m_CoarseOffset;
	int					m_RegionCurrent;
	int					m_RegionPrevious;
	vector2				m_TileIndexCurrent;
	vector2				m_TileIndexPrevious;

	// threading
	SDL_Thread*			m_cacheThread;
	SDL_mutex*			m_doneQueueMutex;
	SDL_mutex*			m_loadQueueMutex;
	SDL_mutex*			m_unloadQueueMutex;
	list<CacheRequest>	m_readyQueue;
	list<CacheRequest>	m_doneQueue;
	list<CacheRequest>	m_loadQueue;
	list<CacheRequest>	m_unloadQueue;
	queue<GLuint>		m_pboPackPool;
	queue<GLuint>		m_pboUnpackPool;
	GLuint				m_pbos[PBO_POOL*2];
	bool				m_threadRunning;
};


#endif
