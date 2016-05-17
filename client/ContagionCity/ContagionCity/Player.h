#pragma once

#include "Object.h"
#include "Camera.h"

class CPlayer : public AnimatedObjectInfo
{
protected:
	CCamera *m_pCamera;
	bool m_bInUpdateCamera;
protected:
	// �÷��̾��� ��ġ ����, x,y,z �� ����
	XMFLOAT3 m_vPosition;
	XMFLOAT3 m_vRight;
	XMFLOAT3 m_vUp;
	XMFLOAT3 m_vLook;

	// �÷��̾ ���� x,y,z������ ��ŭ ȸ���ߴ��� ��Ÿ���� ����
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// �÷��̾��� �̵��ӵ��� ��Ÿ���� ����
	XMFLOAT3 m_vVelocity;
	// �÷��̾ �ۿ��ϴ� �߷��� ��Ÿ���� ����
	XMFLOAT3 m_vGravity;
	// xz��鿡�� �÷��̾��� �̵� �ӷ��� �ִ밪
	float m_fMaxVelocityXZ;
	// y�� �������� �÷��̾��� �̵� �ӷ��� �ִ밪
	float m_fMaxVelocityY;
	// �÷��̾ �ۿ��ϴ� ������
	float m_fFriction;

	// �÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdated() �Լ����� ����ϴ� ������
	LPVOID m_pPlayerUpdatedContext;
	// ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnCameraUpdated() �Լ����� ����ϴ� ������
	LPVOID m_pCameraUpdatedContext;

public:
	CPlayer( CFbxMesh vertex, int nMeshes = 1 );
	virtual ~CPlayer( );

	// �÷��̾��� ���� ī�޶� �����ϰ� ��ȯ�ϴ� ��� �Լ�
	void SetCamera( CCamera *pCamera ) { m_pCamera = pCamera; }
	CCamera* GetCamera( ) { return m_pCamera; }

	// �÷��̾��� ��� ���۸� �����ϰ� �����ϴ� ��� �Լ�
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

	/*�÷��̾��� ��ġ�� d3dxvPosition ��ġ�� �����Ѵ�.
	d3dxvPosition ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ���� �÷��̾��� ��ġ���� d3dxvPosition ���������� ���� ���Ͱ� �ȴ�.
	���� �÷��̾��� ��ġ���� �� ���� ���� ��ŭ �̵��Ѵ�.*/
	void SetPosition( XMFLOAT3 vPosition ) {
		m_vPosition = vPosition;
		//Move( MathHelper::GetInstance( )->Float3MinusFloat3( vPosition, m_vPosition ), false );
		CGameObject::SetPosition( vPosition );
	}

	const XMFLOAT3& GetVelocity( ) const { return m_vVelocity; }
	float GetYaw( ) const { return m_fYaw; }
	float GetPitch( ) const { return m_fPitch; }
	float GetRoll( ) const { return m_fRoll; }

	// �÷��̾ �̵��ϴ� �Լ�
	void Move( DWORD dwDirection, float fDistance, bool bUpdateVelocity );
	void Move( XMFLOAT3& vShift, bool bVelocity = false );
	void Move( float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f );
	// �÷��̾ ȸ���ϴ� �Լ�
	void Rotate( float x, float y, float z );

	// �÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ�
	void Update( float fTimeElapsed, XMFLOAT3 DestPos );
	
	// �÷��̾��� �ִϸ��̼�
	virtual void Animate( float fTimeElapsed );

	// �÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ�
	virtual void OnPlayerUpdated( float fTimeElapsed );
	void SetPlayerUpdatedContext( LPVOID pContext ) { m_pPlayerUpdatedContext = pContext; }
	// ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ�
	virtual void OnCameraUpdated( float fTimeElapsed );
	void SetCameraUpdatedContext( LPVOID pContext ) { m_pCameraUpdatedContext = pContext; }

	// ī�޶� ������ �� ȣ��Ǵ� �Լ�
	CCamera* OnChangeCamera( ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode );
	virtual void ChangeCamera( ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed );

	// �÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ�
	virtual void OnPrepareRender( );
	// �÷��̾��� ī�޶� 3��Ī ī�޶��� �� �÷��̾��� �޽ø� ������
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );

	float m_fSpeed;
};

