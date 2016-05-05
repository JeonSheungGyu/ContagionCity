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

/*플레이어의 위치를 변경하는 함수
플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를 누를 때 변경
플레이어의 이동 방향(dwDirection)에 따라 플레이어를 fDistance 만큼 이동*/
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
		// 플레이어를 현재 위치에서 vshift만큼이동
		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, vShift );
		m_vPosition = vPosition;
		// 플레이어가 이동했으므로 카메라도 이동
		m_pCamera->Move( vShift );
	}
}

void CPlayer::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
	DWORD nCurrentCameraMode = m_pCamera->GetMode( );

	// 1,3인칭 카메라의 경우 플레이어 회전의 제약
	if (nCurrentCameraMode == FIRST_PERSON_CAMERA || nCurrentCameraMode == THIRD_PERSON_CAMERA)
	{
		/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다.
		그러므로 x-축을 중심으로 회전하는 각도는 -89.0~+89.0도 사이로 제한한다.
		x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음 Pitch가 +89도 보다 크거나 -89도 보다 작으면 m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= ( m_fPitch - 89.0f ); m_fPitch = 89.0f; }
			if (m_fPitch < -89.0f) { x -= ( m_fPitch + 89.0f ); m_fPitch = -89.0f; }
		}
		// 로컬 y축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없음
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된다.
		z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다 작으면 m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > 20.0f) { z -= ( m_fRoll - 20.0f ); m_fRoll = 20.0f; }
			if (m_fRoll < -20.0f) { z -= ( m_fRoll + 20.0f ); m_fRoll = -20.0f; }
		}
		// 카메라를 x,y,z 만큼 회전 플레이어를 회전하면 카메라가 회전됨
		m_pCamera->Rotate( x, y, z );

		/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다.
		플레이어의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다.
		기본적으로 Up 벡터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
		if (y != 0.0f)
		{
			mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, y );
			m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
			m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		// 회전 제한이 없음
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

	/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로
	z-축(LookAt 벡터)을 기준으로 하여 서로 직교하고 단위벡터가 되도록 한다.*/
	m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
	m_vRight = MathHelper::GetInstance( )->CrossFloat3( m_vUp, m_vLook );
	m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
	m_vUp = MathHelper::GetInstance( )->CrossFloat3( m_vLook, m_vRight );
	m_vUp = MathHelper::GetInstance( )->NormalizeFloat( m_vUp );
}

void CPlayer::Update( float fTimeElapsed, XMFLOAT3 DestPos  )
{
	/*플레이어의 속도 벡터를 중력 벡터와 더한다.
	중력 벡터에 fTimeElapsed를 곱하는 것은 중력을 시간에 비례하도록 적용한다는 의미이다.*/
	if (MathHelper::GetInstance( )->DistanceVector3ToVector3( DestPos, m_vPosition ) > 10)
		m_vGravity = MathHelper::GetInstance( )->Float3PlusFloat3( DestPos, m_vPosition ); // XMFLOAT3( 0, 0, 0 );
	else
		m_vGravity = XMFLOAT3( 0, 0, 0 );
	m_vVelocity = MathHelper::GetInstance( )->Float3MulFloat( m_vGravity, fTimeElapsed );

	/*플레이어의 속도 벡터의 XZ-성분의 크기를 구한다.
	이것이 XZ-평면의 최대 속력보다 크면 속도 벡터의 x와 z-방향 성분을 조정한다.*/
	float fLength = sqrtf( m_vVelocity.x * m_vVelocity.x + m_vVelocity.z * m_vVelocity.z );
	float fMaxVeclocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityY) m_vVelocity.y *= ( m_fMaxVelocityY / fLength );

	// 플레이어를 속도 벡터만큼 실제로 이동
	Move( m_vVelocity, false );

	/*플레이어의 위치가 변경될 때 추가로 수행할 작업을 수행한다.
	예를 들어, 플레이어의 위치가 변경되었지만 플레이어 객체에는 지형(Terrain)의 정보가 없다.
	플레이어의 새로운 위치가 유효한 위치가 아닐 수도 있고 또는 플레이어의 충돌 검사 등을 수행할 필요가 있다.
	이러한 상황에서 플레이어의 위치를 유효한 위치로 다시 변경할 수 있다.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdated( fTimeElapsed );

	DWORD nCurrentCameraMode = m_pCamera->GetMode( );
	// 플레이어의 위치가 변경되었으므로 카메라의 상태를 갱신
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update( m_vPosition, fTimeElapsed );
	// 카메라라의 위치가 변경될 때 추가적으로 수행할 작업 수행
	if (m_pCameraUpdatedContext) OnCameraUpdated( fTimeElapsed );
	// 카메라가 3인칭 카메라이면 카메라가 변경된 플레이어를 바라보도록 한다.
//	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt( m_vPosition );
	// 카메라의 카메라 변환행렬을 다시 생성
	m_pCamera->RegenerateViewMatrix( );

	/*플레이어의 속도 벡터가 마찰력 때문에 감속이 되어야 한다면 감속 벡터를 생성한다.
	속도 벡터의 반대 방향 벡터를 구하고 단위 벡터로 만든다.
	마찰 계수를 시간에 비례하도록 하여 마찰력을 구한다.
	단위 벡터에 마찰력을 곱하여 감속 벡터를 구한다.
	속도 벡터에 감속 벡터를 더하여 속도 벡터를 줄인다.
	마찰력이 속력보다 크면 속력은 0이 될 것이다.*/
	XMFLOAT3 vDeceleration = MathHelper::GetInstance( )->Float3MulFloat( m_vVelocity, -1 );
	vDeceleration = MathHelper::GetInstance( )->NormalizeFloat( vDeceleration );
	fLength = MathHelper::GetInstance( )->Float3ToLength( m_vVelocity );
	float fDeceleration = ( m_fFriction * fTimeElapsed );
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_vVelocity = MathHelper::GetInstance( )->Float3PlusFloat3( m_vVelocity, MathHelper::GetInstance( )->Float3MulFloat( vDeceleration, fDeceleration ) );
}

// 카메라를 변경하고 플레이어의 상태 조정
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

	/*현재 카메라의 모드가 스페이스-쉽 모드의 카메라이고 
	새로운 카메라가 1인칭 또는 3인칭 카메라이면 플레이어의 Up 벡터를 
	월드좌표계의 y-축 방향 벡터(0, 1, 0)이 되도록 한다. 즉, 똑바로 서도록 한다. 
	그리고 스페이스-쉽 카메라의 경우 플레이어의 이동에는 제약이 없다. 특히, y-축 방향의 움직임이 자유롭다. 
	그러므로 플레이어의 위치는 공중(위치 벡터의 y-좌표가 0보다 크다)이 될 수 있다. 
	이때 새로운 카메라가 1인칭 또는 3인칭 카메라이면 플레이어의 위치는 지면이 되어야 한다. 
	그러므로 플레이어의 Right 벡터와 Look 벡터의 y 값을 0으로 만든다. 
	이제 플레이어의 Right 벡터와 Look 벡터는 단위벡터가 아니므로 정규화한다.*/

	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
		m_vRight.y = 0.0f;
		m_vLook.y = 0.0f;
		m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
		m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look 벡터와 월드좌표계의 z-축(0, 0, 1)이 이루는 각도(내적=cos)를 계산하여
		플레이어의 y-축의 회전 각도 m_fYaw로 설정한다.*/
		m_fYaw = (float)XMConvertToDegrees( acosf( MathHelper::GetInstance( )->Float3ToDot( XMFLOAT3( 0.0f, 0.0f, 1.0f ), m_vLook ) ) );
		if (m_vLook.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if (nNewCameraMode == SPACESHIP_CAMERA && m_pCamera)
	{
		/*새로운 카메라의 모드가 스페이스-쉽 모드의 카메라이고
		현재 카메라 모드가 1인칭 또는 3인칭 카메라이면 플레이어의
		로컬 축을 현재 카메라의 로컬 축과 같게 만든다.*/
		m_vRight = m_pCamera->GetRightVector( );
		m_vUp = m_pCamera->GetUpVector( );
		m_vLook = m_pCamera->GetLookVector( );
	}

	if (pNewCamera)
	{
		// 기존 카메라가 없으면 새로운 카메라를 위한 셰이더 변수 생성
		if (!m_pCamera) pNewCamera->CreateShaderVariables( pd3dDevice );
		pNewCamera->SetMode( nNewCameraMode );
		// 현재 카메라를 사용하는 플레이어 객체를 설정
		pNewCamera->SetPlayer( this );
	}

	if (m_pCamera) delete m_pCamera;

	return pNewCamera;
}

// 플레이어 위치 재조정
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

/*플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수
플레이어의 Right 벡터가 월드 변환 행렬의 첫 번째 행 벡터,
Up 벡터가 두 번째 행 벡터,
Look 벡터가 세 번째 행 벡터,
플레이어의 위치 벡터가 네 번째 행 벡터가 된다.*/
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