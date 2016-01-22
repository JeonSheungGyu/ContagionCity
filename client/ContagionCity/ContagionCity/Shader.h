#pragma once

#include "Object.h"
#include "Camera.h"
#include "Player.h"

// ���� ������ Ÿ�� ���� �� ���
struct VS_CB_WORLD_MATRIX
{
	XMFLOAT4X4 m_mtxWorld;
};

struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_mtxTransform;
};

class CShader
{
public:
	CShader( );
	virtual ~CShader( );

	void CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName,
		LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader,
		D3D11_INPUT_ELEMENT_DESC *pd3dInputLayout, UINT nElements, ID3D11InputLayout **ppd3dVertexLayout );
	void CreatePixelShaderFromFile( ID3D11Device *pd3dDevice, WCHAR *pszFilename,
		LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader );

	virtual void CreateShader( ID3D11Device *pd3dDeivce );

	static void CreateShaderVariables( ID3D11Device *pd3dDevice );
	static void ReleaseShaderVariables( );
	static void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 *pmtxWorld );

	// ���� ��ü���� �����ϰ� �ִϸ��̼� ó���ϰ� �������ϱ� ���� �Լ�
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void ReleaseObject( );
	virtual void AnimateObjects( float fTiemElapsed );
	virtual void OnPrepareRender( ID3D11DeviceContext *pd3dDeviceContext );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );

protected:
	ID3D11VertexShader *m_pd3dVertexShader;
	ID3D11InputLayout *m_pd3dVertexLayout;
	ID3D11PixelShader *m_pd3dPixelShader;

	// ���̴� ��ü�� ���� ��ü���� ����Ʈ�� ������.
	CGameObject **m_ppObjects;
	int m_nObjects;

	// ���� ��ȯ ����� ���� ��� ���۴� �ϳ��� �־ �Ǳ� ������ �������� ����
	static ID3D11Buffer *m_pd3dcbWorldMatrix;
};

class CDiffusedShader : public CShader
{
public:
	CDiffusedShader( );
	virtual ~CDiffusedShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
};

// ���� ��ü���� �������ϱ� ���� ���̴� Ŭ����
class CSceneShader : public CShader
{
public:
	CSceneShader( );
	virtual ~CSceneShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
};

// �÷��̾ �������ϱ� ���� ���̴� Ŭ����
class CPlayerShader : public CDiffusedShader
{
public:
	CPlayerShader( );
	virtual ~CPlayerShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );

	CPlayer* GetPlayer( int nIndex = 0 ){ return (CPlayer*)m_ppObjects[nIndex]; }
};

class CIlluminatedShader : public CShader
{
public:
	// ������ �����ϱ� ���� �������
	static ID3D11Buffer *m_pd3dcbMaterial;

	CIlluminatedShader( );
	virtual ~CIlluminatedShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	static void CreateShaderVariables( ID3D11Device *pd3dDevice );
	static void ReleaseShaderVariables( );

	// ������ ���̴� ������ �����ϱ� ���� �Լ�
	static void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, MATERIAL *pMaterial );
};

class CTexturedShader : public CShader
{
public:
	CTexturedShader( );
	virtual ~CTexturedShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
};

class CDetailTexturedShader : public CTexturedShader
{
public:
	CDetailTexturedShader( );
	virtual ~CDetailTexturedShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
};

class CSkyBoxShader : public CTexturedShader
{
public:
	CSkyBoxShader( );
	virtual ~CSkyBoxShader( );

	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );
};