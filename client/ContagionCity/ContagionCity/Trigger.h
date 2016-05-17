#pragma once
#include "Object.h"
class CTrigger : public CGameObject
{
	// 메시가 필요없음 그려지지 않음
	// 특정 위치에 오면 함수 실행
	
public:
	CTrigger( int Stage);
	~CTrigger( );

	int potalStage;
};

