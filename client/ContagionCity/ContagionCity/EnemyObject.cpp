#include "stdafx.h"
#include "EnemyObject.h"


CEnemyObject::CEnemyObject( CFbxMesh vertex, int nMeshes ) : AnimatedObjectInfo( vertex, nMeshes )
{
	m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_vRight = XMFLOAT3( 1.0f, 0.0f, 0.0f );
	m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
	m_vLook = XMFLOAT3( 0.0f, 0.0f, 1.0f );

	m_vVelocity = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_fMaxVelocityXZ = 50.0f;
	m_fMaxVelocityY = 50.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_fTimePos = 0.0f;
	m_fSpeed = 1.5;
}


CEnemyObject::~CEnemyObject( )
{
}


void CEnemyObject::Move( DWORD dwDirection, float fDistance, bool bUpdateVelocity )
{
	if (dwDirection)
	{
		fDistance *= 3;
		XMFLOAT3 vShift = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		XMVECTOR playerDirection = XMVectorSet( m_fPitch, m_fYaw, m_fRoll, 0.0f );
		playerDirection = XMVector3Normalize( playerDirection );
		if (dwDirection & DIR_FORWARD){
			// 현재 플레이어가 보고 있는 방향과 XMFLOAT3(0.0f, 0.0f, -1.0f) 와 비교하여 방향을 설정함
			vShift = MathHelper::GetInstance( )->Float3MinusFloat3( vShift, MathHelper::GetInstance( )->Float3MulFloat( m_vLook, fDistance * m_fSpeed ) );
		}
		if (dwDirection & DIR_BACKWARD){
			vShift = MathHelper::GetInstance( )->Float3PlusFloat3( vShift, MathHelper::GetInstance( )->Float3MulFloat( m_vLook, fDistance * m_fSpeed ) );
		}
		if (dwDirection & DIR_RIGHT){
			XMVECTOR rotatedir = MathHelper::GetInstance( )->Float3ToVector( XMFLOAT3( 0.0f, 1.0f, 0.0f ) )  * fDistance;
			XMFLOAT3 rotateDiretion = MathHelper::GetInstance( )->VectorToFloat3( rotatedir );
			Rotate( rotateDiretion.x, rotateDiretion.y, rotateDiretion.z );
		}
		if (dwDirection & DIR_LEFT){
			XMVECTOR rotatedir = MathHelper::GetInstance( )->Float3ToVector( XMFLOAT3( 0.0f, -1.0f, 0.0f ) ) * fDistance;
			XMFLOAT3 rotateDiretion = MathHelper::GetInstance( )->VectorToFloat3( rotatedir );
			Rotate( rotateDiretion.x, rotateDiretion.y, rotateDiretion.z );
		}
		Move( vShift, bUpdateVelocity );
	}
}

void CEnemyObject::Move( XMFLOAT3& vShift, bool bUpdateVelocity )
{
	if (bUpdateVelocity)
	{
		m_vVelocity = MathHelper::GetInstance( )->Float3PlusFloat3( m_vVelocity, vShift );
	}
	else
	{
		// 현재 위치에서 vshift만큼이동
		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, vShift );
		m_vPosition = vPosition;
	}
}

void CEnemyObject::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
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
	//		m_pCamera->Rotate( x, y, z );

	/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다.
	플레이어의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다.
	기본적으로 Up 벡터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
	if (y != 0.0f)
	{
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, y );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
	}

	/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로
	z-축(LookAt 벡터)을 기준으로 하여 서로 직교하고 단위벡터가 되도록 한다.*/
	m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
	m_vRight = MathHelper::GetInstance( )->CrossFloat3( m_vUp, m_vLook );
	m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
	m_vUp = MathHelper::GetInstance( )->CrossFloat3( m_vLook, m_vRight );
	m_vUp = MathHelper::GetInstance( )->NormalizeFloat( m_vUp );
}

void CEnemyObject::Animate( float fTimeElapsed )
{
	m_fTimePos += fTimeElapsed;
	if (m_bVisible)
		( (CAnimatedMesh *)GetMesh( ) )->GetSkinnedData( )->GetFinalTransforms( m_iAnimState, m_fTimePos, m_pmtxFinalTransforms );

	if (m_fTimePos > ( (CAnimatedMesh *)GetMesh( ) )->GetSkinnedData( )->GetClipEndTime( m_iAnimState ))
		m_fTimePos = 0.0f;

	DWORD dwDirection = 0;
	int dir = rand( ) % 6;
	switch (dir)
	{
		case 0:
			dwDirection = DIR_FORWARD;
			break;
		case 2:
			dwDirection = DIR_LEFT;
			break;
		case 3:
			dwDirection = DIR_RIGHT;
			break;
		case 4:
			dwDirection = DIR_FORWARD | DIR_LEFT;
			break;
		case 1:
			dwDirection = DIR_FORWARD | DIR_RIGHT;
			break;
	}
	if (dwDirection)
	{
		if (this->m_iAnimState == (int)AnimationState::ANIM_IDLE || this->m_iAnimState == (int)AnimationState::ANIM_WALKING)
		{
			if (this->m_iAnimState != (int)AnimationState::ANIM_WALKING)
			{
				this->m_fTimePos = 0.0f;
				this->m_iAnimState = (int)AnimationState::ANIM_WALKING;
			}
			this->Move( dwDirection, 50.0f *fTimeElapsed, false );
		}
	}
}