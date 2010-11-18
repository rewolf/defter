#ifndef _FIGHTER_JET_H
#define _FIGHTER_JET_H

enum JetState {	INACTIVE, ENTERING, TURNIN, APPROACHING};

//--------------------------------------------------------
class FighterJet : public GameEntity{
public:
	FighterJet				(Node* pModel, list<GameEntity*>* pBombs, ModelManager* pModelMan);
	virtual ~FighterJet		();

	void			CarpetBomb			(vector2 camera, vector2 target);
	void			Update				(void);
	bool			IsBusy				(void);

public:
	JetState		m_state;
	list<GameEntity*>*	m_pBombs;
	ModelManager*	m_pModelManager;
	int				m_nDropped;
	vector2			m_target;

private:
	vector2			corner;
	vector2			dir;
	vector2			dir2;
	vector2			start;
	vector2			circleCentre;
};

#endif
