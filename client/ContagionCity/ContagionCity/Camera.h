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
	// 카메라 변환 행렬과 투영 변환 행렬을 나타내는 멤버 변수
	XMFLOAT4X4 m_mtxView;
	XMFLOAT4X4 m_mtxProjection;

	// 뷰포트를 나타내는 멤버 변수
	D3D11_VIEWPORT m_d3dViewport;

	// 카메라 변환 행렬과 투영 변환 행렬을 위한 상수 버퍼 인터페이스 포인터를 선언
	ID3D11Buffer *m_pd3dcbCamera;

	// 카메라에 연결된 플레이어 객체
	CPlayer *m_pPlayer;

	// 카메라의 위치 벡터
	XMFLOAT3 m_vPosition;
	// 카메라의 로컬 x,y,z축을 나타내는 벡터
	XMFLOAT3 m_vRight;
	XMFLOAT3 m_vUp;
	XMFLOAT3 m_vLook;

	// 카메라가 x,y,z축으로 얼마만큼 회전했는지를 나타내는 각도
	float m_fPitch;
	float m_fRoll;
	float m_fYaw;

	// 카메라의 종류
	DWORD m_nMode;

	// 카메라가 바라보는 점을 나타내는 벡터
	XMFLOAT3 m_vLookAtWorld;
	// 플레이어와 카메라의 와프셋
	XMFLOAT3 m_vOffset;
	// 플레이어가 회전할 때 얼마만큼의 시간을 지연시킨 후 카메라를 회전시킬 것인지 나타내는 변수
	float m_fTimeLag;

	// 절두체의 6개 평면
	XMFLOAT4 m_FrustumPlanes[6];

public:
	// 절두체의 6개 평면 계산
	void CalculateFrustumPlanes( );
	// 바운딩 박스가 절두체에 완전히 포함되거나 일부라도 포함되는지 검사
	bool IsInFrustum( XMFLOAT3& vMin, XMFLOAT3& vMax );
	bool IsInFrustum( AABB *pAABB );

	void SetPlayer( CPlayer *pPlayer ) { m_pPlayer = pPlayer; }
	CPlayer* GetPlayer( ) { return m_pPlayer; }

	// 뷰 포트를 설정하는 멤버함수
	void SetViewport( ID3D11DeviceContext *pd3dDeviceContext, DWORD xStart, DWORD yStart, DWORD nWidth, DWORD nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f );

	// 카메라 변환행렬과 투영변환행렬을 생성하는 멤버 함수 선언
	void GenerateProjectionMatrix( float fNearPlaneDist, float fFarPlaneDist, float fAspectRatio, float fFOVAngle );
	// 카메라 변환행렬을 생성
	void GenerateViewMatrix( );
	// 카메라가 여러번 회전하면 누적된 실수연산의 부적확성으로 인해 카메라의 x,y,z축이 서로 직교하지 않을 수 있다
	// 따라서 다시 직교하도록 만드는 함수를 구현한다.
	void RegenerateViewMatrix( );

	// 상수버퍼를 생성하고 내용을 갱신하는 함수
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

	// 카메라를 vShift만큼 이동하는 가상함수
	virtual void Move( XMFLOAT3& vShift ) { m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, vShift ); }
	// 카메라를 x,y,z축으로 회전하는 가상함수
	virtual void Rotate( float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f ){}
	// 카메라의 이동, 회전에 따라 카메라의 정보를 갱신하는 가상함수
	virtual void Update( XMFLOAT3& vLookAt, float fTimeElapsed ) {}
	// 3인칭 카메라에서 카메라가 바라보는 지점을 설정하는 가상함수, 플레이어를 바라보고 있는다
	virtual void SetLookAt( XMFLOAT3& vLookAt ) {}

}; 

/*
1인칭과, Space 모드는 Rotate를 통해서 바로 카메라 회전을 실시하지만
3인칭경우는 프레임별 회전각도를 계산해야 하므로 update로 회전을 실시한다.
3인칭경우는 Rotate 함수가 따로 없다.
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