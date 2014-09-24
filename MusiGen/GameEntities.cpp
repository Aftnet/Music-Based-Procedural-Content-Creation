#include "GameEntities.h"

GameEntity::GameEntity(GameEntity::EntityTypes Type):
m_eEntityType(Type),
m_vPos(Vector4(0.0f,0.0f,0.0f,1.0f)),
m_vVel(Vector4(0.0f,0.0f,0.0f,1.0f))
{
	switch(m_eEntityType)
	{
	case PLAYER_SHIP:
		m_fRadius = 1.0f;
		break;
	case PLAYER_BULLET:
		m_fRadius = 8.0f;
		break;
	case ENEMY_SHIP_01:
		m_fRadius = 25.0f;
		m_uiEnergy = 2;
		break;
	case ENEMY_SHIP_02:
		m_fRadius = 25.0f;
		m_uiEnergy = 2;
		break;
	case ENEMY_SHIP_03:
		m_fRadius = 25.0f;
		m_uiEnergy = 2;
		break;
	case ENEMY_BULLET_01:
		m_fRadius = 8.0f;
		m_uiEnergy = 0;
		break;
	}
	
}

GameEntity::EntityTypes GameEntity::GetType(void)
{
	return m_eEntityType;
}

float GameEntity::GetRadius(void)
{
	return m_fRadius;
}