#pragma once

#define DIR_FORWARD		0x01
#define DIR_BACKWARD	0x02
#define DIR_LEFT		0x04
#define DIR_RIGHT		0x08
#define DIR_UP			0x10
#define DIR_DOWN		0x20

#include "Mesh.h"
#include "Camera.h"

struct MATERIAL
{
	XMCOLOR m_cAmbient;
	XMCOLOR m_cDiffuse;
	XMCOLOR m_cSpecular;
	XMCOLOR m_cEmissive;
};

class CMaterial
{
public:
	CMaterial( );
	virtual ~CMaterial( );

private:
	int m_nReferences;

public:
	void AddRef( ) { m_nReferences++; }
	void Release( ) { if (--m_nReferences <= 0) delete this; }

	MATERIAL m_Material;
};

class CTexture
{
public:
	CTexture( int nTextures = 1, int nSamplers = 1, int nTextureStartSlot = 0, int nSamplerStartSlot = 0 );
	virtual ~CTexture( );

private:
	int m_nReferences;

public:
	void AddRef( ) { m_nReferences++; }
	void Release( ) { if (--m_nReferences <= 0) delete this; }

private:
	// 텍스처 리소스의 개수
	int m_nTextures;
	ID3D11ShaderResourceView **m_ppd3dsrvTextures;
	// 텍스처 리소스를 연결할 시작 슬롯
	int m_nTextureStartSlot;
	// 샘플러 상태 객체의 개수
	int m_nSamplers;
	ID3D11SamplerState **m_ppd3dSamplerStates;
	// 샘플러 상태 객체를 연결할 시작 슬롯
	int m_nSamplerStartSlot;

public:
	void SetTexture( int nIndex, ID3D11ShaderResourceView *pd3dsrvtexture );
	void SetSampler( int nIndex, ID3D11SamplerState *pd3dSamplerState );
	// 텍스처 리소스와 샘플러 상태 객체에 대한 셰이더 변수를 변경
	void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext );
	// 텍스처 리소스에 대한 셰이더 변수를 변경
	void UpdateTextureShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0 );
	// 샘플러 상태 객체에 대한 셰이더 변수를 변경
	void UpdateSamplerShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0 );
};

// Object.h 파일에 게임 객체를 표현하기 위한 CGameObject 클래스를 다음과 같이 선언
// 게임 객체는 적어도 월드 변환 행렬과 메시를 가져야 한다
// 메시는 같은 종류의 객체들이 하나의 메시를 공유할 수 있도록 메시에 대한 포인터로 선언
class CGameObject
{
public:
	// 객체는 여러개의 메시를 가질 수 없다.
	CGameObject( int nMeshes = 0 );
	virtual ~CGameObject( );

private:
	int m_nReferences;

public:
	// 오브젝트의 타입 설정, 무엇인지 알 수 있어야한다.
	int m_iType;

	// 게임 객체는 하나의 재질을 가질 수 있다.
	CMaterial *m_pMaterial;
	void SetMaterial( CMaterial *pMaterial );

	bool IsVisible( CCamera *pCamera = NULL );

	void AddRef( ) { m_nReferences++; }
	void Release( ) { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4X4 m_mtxWorld;

	XMFLOAT3 m_vMin, m_vMax;
	XMFLOAT3 m_vVertices[8];

	virtual void Animate( float fTimeElapsed );

	XMFLOAT3 m_vMovingDirection;
	float	 m_fMovingSpeed;
	float	 m_fMovingRange;
	bool	 m_bInChangeDirection;

	XMFLOAT3 m_vRotationAxis;
	float	 m_fRotationSpeed;

	// 게임 객체는 텍스처를 가질 수 있다.
	CTexture *m_pTexture;
	void SetTexture( CTexture *pTexture );
	CTexture* GetTexture( ) { return m_pTexture; }

	// 월드 변환
	void SetMovingDeirection( XMFLOAT3 vMovingDirection ){
		m_vMovingDirection = MathHelper::GetInstance( )->NormalizeFloat( vMovingDirection );
		m_bInChangeDirection = true;
	}
	void SetMovingSpeed( float fSpeed ) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange( float fRange ) { m_fMovingRange = fRange; }

	void SetRotationAxis( XMFLOAT3 vRotationAxis ) {
		m_vRotationAxis = MathHelper::GetInstance( )->NormalizeFloat( m_vRotationAxis );
	}
	void SetRotationSpeed( float fSpeed ) { m_fRotationSpeed = fSpeed; }

	void SetPosition( float x, float y, float z );
	void SetPosition( XMFLOAT3 vPosition );
	XMFLOAT3 GetPosition( );
	
	void Move( XMFLOAT3& vDirection, float fSpeed );

	// 로컬 x,y,z 축 방향으로 이동
	void MoveStrafe( float fDistance = 1.0f );
	void MoveUp( float fDistance = 1.0f );
	void MoveForward( float fDistance = 1.0f );

	// 로컬 x,y,z 축 방향으로 회전
	void Rotate( float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f );
	void Rotate( XMFLOAT3 *vAxis, float fAngle );

	// 객체의 위치, 로컬 x,y,z축 방향 벡터를 반환
	XMFLOAT3 GetLookAt( );
	XMFLOAT3 GetUp( );
	XMFLOAT3 GetRight( );

	// 객체를 렌더링하기 전에 호출되는 함수
	virtual void OnPrepareRender( ){}
public:
	// 객체가 가지는 메시들에 대한 포인터와 그 개수
	CMesh **m_ppMeshes;
	int m_nMeshes;

	// 객체가 가지는 메시 전체에 대한 바운딩 박스
	AABB m_bcMeshBoundingCube;

	void SetMesh( CMesh *pMesh, int nIndex = 0 );
	CMesh* GetMesh( int nIndex = 0 ){ return m_ppMeshes[nIndex]; }

	//메시 단위의 절두체 컬링을 하기위해 카메라가 필요
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );
};

class CSkyBox : public CGameObject
{
public:
	CSkyBox( ID3D11Device *pd3dDevice );
	virtual ~CSkyBox( );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );
};

class CGround : public CGameObject
{
public:
	CGround( ID3D11Device *pd3dDevice );
	virtual ~CGround( );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );
};