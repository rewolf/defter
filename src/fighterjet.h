#ifndef _FIGHTER_JET_H
#define _FIGHTER_JET_H

enum JetState {	INACTIVE, ENTERING, TURNIN	};

//--------------------------------------------------------
class FighterJet : public GameEntity{
public:
	FighterJet				(Node* pModel);
	virtual ~FighterJet		();

	void			Update				();

public:
	JetState		m_state;
};

#endif