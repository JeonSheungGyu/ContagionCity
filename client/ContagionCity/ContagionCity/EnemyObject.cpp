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
			// ���� �÷��̾ ���� �ִ� ����� XMFLOAT3(0.0f, 0.0f, -1.0f) �� ���Ͽ� ������ ������
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
		// ���� ��ġ���� vshift��ŭ�̵�
		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, vShift );
		m_vPosition = vPosition;
	}
}

void CEnemyObject::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
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
	//		m_pCamera->Rotate( x, y, z );

	/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��.
	�÷��̾��� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��.
	�⺻������ Up ���͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
	if (y != 0.0f)
	{
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, y );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
	}

	/*ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ�
	z-��(LookAt ����)�� �������� �Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.*/
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