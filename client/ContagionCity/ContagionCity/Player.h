#pragma once

#include "Object.h"
#include "Camera.h"

class CPlayer : public AnimatedObjectInfo
{
protected:
	CCamera *m_pCamera;
	bool m_bInUpdateCamera;
protected:
	// 플레이어의 위치 벡터, x,y,z 축 벡터
	XMFLOAT3 m_vPosition;
	XMFLOAT3 m_vRight;
	XMFLOAT3 m_vUp;
	XMFLOAT3 m_vLook;

	// 플레이어가 로컬 x,y,z축으로 얼만큼 회전했는지 나타내는 변수
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// 플레이어의 이동속도를 나타내는 벡터
	XMFLOAT3 m_vVelocity;
	// 플레이어에 작용하는 중력을 나타내는 벡터
	XMFLOAT3 m_vGravity;
	// xz평면에서 플레이어의 이동 속력의 최대값
	float m_fMaxVelocityXZ;
	// y축 방향으로 플레이어의 이동 속력의 최대값
	float m_fMaxVelocityY;
	// 플레이어에 작용하는 마찰력
	float m_fFriction;

	// 플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdated() 함수에서 사용하는 데이터
	LPVOID m_pPlayerUpdatedContext;
	// 카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdated() 함수에서 사용하는 데이터
	LPVOID m_pCameraUpdatedContext;

public:
	CPlayer( CFbxMesh vertex, int nMeshes = 1 );
	virtual ~CPlayer( );

	// 플레이어의 현재 카메라를 설정하고 반환하는 멤버 함수
	void SetCamera( CCamera *pCamera ) { m_pCamera = pCamera; }
	CCamera* GetCamera( ) { return m_pCamera; }

	// 플레이어의 상수 버퍼를 생성하고 갱신하는 멤버 함수
	void CreateShaderVariables( ID3D11Device *pd3dDevice );
	void UpdateShaderVariables( ID3D11DeviceContext *pd3dDeviceContext );

	XMFLOAT3 GetPosition( )							{ return m_vPosition; }
	XMFLOAT3 GetLookVector( )						{ return m_vLook; }
	XMFLOAT3 GetUpVector( )							{ return m_vUp; }
	XMFLOAT3 GetRightVector( )						{ return m_vRight; }

	void SetFriction( float fFriction )				{ m_fFriction = fFriction; }
	void SetGravity( const XMFLOAT3& vGravity )		{ m_vGravity = vGravity; }
	void SetMaxVelocityXZ( float fMaxVel )			{ m_fMaxVelocityXZ = fMaxVel; }
	void SetMaxVelocityY( float fMaxVel )			{ m_fMaxVelocityY = fMaxVel; }

	/*플레이어의 위치를 d3dxvPosition 위치로 설정한다.
	d3dxvPosition 벡터에서 현재 플레이어의 위치 벡터를 빼면 현재 플레이어의 위치에서 d3dxvPosition 방향으로의 방향 벡터가 된다.
	현재 플레이어의 위치에서 이 방향 벡터 만큼 이동한다.*/
	void SetPosition( XMFLOAT3 vPosition ) {
		m_vPosition = vPosition;
		//Move( MathHelper::GetInstance( )->Float3MinusFloat3( vPosition, m_vPosition ), false );
		CGameObject::SetPosition( vPosition );
	}

	const XMFLOAT3& GetVelocity( ) const { return m_vVelocity; }
	float GetYaw( ) const { return m_fYaw; }
	float GetPitch( ) const { return m_fPitch; }
	float GetRoll( ) const { return m_fRoll; }

	// 플레이어를 이동하는 함수
	void Move( DWORD dwDirection, float fDistance, bool bUpdateVelocity );
	void Move( XMFLOAT3& vShift, bool bVelocity = false );
	void Move( float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f );
	// 플레이어를 회전하는 함수
	void Rotate( float x, float y, float z );

	// 플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수
	void Update( float fTimeElapsed, XMFLOAT3 DestPos );
	
	// 플레이어의 애니메이션
	virtual void Animate( float fTimeElapsed );

	// 플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수
	virtual void OnPlayerUpdated( float fTimeElapsed );
	void SetPlayerUpdatedContext( LPVOID pContext ) { m_pPlayerUpdatedContext = pContext; }
	// 카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수
	virtual void OnCameraUpdated( float fTimeElapsed );
	void SetCameraUpdatedContext( LPVOID pContext ) { m_pCameraUpdatedContext = pContext; }

	// 카메라를 변경할 때 호출되는 함수
	CCamera* OnChangeCamera( ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode );
	virtual void ChangeCamera( ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed );

	// 플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수
	virtual void OnPrepareRender( );
	// 플레이어의 카메라가 3인칭 카메라일 때 플레이어의 메시를 렌더링
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );

	float m_fSpeed;
};

