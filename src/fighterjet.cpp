

#include "constants.h"
using namespace std;
#include "game_entity.h"
#include "model_manager.h"
#include "fighterjet.h"

const float ENTER_DIST		= 400.0f;
const float	SPEED			= 200.0f;
const float ANG_SPEED		= SPEED/(.5f*ENTER_DIST);
const int	BOMB_COUNT		= 2;

//--------------------------------------------------------
FighterJet::FighterJet(Node* pModel, list<GameEntity*>* pBombs,
					   ModelManager* pModelMan) : GameEntity(pModel){
	m_state		= INACTIVE;
	m_pBombs	= pBombs;
	m_nDropped	= 0;
	m_pModelManager = pModelMan;
}

//--------------------------------------------------------
FighterJet::~FighterJet(){
}

//--------------------------------------------------------
void
FighterJet::Update(){
	float angle;
	vector2 circleR;
	vector2 temp;
	vector3 direction	= GetVelocity().GetUnit();
	float distCheck;

	switch(m_state){
		case INACTIVE:
			return;
		case ENTERING:
			m_lastTranslate = m_translate;
			m_translate.x += -dir2.x * SPEED * DT;
			m_translate.z += -dir2.y * SPEED * DT;

			circleR		= GetHorizPosition() - circleCentre;
			distCheck	= circleR.Dot(dir2);
			
			if (distCheck < 0){
				m_state		= TURNIN;
				temp = circleCentre + circleR.GetUnit() * ENTER_DIST * .5f;
				m_translate.x = temp.x;
				m_translate.z = temp.y;
			}
			break;

		case TURNIN:
			m_lastTranslate	 = m_translate;
			m_translate.x	+= direction.x * SPEED * DT;
			m_translate.z	+= direction.z * SPEED * DT;
			circleR			 = GetHorizPosition() - circleCentre;
			// adjust onto circle
			temp = circleCentre + circleR.GetUnit() * ENTER_DIST * .5f;
			m_translate.x = temp.x;
			m_translate.z = temp.y;

			// calculate the roll based on direction
			//m_rotate.z = PI*.25f * sinf(PI*dir2.Dot(-vector2(direction.x,direction.z)));
			
			// Calculate if its completed the quarter circle yet
			// by using the other "dir"
			circleR		= GetHorizPosition() - circleCentre;
			distCheck	= circleR.Dot(dir);
			if (distCheck < 0){
				m_state		= APPROACHING;
			}
			break;

		case APPROACHING:
			m_lastTranslate	 = m_translate;
			m_translate.x	+= -dir.x * SPEED * DT;
			m_translate.z	+= -dir.y * SPEED * DT;

			circleR		= GetHorizPosition() - m_target;
			distCheck	= circleR.Dot(dir);
			if (distCheck < 25.0f && !m_nDropped || (distCheck < 5.0f && m_nDropped < BOMB_COUNT)){
				GameEntity* newBomb			= new GameEntity(m_pModelManager->GetModel("bomb"));
				newBomb->m_translate		= m_translate;
				newBomb->m_lastTranslate	= .15f*m_lastTranslate + .85f*m_translate;
				newBomb->m_rotate			= m_rotate;
				newBomb->m_translate.y	   -= 5.0f;
				newBomb->m_lastTranslate.y -= 5.0f;
				newBomb->m_scale			= vector3(2.0f);
				m_pBombs->push_back(newBomb);
				m_nDropped ++;
			}
			else if (distCheck < -1000.0f){
				m_state = INACTIVE;
			}
			break;

	}
	// base yaw on velocity
	m_rotate.y = atan2f(direction.x, direction.z) + PI;
}

//--------------------------------------------------------
bool
FighterJet::IsBusy(){
	return m_state!=INACTIVE;
}

//--------------------------------------------------------
void
FighterJet::CarpetBomb(vector2 camera, vector2 target){
	m_target = target;
	dir = target - camera;
	dir.Normalize();
	corner	= target + dir * ENTER_DIST;
	dir2	= vector2(dir.y, -dir.x).GetUnit();
	start	= corner + dir2 * ENTER_DIST * 2;
	m_translate.set(start.x, 70.0f, start.y);
	circleCentre = corner + .5f * ENTER_DIST * dir2 - .5f * ENTER_DIST * dir;

	m_state = ENTERING;
	m_nDropped = 0;
}