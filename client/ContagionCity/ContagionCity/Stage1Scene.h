#pragma once
#include "Scene.h"

class CStage1Scene : public CScene
{
public:
	CStage1Scene( );
	~CStage1Scene( );

	virtual void LoadFBXs( );
	virtual void BuildObjects( ID3D11Device *pd3dDevice );

	// »ç¿îµå 
	virtual bool LoadingSoundResource( );

};

