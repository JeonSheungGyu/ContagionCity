#pragma once

#include "Object.h"
#include "Camera.h"
#include "Player.h"
#include "Mesh.h"

// ���� ������ Ÿ�� ���� �� ���
struct VS_CB_WORLD_MATRIX
{
	XMFLOAT4X4 m_mtxWorld;
};

struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_mtxTransform;
};

struct VS_CB_OFFSET_MATRIX
{
	XMFLOAT4X4 m_mtxOffsets[96];
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

	int getObjectCount( ){ return m_nObjects; }
	std::vector<CGameObject*> getObjects( ){ return m_ppObjects; }

protected:
	ID3D11VertexShader *m_pd3dVertexShader;
	ID3D11InputLayout *m_pd3dVertexLayout;
	ID3D11PixelShader *m_pd3dPixelShader;

	// ���̴� ��ü�� ���� ��ü���� ����Ʈ�� ������.
	std::vector<CGameObject*> m_ppObjects;
	int m_nObjects;

	// ���� ��ȯ ����� ���� ��� ���۴� �ϳ��� �־ �Ǳ� ������ �������� ����
	static ID3D11Buffer *m_pd3dcbWorldMatrix;
};

class CTexturedShader : public CShader
{
public:
	CTexturedShader( );
	virtual ~CTexturedShader( );

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

class CAnimatedObjShader : public CTexturedShader
{
public:
	CAnimatedObjShader( );
	virtual ~CAnimatedObjShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void CreateShaderVariables( ID3D11Device *pd3dDevice );
	static void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, std::vector<XMFLOAT4X4> pmtxWorld );

	static ID3D11Buffer *m_pd3dcbOffsetMatrix;
};
// �÷��̾ �������ϱ� ���� ���̴� Ŭ����
class CPlayerShader : public CAnimatedObjShader
{
public:
	int m_rightHandIdx;

	CPlayerShader( );
	virtual ~CPlayerShader( );

	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );

	bool CollisionCheck( CGameObject* pObject );

	CPlayer* GetPlayer( int nIndex = 0 ){ return (CPlayer*)m_ppObjects[nIndex]; }
};

class CEnemyShader : public CAnimatedObjShader
{
public:
	CEnemyShader( );
	virtual ~CEnemyShader( );

	UINT m_nInstanceBufferStride;
	UINT m_nInstanceBufferOffset;
	ID3D11Buffer *m_pd3dInstanceBuffer;

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	ID3D11Buffer *CreateInstanceBuffer( ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData );
	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
};

// �÷��̾��� bone�� ������ �ϱ� ���� ���̴�Ŭ����
class CPlayerBoneShader : public CShader
{
public:
	CPlayerBoneShader( );
	virtual ~CPlayerBoneShader( );

	SkinnedData m_skinnedData;
	float m_TimePos;

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex );
	virtual void AnimateObjects( float fTimeElapsed );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
};

// ���ٴ��� �������ϱ� ���� ���̴� Ŭ����
class CStage1BackgroundShader : public CTexturedShader
{
public:
	CStage1BackgroundShader( );
	virtual ~CStage1BackgroundShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
};

class CVilligeBackgroundShader : public CTexturedShader
{
public:
	CVilligeBackgroundShader( );
	virtual ~CVilligeBackgroundShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
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

class CSkyBoxShader : public CTexturedShader
{
public:
	CSkyBoxShader( );
	virtual ~CSkyBoxShader( );

	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );
};
