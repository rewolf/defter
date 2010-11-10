

#include "constants.h"
using namespace std;
#include "game_entity.h"


//--------------------------------------------------------
GameEntity::GameEntity(Node* pModel){
	m_scale		= vector3(1.0f);
	m_pModel	= pModel;
	m_onGround	= false;
}

//--------------------------------------------------------
GameEntity::~GameEntity(){
	delete m_pModel;
}

//--------------------------------------------------------
// Changes the translate and sets zero velocity
void
GameEntity::SetTranslate(vector3 position){
	m_translate = position;
	m_lastTranslate = m_translate;
}

//--------------------------------------------------------
void
GameEntity::ZeroVelocity(){
	SetTranslate(m_translate);
}

//--------------------------------------------------------
void
GameEntity::SetVelocity(vector3 velocity){
	m_lastTranslate = m_translate - velocity * DT;
}

//--------------------------------------------------------
vector3
GameEntity::GetVelocity(){
	return invDT * (m_translate - m_lastTranslate);// (f(t)-f(t-1))/(dt)
}

//--------------------------------------------------------
void
GameEntity::AddVelocity(vector3 velocity){
	m_lastTranslate += - velocity * DT;
}

//--------------------------------------------------------
vector2
GameEntity::GetHorizPosition(){
	return vector2(m_translate.x, m_translate.z);
}

//--------------------------------------------------------
void
GameEntity::Update(){
	vector3 accel 	= m_frameAcceleration;
	vector3 velocity= GetVelocity(); 
	
	if (m_onGround)
		accel += - FRICTION * vector3(velocity.x, .0f, velocity.z);
	else
		accel += - AIR_DRAG * velocity;

	vector3 temp 	= m_translate;
	m_translate		= 2*m_translate - m_lastTranslate + accel * DT * DT;	// verlet update

	m_lastTranslate	= temp;
}
