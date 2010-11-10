#ifndef _GAME_ENTITY_H_
#define _GAME_ENTITY_H_


class GameEntity{
public:
	GameEntity();
	~GameEntity();

public:
	Node*		m_model;
	vector3		m_translate;
	vector3		m_rotate;
	vector3		m_scale;
};

#endif