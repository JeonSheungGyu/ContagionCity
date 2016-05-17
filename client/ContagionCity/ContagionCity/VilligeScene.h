#pragma once
#include "Scene.h"

class VilligeScene : public CScene
{
public:
	VilligeScene( );
	~VilligeScene( );

	virtual void CreateShaderVariables( ID3D11Device *pd3dDevice );
	virtual void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights );
	virtual void ReleaseShaderVariables( );

	virtual void LoadFBXs( );
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void AnimateObjects( float fTimeElapsed );

	// »ç¿îµå 
	virtual bool LoadingSoundResource( );
};

