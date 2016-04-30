#pragma once

#include "Object.h"
#include "Camera.h"
#include "Player.h"
#include "Mesh.h"

// 버퍼 데이터 타입 정의 시 사용
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

	// 게임 객체들을 생성하고 애니메이션 처리하고 렌더링하기 위한 함수
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void ReleaseObject( );
	virtual void AnimateObjects( float fTiemElapsed );
	virtual void OnPrepareRender( ID3D11DeviceContext *pd3dDeviceContext );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );

	int getObjectCount( ){ return m_nObjects; }
	CGameObject** getObjects( ){ return m_ppObjects; }

protected:
	ID3D11VertexShader *m_pd3dVertexShader;
	ID3D11InputLayout *m_pd3dVertexLayout;
	ID3D11PixelShader *m_pd3dPixelShader;

	// 셰이더 객체가 게임 객체들의 리스트를 가진다.
	CGameObject **m_ppObjects;
	int m_nObjects;

	// 월드 변환 행렬을 위한 상수 버퍼는 하나만 있어도 되기 때문에 정적으로 선언
	static ID3D11Buffer *m_pd3dcbWorldMatrix;
};

class CTexturedShader : public CShader
{
public:
	CTexturedShader( );
	virtual ~CTexturedShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
};

// 게임 객체들을 렌더링하기 위한 셰이더 클래스
class CSceneShader : public CShader
{
public:
	CSceneShader( );
	virtual ~CSceneShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice );
};

// 플레이어를 렌더링하기 위한 세이더 클래스
class CPlayerShader : public CTexturedShader
{
public:
	CPlayerShader( );
	virtual ~CPlayerShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void CreateShaderVariables( ID3D11Device *pd3dDevice );
	virtual void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, std::vector<XMFLOAT4X4> pmtxWorld );
	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
	void CPlayerShader::AnimateObjects( float fTimeElapsed );

	bool CollisionCheck( CGameObject* pObject );

	CPlayer* GetPlayer( int nIndex = 0 ){ return (CPlayer*)m_ppObjects[nIndex]; }

	static ID3D11Buffer *m_pd3dcbOffsetMatrix;
};

// 플레이어의 bone을 렌더링 하기 위한 셰이더클래스
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

// 땅바닥을 렌더링하기 위한 셰이더 클래스
class CBackgroundShader : public CTexturedShader
{
public:
	CBackgroundShader( );
	virtual ~CBackgroundShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
};

class CIlluminatedShader : public CShader
{
public:
	// 재질을 설정하기 위한 상수버퍼
	static ID3D11Buffer *m_pd3dcbMaterial;

	CIlluminatedShader( );
	virtual ~CIlluminatedShader( );

	virtual void CreateShader( ID3D11Device *pd3dDevice );
	static void CreateShaderVariables( ID3D11Device *pd3dDevice );
	static void ReleaseShaderVariables( );

	// 재질을 셰이더 변수에 설정하기 위한 함수
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
