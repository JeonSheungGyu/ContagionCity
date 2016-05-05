#include "stdafx.h"
#include "Player.h"


CPlayer::CPlayer( CFbxMesh vertex, int nMeshes ) : AnimatedObjectInfo( vertex, nMeshes )
{
	m_pCamera = NULL;

	m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_vRight = XMFLOAT3( 1.0f, 0.0f, 0.0f );
	m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
	m_vLook = XMFLOAT3( 0.0f, 0.0f, 1.0f );

	m_vVelocity = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_vGravity = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_fMaxVelocityXZ = 50.0f;
	m_fMaxVelocityY = 50.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

	m_fTimePos = 0.0f;
}


CPlayer::~CPlayer( )
{
	if (m_pCamera)
		delete m_pCamera;
}

void CPlayer::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	m_pmtxFinalTransforms = ( (CAnimatedMesh*)GetMesh( ) )->GetSkinnedData( ).mBoneOffsets;
}

void CPlayer::UpdateShaderVariables( ID3D11DeviceContext *pd3dDeviceContext )
{
	if (m_pCamera) 
		m_pCamera->UpdateShaderVariables( pd3dDeviceContext );

}

/*�÷��̾��� ��ġ�� �����ϴ� �Լ�
�÷��̾��� ��ġ�� �⺻������ ����ڰ� �÷��̾ �̵��ϱ� ���� Ű���带 ���� �� ����
�÷��̾��� �̵� ����(dwDirection)�� ���� �÷��̾ fDistance ��ŭ �̵�*/
void CPlayer::Move( DWORD dwDirection, float fDistance, bool bUpdateVelocity )
{
	if (dwDirection)
	{
		fDistance *= 3;
		XMFLOAT3 vShift = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		if (dwDirection & DIR_FORWARD)		
			vShift = MathHelper::GetInstance( )->Float3PlusFloat3( vShift, MathHelper::GetInstance( )->Float3MulFloat( m_vLook, fDistance ) );
		if (dwDirection & DIR_BACKWARD)		
			vShift = MathHelper::GetInstance( )->Float3MinusFloat3( vShift, MathHelper::GetInstance( )->Float3MulFloat( m_vLook, fDistance ) );
		if (dwDirection & DIR_RIGHT)		
			vShift = MathHelper::GetInstance( )->Float3PlusFloat3( vShift, MathHelper::GetInstance( )->Float3MulFloat( m_vRight, fDistance ) );
		if (dwDirection & DIR_LEFT)			
			vShift = MathHelper::GetInstance( )->Float3MinusFloat3( vShift, MathHelper::GetInstance( )->Float3MulFloat( m_vRight, fDistance ) );

		Move( vShift, bUpdateVelocity );
	}
}

void CPlayer::Move( XMFLOAT3& vShift, bool bUpdateVelocity )
{
	if (bUpdateVelocity)
	{
		m_vVelocity = MathHelper::GetInstance( )->Float3PlusFloat3( m_vVelocity, vShift );
	}
	else
	{
		// �÷��̾ ���� ��ġ���� vshift��ŭ�̵�
		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, vShift );
		m_vPosition = vPosition;
		// �÷��̾ �̵������Ƿ� ī�޶� �̵�
		m_pCamera->Move( vShift );
	}
}

void CPlayer::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
	DWORD nCurrentCameraMode = m_pCamera->GetMode( );

	// 1,3��Ī ī�޶��� ��� �÷��̾� ȸ���� ����
	if (nCurrentCameraMode == FIRST_PERSON_CAMERA || nCurrentCameraMode == THIRD_PERSON_CAMERA)
	{
		/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�.
		�׷��Ƿ� x-���� �߽����� ȸ���ϴ� ������ -89.0~+89.0�� ���̷� �����Ѵ�.
		x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ���� Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= ( m_fPitch - 89.0f ); m_fPitch = 89.0f; }
			if (m_fPitch < -89.0f) { x -= ( m_fPitch + 89.0f ); m_fPitch = -89.0f; }
		}
		// ���� y���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵȴ�.
		z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������ ������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > 20.0f) { z -= ( m_fRoll - 20.0f ); m_fRoll = 20.0f; }
			if (m_fRoll < -20.0f) { z -= ( m_fRoll + 20.0f ); m_fRoll = -20.0f; }
		}
		// ī�޶� x,y,z ��ŭ ȸ�� �÷��̾ ȸ���ϸ� ī�޶� ȸ����
		m_pCamera->Rotate( x, y, z );

		/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��.
		�÷��̾��� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��.
		�⺻������ Up ���͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
		if (y != 0.0f)
		{
			mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, y );
			m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
			m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		// ȸ�� ������ ����
		m_pCamera->Rotate( x, y, z );
		if (x != 0.0f)
		{
			mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vRight, x );
			m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
			m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		}
		if (y != 0.0f)
		{
			mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, y );
			m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
			m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		}
		if( z != 0.0f )
		{
			mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vLook, z );
			m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
			m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		}
	}

	/*ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ�
	z-��(LookAt ����)�� �������� �Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.*/
	m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
	m_vRight = MathHelper::GetInstance( )->CrossFloat3( m_vUp, m_vLook );
	m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
	m_vUp = MathHelper::GetInstance( )->CrossFloat3( m_vLook, m_vRight );
	m_vUp = MathHelper::GetInstance( )->NormalizeFloat( m_vUp );
}

void CPlayer::Update( float fTimeElapsed, XMFLOAT3 DestPos  )
{
	/*�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�.
	�߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ��� �����Ѵٴ� �ǹ��̴�.*/
	if (MathHelper::GetInstance( )->DistanceVector3ToVector3( DestPos, m_vPosition ) > 10)
		m_vGravity = MathHelper::GetInstance( )->Float3PlusFloat3( DestPos, m_vPosition ); // XMFLOAT3( 0, 0, 0 );
	else
		m_vGravity = XMFLOAT3( 0, 0, 0 );
	m_vVelocity = MathHelper::GetInstance( )->Float3MulFloat( m_vGravity, fTimeElapsed );

	/*�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�.
	�̰��� XZ-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-���� ������ �����Ѵ�.*/
	float fLength = sqrtf( m_vVelocity.x * m_vVelocity.x + m_vVelocity.z * m_vVelocity.z );
	float fMaxVeclocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityY) m_vVelocity.y *= ( m_fMaxVelocityY / fLength );

	// �÷��̾ �ӵ� ���͸�ŭ ������ �̵�
	Move( m_vVelocity, false );

	/*�÷��̾��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�.
	���� ���, �÷��̾��� ��ġ�� ����Ǿ����� �÷��̾� ��ü���� ����(Terrain)�� ������ ����.
	�÷��̾��� ���ο� ��ġ�� ��ȿ�� ��ġ�� �ƴ� ���� �ְ� �Ǵ� �÷��̾��� �浹 �˻� ���� ������ �ʿ䰡 �ִ�.
	�̷��� ��Ȳ���� �÷��̾��� ��ġ�� ��ȿ�� ��ġ�� �ٽ� ������ �� �ִ�.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdated( fTimeElapsed );

	DWORD nCurrentCameraMode = m_pCamera->GetMode( );
	// �÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ���¸� ����
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update( m_vPosition, fTimeElapsed );
	// ī�޶���� ��ġ�� ����� �� �߰������� ������ �۾� ����
	if (m_pCameraUpdatedContext) OnCameraUpdated( fTimeElapsed );
	// ī�޶� 3��Ī ī�޶��̸� ī�޶� ����� �÷��̾ �ٶ󺸵��� �Ѵ�.
//	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt( m_vPosition );
	// ī�޶��� ī�޶� ��ȯ����� �ٽ� ����
	m_pCamera->RegenerateViewMatrix( );

	/*�÷��̾��� �ӵ� ���Ͱ� ������ ������ ������ �Ǿ�� �Ѵٸ� ���� ���͸� �����Ѵ�.
	�ӵ� ������ �ݴ� ���� ���͸� ���ϰ� ���� ���ͷ� �����.
	���� ����� �ð��� ����ϵ��� �Ͽ� �������� ���Ѵ�.
	���� ���Ϳ� �������� ���Ͽ� ���� ���͸� ���Ѵ�.
	�ӵ� ���Ϳ� ���� ���͸� ���Ͽ� �ӵ� ���͸� ���δ�.
	�������� �ӷº��� ũ�� �ӷ��� 0�� �� ���̴�.*/
	XMFLOAT3 vDeceleration = MathHelper::GetInstance( )->Float3MulFloat( m_vVelocity, -1 );
	vDeceleration = MathHelper::GetInstance( )->NormalizeFloat( vDeceleration );
	fLength = MathHelper::GetInstance( )->Float3ToLength( m_vVelocity );
	float fDeceleration = ( m_fFriction * fTimeElapsed );
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_vVelocity = MathHelper::GetInstance( )->Float3PlusFloat3( m_vVelocity, MathHelper::GetInstance( )->Float3MulFloat( vDeceleration, fDeceleration ) );
}

// ī�޶� �����ϰ� �÷��̾��� ���� ����
CCamera* CPlayer::OnChangeCamera( ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode )
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera( m_pCamera );
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera( m_pCamera );
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera( m_pCamera );
			break;
		default:
			break;
	}

	/*���� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰� 
	���ο� ī�޶� 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� Up ���͸� 
	������ǥ���� y-�� ���� ����(0, 1, 0)�� �ǵ��� �Ѵ�. ��, �ȹٷ� ������ �Ѵ�. 
	�׸��� �����̽�-�� ī�޶��� ��� �÷��̾��� �̵����� ������ ����. Ư��, y-�� ������ �������� �����Ӵ�. 
	�׷��Ƿ� �÷��̾��� ��ġ�� ����(��ġ ������ y-��ǥ�� 0���� ũ��)�� �� �� �ִ�. 
	�̶� ���ο� ī�޶� 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� ��ġ�� ������ �Ǿ�� �Ѵ�. 
	�׷��Ƿ� �÷��̾��� Right ���Ϳ� Look ������ y ���� 0���� �����. 
	���� �÷��̾��� Right ���Ϳ� Look ���ʹ� �������Ͱ� �ƴϹǷ� ����ȭ�Ѵ�.*/

	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
		m_vRight.y = 0.0f;
		m_vLook.y = 0.0f;
		m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
		m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look ���Ϳ� ������ǥ���� z-��(0, 0, 1)�� �̷�� ����(����=cos)�� ����Ͽ�
		�÷��̾��� y-���� ȸ�� ���� m_fYaw�� �����Ѵ�.*/
		m_fYaw = (float)XMConvertToDegrees( acosf( MathHelper::GetInstance( )->Float3ToDot( XMFLOAT3( 0.0f, 0.0f, 1.0f ), m_vLook ) ) );
		if (m_vLook.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if (nNewCameraMode == SPACESHIP_CAMERA && m_pCamera)
	{
		/*���ο� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰�
		���� ī�޶� ��尡 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾���
		���� ���� ���� ī�޶��� ���� ��� ���� �����.*/
		m_vRight = m_pCamera->GetRightVector( );
		m_vUp = m_pCamera->GetUpVector( );
		m_vLook = m_pCamera->GetLookVector( );
	}

	if (pNewCamera)
	{
		// ���� ī�޶� ������ ���ο� ī�޶� ���� ���̴� ���� ����
		if (!m_pCamera) pNewCamera->CreateShaderVariables( pd3dDevice );
		pNewCamera->SetMode( nNewCameraMode );
		// ���� ī�޶� ����ϴ� �÷��̾� ��ü�� ����
		pNewCamera->SetPlayer( this );
	}

	if (m_pCamera) delete m_pCamera;

	return pNewCamera;
}

// �÷��̾� ��ġ ������
void CPlayer::OnPlayerUpdated( float fTimeElapsed )
{

}

void CPlayer::OnCameraUpdated( float fTimeElapsed )
{

}

void CPlayer::ChangeCamera( ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed )
{
	DWORD nCurrentCameraMode = ( m_pCamera ) ? m_pCamera->GetMode( ) : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction( 250.0f );
			SetGravity( XMFLOAT3( 0.0f, -300.0f, 0.0f ) );
			SetMaxVelocityXZ( 300.0f );
			SetMaxVelocityY( 400.0f );
			m_pCamera = OnChangeCamera( pd3dDevice, FIRST_PERSON_CAMERA, nCurrentCameraMode );
			m_pCamera->SetTimeLag( 0.0f );
			m_pCamera->SetOffset( XMFLOAT3( 0.0f, 20.0f, 0.0f ) );
			m_pCamera->GenerateProjectionMatrix( 1.01f, 50000.0f, ASPECT_RATIO, 60.0f );
			break;

		case SPACESHIP_CAMERA:
			SetFriction( 125.0f );
			SetGravity( XMFLOAT3( 0.0f, 0.0f, 0.0f ) );
			SetMaxVelocityXZ( 300.0f );
			SetMaxVelocityY( 400.0f );
			m_pCamera = OnChangeCamera( pd3dDevice, SPACESHIP_CAMERA, nCurrentCameraMode );
			m_pCamera->SetTimeLag( 0.0f );
			m_pCamera->SetOffset( XMFLOAT3( 0.0f, 0.0f, 0.0f ) );
			m_pCamera->GenerateProjectionMatrix( 1.01f, 50000.0f, ASPECT_RATIO, 60.0f );
			break;

		case THIRD_PERSON_CAMERA:
			SetFriction( 250.0f );
			SetGravity( XMFLOAT3( 0.0f, -300.0f, 0.0f ) );
			SetMaxVelocityXZ( 50.0f );
			SetMaxVelocityY( 50.0f );
			m_pCamera = OnChangeCamera( pd3dDevice, THIRD_PERSON_CAMERA, nCurrentCameraMode );
			m_pCamera->SetTimeLag( 0.25f );
			m_pCamera->SetOffset( XMFLOAT3( 0.0f, 200.0f, 400.0f ) );
			m_pCamera->GenerateProjectionMatrix( 1.01f, 50000.0f, ASPECT_RATIO, 60.0f );
			break;

		default:
			break;
	}
	Update( fTimeElapsed, XMFLOAT3(0,0,0) );
}

/*�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ�
�÷��̾��� Right ���Ͱ� ���� ��ȯ ����� ù ��° �� ����,
Up ���Ͱ� �� ��° �� ����,
Look ���Ͱ� �� ��° �� ����,
�÷��̾��� ��ġ ���Ͱ� �� ��° �� ���Ͱ� �ȴ�.*/
void CPlayer::OnPrepareRender( )
{
	m_mtxWorld._11 = m_vRight.x; m_mtxWorld._12 = m_vRight.y; m_mtxWorld._13 = m_vRight.z;
	m_mtxWorld._21 = m_vUp.x;	 m_mtxWorld._22 = m_vUp.y;	  m_mtxWorld._23 = m_vUp.z;
	m_mtxWorld._31 = m_vLook.x;  m_mtxWorld._32 = m_vLook.y;  m_mtxWorld._33 = m_vLook.z;
	m_mtxWorld._41 = m_vPosition.x;
	m_mtxWorld._42 = m_vPosition.y;
	m_mtxWorld._43 = m_vPosition.z;
}

void CPlayer::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CGameObject::Render( pd3dDeviceContext, pCamera );
}

void CPlayer::Animate( float fTimeElapsed )
{
	m_fTimePos += fTimeElapsed;
	if (m_bVisible)
		( (CAnimatedMesh *)GetMesh( ) )->GetSkinnedData( ).GetFinalTransforms( m_iAnimState, m_fTimePos, m_pmtxFinalTransforms );

	if (m_fTimePos > ( (CAnimatedMesh *)GetMesh( ) )->GetSkinnedData( ).GetClipEndTime( m_iAnimState ))
		m_fTimePos = 0.0f;
}