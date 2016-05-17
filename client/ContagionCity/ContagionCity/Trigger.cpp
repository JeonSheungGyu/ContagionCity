#include "stdafx.h"
#include "Trigger.h"


CTrigger::CTrigger( int Stage ) : CGameObject( 0 )
{
	potalStage = Stage;
	m_bcMeshBoundingCube.m_vMax = XMFLOAT3( 50.0f, 50.0f, 50.0f );
	m_bcMeshBoundingCube.m_vMin = XMFLOAT3( -50.0f, -50.0f, -50.0f );
	m_iLayer = TRIGGER;
}


CTrigger::~CTrigger( )
{
}
