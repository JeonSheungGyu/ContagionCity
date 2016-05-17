#pragma once
#include "Scene.h"

class CStage1Scene : public CScene
{
public:
	CStage1Scene( );
	~CStage1Scene( );

	virtual void CreateShaderVariables( ID3D11Device *pd3dDevice );
	virtual void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights );
	virtual void ReleaseShaderVariables( );

	virtual void LoadFBXs( );
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void AnimateObjects( float fTimeElapsed );

	// ���� 
	virtual bool LoadingSoundResource( );

};

