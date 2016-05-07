#pragma once

#define FIRST_PERSON_CAMERA	0x01
#define SPACESHIP_CAMERA	0x02
#define THIRD_PERSON_CAMERA	0x03
#define ASPECT_RATIO	(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))
#include "Mesh.h"

struct VS_CB_CAMERA
{
	XMFLOAT4X4 m_mtxView;
	XMFLOAT4X4 m_mtxProjection;
};

class CPlayer;

class CCamera
{
public:
	CCamera( );
	CCamera( CCamera *pCamera );
	virtual ~CCamera( );

protected:
	// ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ��Ÿ���� ��� ����
	XMFLOAT4X4 m_mtxView;
	XMFLOAT4X4 m_mtxProjection;

	// ����Ʈ�� ��Ÿ���� ��� ����
	D3D11_VIEWPORT m_d3dViewport;

	// ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ���� ��� ���� �������̽� �����͸� ����
	ID3D11Buffer *m_pd3dcbCamera;

	// ī�޶� ����� �÷��̾� ��ü
	CPlayer *m_pPlayer;

	// ī�޶��� ��ġ ����
	XMFLOAT3 m_vPosition;
	// ī�޶��� ���� x,y,z���� ��Ÿ���� ����
	XMFLOAT3 m_vRight;
	XMFLOAT3 m_vUp;
	XMFLOAT3 m_vLook;

	// ī�޶� x,y,z������ �󸶸�ŭ ȸ���ߴ����� ��Ÿ���� ����
	float m_fPitch;
	float m_fRoll;
	float m_fYaw;

	// ī�޶��� ����
	DWORD m_nMode;

	// ī�޶� �ٶ󺸴� ���� ��Ÿ���� ����
	XMFLOAT3 m_vLookAtWorld;
	// �÷��̾�� ī�޶��� ������
	XMFLOAT3 m_vOffset;
	// �÷��̾ ȸ���� �� �󸶸�ŭ�� �ð��� ������Ų �� ī�޶� ȸ����ų ������ ��Ÿ���� ����
	float m_fTimeLag;

	// ����ü�� 6�� ���
	XMFLOAT4 m_FrustumPlanes[6];

public:
	// ����ü�� 6�� ��� ���
	void CalculateFrustumPlanes( );
	// �ٿ�� �ڽ��� ����ü�� ������ ���Եǰų� �Ϻζ� ���ԵǴ��� �˻�
	bool IsInFrustum( XMFLOAT3& vMin, XMFLOAT3& vMax );
	bool IsInFrustum( AABB *pAABB );

	void SetPlayer( CPlayer *pPlayer ) { m_pPlayer = pPlayer; }
	CPlayer* GetPlayer( ) { return m_pPlayer; }

	// �� ��Ʈ�� �����ϴ� ����Լ�
	void SetViewport( ID3D11DeviceContext *pd3dDeviceContext, DWORD xStart, DWORD yStart, DWORD nWidth, DWORD nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f );

	// ī�޶� ��ȯ��İ� ������ȯ����� �����ϴ� ��� �Լ� ����
	void GenerateProjectionMatrix( float fNearPlaneDist, float fFarPlaneDist, float fAspectRatio, float fFOVAngle );
	// ī�޶� ��ȯ����� ����
	void GenerateViewMatrix( );
	// ī�޶� ������ ȸ���ϸ� ������ �Ǽ������� ����Ȯ������ ���� ī�޶��� x,y,z���� ���� �������� ���� �� �ִ�
	// ���� �ٽ� �����ϵ��� ����� �Լ��� �����Ѵ�.
	void RegenerateViewMatrix( );

	// ������۸� �����ϰ� ������ �����ϴ� �Լ�
	void CreateShaderVariables( ID3D11Device *pd3dDevice );
	void UpdateShaderVariables( ID3D11DeviceContext *pd3dDeviceContext );

	void SetMode( DWORD nMode ) { m_nMode = nMode; }
	DWORD GetMode( ) { return m_nMode; }
	
	D3D11_VIEWPORT GetViewport( ) { return m_d3dViewport; }

	XMFLOAT4X4 GetViewMatrix( ) { return m_mtxView; }
	XMFLOAT4X4 GetProjectionMatrix( ) { return m_mtxProjection; }
	
	ID3D11Buffer* GetCameraConstantBuffer( ) { return m_pd3dcbCamera; }
	void SetPosition( XMFLOAT3 vPosition ) { m_vPosition = vPosition; }
	XMFLOAT3& GetPosition( ){ return m_vPosition; }

	void SetLookAtPosition( XMFLOAT3 vLookAtWorld ) { m_vLookAtWorld = vLookAtWorld; }
	XMFLOAT3& GetLookAtPosition( ) { return m_vLookAtWorld; }

	XMFLOAT3& GetRightVector( ){ return m_vRight; }
	XMFLOAT3& GetUpVector( ) { return m_vUp; }
	XMFLOAT3& GetLookVector( ) { return m_vLook; }

	float& GetPitch( ) { return m_fPitch; }
	float& GetRoll( ) { return m_fRoll; }
	float& GetYaw( ) { return m_fYaw; }

	void SetOffset( XMFLOAT3 vOffset ) { m_vOffset = vOffset; m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, vOffset ); }
	XMFLOAT3& GetOffset( ){ return m_vOffset; }

	void SetTimeLag( float fTimeLag ){ m_fTimeLag = fTimeLag; }
	float GetTimeLag( ) { return m_fTimeLag; }

	// ī�޶� vShift��ŭ �̵��ϴ� �����Լ�
	virtual void Move( XMFLOAT3& vShift ) { m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, vShift ); }
	// ī�޶� x,y,z������ ȸ���ϴ� �����Լ�
	virtual void Rotate( float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f ){}
	// ī�޶��� �̵�, ȸ���� ���� ī�޶��� ������ �����ϴ� �����Լ�
	virtual void Update( XMFLOAT3& vLookAt, float fTimeElapsed ) {}
	// 3��Ī ī�޶󿡼� ī�޶� �ٶ󺸴� ������ �����ϴ� �����Լ�, �÷��̾ �ٶ󺸰� �ִ´�
	virtual void SetLookAt( XMFLOAT3& vLookAt ) {}

}; 

/*
1��Ī��, Space ���� Rotate�� ���ؼ� �ٷ� ī�޶� ȸ���� �ǽ�������
3��Ī���� �����Ӻ� ȸ�������� ����ؾ� �ϹǷ� update�� ȸ���� �ǽ��Ѵ�.
3��Ī���� Rotate �Լ��� ���� ����.
*/
class CSpaceShipCamera : public CCamera
{
public:
	CSpaceShipCamera( CCamera *pCamera );
	virtual ~CSpaceShipCamera( ) { }

	virtual void Rotate( float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f );
};


class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera( CCamera *pCamera );
	virtual ~CFirstPersonCamera( ) { }

	virtual void Rotate( float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f );
};


class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera( CCamera *pCamera );
	virtual ~CThirdPersonCamera( ) { }

	virtual void Update( XMFLOAT3& vLookAt, float fTimeElapsed );
	virtual void SetLookAt( XMFLOAT3& vLookAt );
	virtual void Rotate( float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f );
};