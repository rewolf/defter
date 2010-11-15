#ifndef _FIGHTER_JET_H
#define _FIGHTER_JET_H

enum JetState {	INACTIVE, ENTERING, TURNIN	};

//--------------------------------------------------------
class FighterJet : public GameEntity{
public:
	FighterJet				(Node* pModel);
	virtual ~FighterJet		();

	void			CarpetBomb			(vector2 camera, vector2 target);
	void			Update				(void);
	bool			IsBusy				(void);

public:
	JetState		m_state;
};

#endif
