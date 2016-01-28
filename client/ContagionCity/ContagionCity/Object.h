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
	// �ؽ�ó ���ҽ��� ����
	int m_nTextures;
	ID3D11ShaderResourceView **m_ppd3dsrvTextures;
	// �ؽ�ó ���ҽ��� ������ ���� ����
	int m_nTextureStartSlot;
	// ���÷� ���� ��ü�� ����
	int m_nSamplers;
	ID3D11SamplerState **m_ppd3dSamplerStates;
	// ���÷� ���� ��ü�� ������ ���� ����
	int m_nSamplerStartSlot;

public:
	void SetTexture( int nIndex, ID3D11ShaderResourceView *pd3dsrvtexture );
	void SetSampler( int nIndex, ID3D11SamplerState *pd3dSamplerState );
	// �ؽ�ó ���ҽ��� ���÷� ���� ��ü�� ���� ���̴� ������ ����
	void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext );
	// �ؽ�ó ���ҽ��� ���� ���̴� ������ ����
	void UpdateTextureShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0 );
	// ���÷� ���� ��ü�� ���� ���̴� ������ ����
	void UpdateSamplerShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0 );
};

// Object.h ���Ͽ� ���� ��ü�� ǥ���ϱ� ���� CGameObject Ŭ������ ������ ���� ����
// ���� ��ü�� ��� ���� ��ȯ ��İ� �޽ø� ������ �Ѵ�
// �޽ô� ���� ������ ��ü���� �ϳ��� �޽ø� ������ �� �ֵ��� �޽ÿ� ���� �����ͷ� ����
class CGameObject
{
public:
	// ��ü�� �������� �޽ø� ���� �� ����.
	CGameObject( int nMeshes = 0 );
	virtual ~CGameObject( );

private:
	int m_nReferences;

public:
	// ������Ʈ�� Ÿ�� ����, �������� �� �� �־���Ѵ�.
	int m_iType;

	// ���� ��ü�� �ϳ��� ������ ���� �� �ִ�.
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

	// ���� ��ü�� �ؽ�ó�� ���� �� �ִ�.
	CTexture *m_pTexture;
	void SetTexture( CTexture *pTexture );
	CTexture* GetTexture( ) { return m_pTexture; }

	// ���� ��ȯ
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

	// ���� x,y,z �� �������� �̵�
	void MoveStrafe( float fDistance = 1.0f );
	void MoveUp( float fDistance = 1.0f );
	void MoveForward( float fDistance = 1.0f );

	// ���� x,y,z �� �������� ȸ��
	void Rotate( float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f );
	void Rotate( XMFLOAT3 *vAxis, float fAngle );

	// ��ü�� ��ġ, ���� x,y,z�� ���� ���͸� ��ȯ
	XMFLOAT3 GetLookAt( );
	XMFLOAT3 GetUp( );
	XMFLOAT3 GetRight( );

	// ��ü�� �������ϱ� ���� ȣ��Ǵ� �Լ�
	virtual void OnPrepareRender( ){}
public:
	// ��ü�� ������ �޽õ鿡 ���� �����Ϳ� �� ����
	CMesh **m_ppMeshes;
	int m_nMeshes;

	// ��ü�� ������ �޽� ��ü�� ���� �ٿ�� �ڽ�
	AABB m_bcMeshBoundingCube;

	void SetMesh( CMesh *pMesh, int nIndex = 0 );
	CMesh* GetMesh( int nIndex = 0 ){ return m_ppMeshes[nIndex]; }

	//�޽� ������ ����ü �ø��� �ϱ����� ī�޶� �ʿ�
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