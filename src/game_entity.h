#ifndef _GAME_ENTITY_H_
#define _GAME_ENTITY_H_


class GameEntity{
public:
	GameEntity		(Node* pModel);
	~GameEntity		();

public:
	Node*		m_pModel;
	vector3		m_translate;
	vector3		m_rotate;
	vector3		m_scale;
};

#endif