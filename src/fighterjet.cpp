

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