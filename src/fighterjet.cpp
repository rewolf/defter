

#include "constants.h"
using namespace std;
#include "game_entity.h"
#include "fighterjet.h"


//--------------------------------------------------------
FighterJet::FighterJet(Node* pModel) : GameEntity(pModel){
	m_state = INACTIVE;
}

//--------------------------------------------------------
FighterJet::~FighterJet(){
}

//--------------------------------------------------------
void
FighterJet::Update(){
	switch(m_state){
		case INACTIVE:
			return;
		case ENTERING:
			break;
		case TURNIN:
			break;
	}
}

//--------------------------------------------------------
bool
FighterJet::IsBusy(){
	return m_state!=INACTIVE;
}

//--------------------------------------------------------
void
FighterJet::CarpetBomb(vector2 camera, vector2 target){
	vector2 dir = target - camera;
	dir.Normalize();
	vector2 corner	= target + dir * 50.0f;
	vector2 dir2	= vector2(dir.x, -dir.y).GetUnit();
	vector2 start	= corner + dir2 * 100.0f;
	m_translate.set(start.x, 20.0f, start.y);
}