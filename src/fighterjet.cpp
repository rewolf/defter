

#include "constants.h"
using namespace std;
#include "game_entity.h"
#include "fighterjet.h"

const float ENTER_DIST		= 400.0f;
const float	APPROACH_DIST	= 200.0f;
const float	SPEED			= 100.0f;
const float ANG_SPEED		= SPEED/(.5f*ENTER_DIST);

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
	vector2 displacement;
	vector2 add;
	float dist;
	float angle;
	switch(m_state){
		case INACTIVE:
			return;
		case ENTERING:
			m_translate.x += -dir2.x * SPEED * DT;
			m_translate.z += -dir2.y * SPEED * DT;

			displacement = GetHorizPosition() - corner;
			dist = displacement.Mag();

			// at halfway point start turning in
			if (dist < ENTER_DIST * .5f){
				m_state = TURNIN;
				t		= asinf((ENTER_DIST*.5f - dist)/(.5f * ENTER_DIST));
				printf("%f\n", t);
			}
			else{
				break;
			}
		case TURNIN:
			angle = ANG_SPEED * t;
			add.x = .5f * ENTER_DIST * sinf(angle);
			add.y = .0f;
			printf("%f\n",add.x);

			m_translate.x = circleCentre.x + add.x;
			t+=DT;

			if (angle > .5f *PI)
				m_state = APPROACHING;
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
	this->target = target;
	dir = target - camera;
	dir.Normalize();
	corner	= target + dir * 200.0f;
	dir2	= vector2(dir.y, -dir.x).GetUnit();
	start	= corner + dir2 * 400.0f;
	m_translate.set(start.x, 30.0f, start.y);
	circleCentre = corner + .5f * ENTER_DIST * dir2 - .5f * ENTER_DIST * dir;

	m_state = ENTERING;
	t		= .0f;
}