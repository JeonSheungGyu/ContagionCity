#pragma once
#include "Mesh.h"
#include "Object.h"
#include "Shader.h"

class CTexture2D : public CMeshTextured
{	
public:
	float m_x;
	float m_y;
	
	CTexture2D( );
	CTexture2D( ID3D11Device *pd3dDevice, const TCHAR *pFileName, float x, float y );
	~CTexture2D( );
	void Render( ID3D11DeviceContext *pd3dDeviceContext );
};

class CTexture2DObject : public CGameObject
{
public:
	CTexture2DObject( ID3D11Device *pd3dDevice, const TCHAR *pFileName, float x, float y );
	~CTexture2DObject( );
};

class CTexture2DShader : public CTexturedShader
{
public:
	CTexture2DShader( ID3D11Device *pd3dDevice, const TCHAR *pFileName, float x, float y );
	~CTexture2DShader( );

	void MakeTransformToCamera( CCamera *pCamera, XMFLOAT3 pos );
};