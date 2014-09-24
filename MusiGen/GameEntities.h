#pragma once

#include "Vector4.h"

class GameEntity
{
public:
	enum EntityTypes {PLAYER_SHIP, PLAYER_BULLET, ENEMY_SHIP_01, ENEMY_SHIP_02, ENEMY_SHIP_03, ENEMY_BULLET_01};

	GameEntity(EntityTypes Type);
	EntityTypes GetType(void);
	float GetRadius(void);
	
	Vector4 m_vPos,m_vVel;
	unsigned int m_uiEnergy;

protected:
	
	EntityTypes m_eEntityType;
	float m_fRadius;
};