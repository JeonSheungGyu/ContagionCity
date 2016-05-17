#pragma once
#include "GameFramework.h"

class CAppManager
{
private:
	CAppManager( );

public:
	static CAppManager *Instance;
	static CAppManager *GetInstance( ) { if (Instance == NULL) Instance = new CAppManager; return Instance; }
	~CAppManager( );

	CGameFramework *m_pFrameWork;
	RECT m_ClientRect;
};

