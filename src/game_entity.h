#ifndef _GAME_ENTITY_H_
#define _GAME_ENTITY_H_


class GameEntity {
public:
	GameEntity		(Node* pModel);
	~GameEntity		();

	void		SetTranslate		(vector3 position);
	void		ZeroVelocity		();
	void		SetVelocity			(vector3 velocity);
	void		AddVelocity			(vector3 velocity);
	vector3		GetVelocity			();
	vector2		GetHorizPosition	();

	void		Update				();

public:
	Node*		m_pModel;
	vector3		m_translate;
	vector3		m_lastTranslate;
	vector3		m_rotate;
	vector3		m_scale;
	vector3		m_frameAcceleration;
	bool		m_onGround;
};

#endif
