#include "stdafx.h"
#include "Camera.h"
#include "Player.h"

CCamera::CCamera( )
{
	m_pPlayer = NULL;

	m_mtxView = MathHelper::GetInstance( )->GetMatrixIdentity( );
	m_mtxProjection = MathHelper::GetInstance( )->GetMatrixIdentity( );
	m_mtxOrtho = MathHelper::GetInstance( )->GetMatrixIdentity( );

	m_pd3dcbCamera = NULL;
}

CCamera::CCamera( CCamera *pCamera )
{
	if (pCamera)
	{
		// ī�޶� �ִ� ���
		m_vPosition = pCamera->GetPosition( );
		m_vRight = pCamera->GetRightVector( );
		m_vLook = pCamera->GetLookVector( );
		m_vUp = pCamera->GetUpVector( );
		m_fPitch = pCamera->GetPitch( );
		m_fYaw = pCamera->GetYaw( );
		m_fRoll = pCamera->GetRoll( );
		m_mtxView = pCamera->GetViewMatrix( );
		m_mtxProjection = pCamera->GetProjectionMatrix( );
		m_mtxOrtho = pCamera->GetOrthMatrix( );
		m_d3dViewport = pCamera->GetViewport( );
		m_vLookAtWorld = pCamera->GetLookAtPosition( );
		m_vOffset = pCamera->GetOffset( );
		m_fTimeLag = pCamera->GetTimeLag( );
		m_pPlayer = pCamera->GetPlayer( );
		m_pd3dcbCamera = pCamera->GetCameraConstantBuffer( );
		if (m_pd3dcbCamera)
			m_pd3dcbCamera->AddRef( );
	}
	else
	{
		// ī�޶� ���� ���
		m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		m_vRight = XMFLOAT3( 1.0f, 0.0f, 0.0f );
		m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
		m_vLook = XMFLOAT3( 0.0f, 0.0f, 1.0f );
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_fTimeLag = 0.0f;
		m_vLookAtWorld = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		m_vOffset = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		m_nMode = 0x00;
		m_pPlayer = NULL;
		m_mtxView = MathHelper::GetInstance( )->GetMatrixIdentity( );
		m_mtxProjection = MathHelper::GetInstance( )->GetMatrixIdentity( );
		m_mtxOrtho = MathHelper::GetInstance( )->GetMatrixIdentity( );
		m_pd3dcbCamera = NULL;
	}
}

CCamera::~CCamera( )
{
	if (m_pd3dcbCamera)
		m_pd3dcbCamera->Release( );
}

void CCamera::CalculateFrustumPlanes( )
{
	XMFLOAT4X4 mtxViewProjection = MathHelper::GetInstance( )->Float4x4MulFloat4x4( m_mtxView, m_mtxProjection );

	// ����ü�� ���� ���
	m_FrustumPlanes[0].x = -( mtxViewProjection._14 + mtxViewProjection._11 );
	m_FrustumPlanes[0].y = -( mtxViewProjection._24 + mtxViewProjection._21 );
	m_FrustumPlanes[0].z = -( mtxViewProjection._34 + mtxViewProjection._31 );
	m_FrustumPlanes[0].w = -( mtxViewProjection._44 + mtxViewProjection._41 );
	// ����ü�� ������ ���
	m_FrustumPlanes[1].x = -( mtxViewProjection._14 - mtxViewProjection._11 );
	m_FrustumPlanes[1].y = -( mtxViewProjection._24 - mtxViewProjection._21 );
	m_FrustumPlanes[1].z = -( mtxViewProjection._34 - mtxViewProjection._31 );
	m_FrustumPlanes[1].w = -( mtxViewProjection._44 - mtxViewProjection._41 );
	// ����ü�� ���� ���
	m_FrustumPlanes[2].x = -( mtxViewProjection._14 - mtxViewProjection._12 );
	m_FrustumPlanes[2].y = -( mtxViewProjection._24 - mtxViewProjection._22 );
	m_FrustumPlanes[2].z = -( mtxViewProjection._34 - mtxViewProjection._32 );
	m_FrustumPlanes[2].w = -( mtxViewProjection._44 - mtxViewProjection._42 );
	// ����ü�� �Ʒ��� ���
	m_FrustumPlanes[3].x = -( mtxViewProjection._14 + mtxViewProjection._12 );
	m_FrustumPlanes[3].y = -( mtxViewProjection._24 + mtxViewProjection._22 );
	m_FrustumPlanes[3].z = -( mtxViewProjection._34 + mtxViewProjection._32 );
	m_FrustumPlanes[3].w = -( mtxViewProjection._44 + mtxViewProjection._42 );
	// ����ü�� �����
	m_FrustumPlanes[4].x = -( mtxViewProjection._13 );
	m_FrustumPlanes[4].y = -( mtxViewProjection._23 );
	m_FrustumPlanes[4].z = -( mtxViewProjection._33 );
	m_FrustumPlanes[4].w = -( mtxViewProjection._43 );
	// ����ü�� �����
	m_FrustumPlanes[5].x = -( mtxViewProjection._14 - mtxViewProjection._13 );
	m_FrustumPlanes[5].y = -( mtxViewProjection._24 - mtxViewProjection._23 );
	m_FrustumPlanes[5].z = -( mtxViewProjection._34 - mtxViewProjection._33 );
	m_FrustumPlanes[5].w = -( mtxViewProjection._44 - mtxViewProjection._43 );

	for (int i = 0; i < 6; i++)
	{
		m_FrustumPlanes[i] = MathHelper::GetInstance( )->NormalizeFloat( m_FrustumPlanes[i] );
	}
}

bool CCamera::IsInFrustum( XMFLOAT3& vMin, XMFLOAT3& vMax )
{
	XMFLOAT3 vNearPoint, vFarPoint, vNormal;
	for (int i = 0; i < 6; i++)
	{
		// ����ü�� �� ��鿡 ���Ͽ� �ٿ�� �ڽ��� ������ ���
		// �������� x,y,z ��ǥ�� ���� ������ �� ��Ұ� �����̸� �ٿ�� �ڽ��� �ִ����̰�, �׷��� ������ �ּ����� �ȴ�
		vNormal = XMFLOAT3( m_FrustumPlanes[i].x, m_FrustumPlanes[i].y, m_FrustumPlanes[i].z );
		if (vNormal.x >= 0.0f)
		{
			if (vNormal.y > 0.0f)
			{
				if (vNormal.z >= 0.0f)
				{
					// ���������� x,y,z ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� �ٿ�� �ڽ��� �ּ���
					vNearPoint.x = vMin.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					// ���������� x,y,z ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� x,y��ǥ�� �ٿ�� �ڽ��� �ּ����� x,y ��ǥ�̴�
					//���� ������ z��ǥ�� �����̹Ƿ� �������� z��ǥ�� �ٿ�� �ڽ��� �ִ����� z��ǥ
					vNearPoint.x = vMin.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMax.z;
				}
			}
			else
			{
				if (vNormal.z >= 0.0f)
				{
					// ���������� x,z ��ǥ�� ��ȣ�� ����̹Ƿ� �������� x,z��ǥ�� �ٿ�� �ڽ��� �ּ����� x,z��ǥ
					// ���������� y��ǥ�� �����̹Ƿ� �������� y��ǥ�� �ٿ�� �ڽ��� �ִ����� y ��ǥ
					vNearPoint.x = vMin.x; 
					vNearPoint.y = vMax.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					// ���������� y,z ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� y,z��ǥ�� �ٿ�� �ڽ��� �ִ����� y,z��ǥ
					// ���������� x��ǥ�� ����̹Ƿ� �������� x��ǥ�� �ٿ�� �ڽ��� �ּ����� x ��ǥ
					vNearPoint.x = vMin.x;
					vNearPoint.y = vMax.y;
					vNearPoint.z = vMax.z;
				}
			}
		}
		else
		{
			if (vNormal.y >= 0.0f)
			{
				if (vNormal.z >= 0.0f)
				{
					//���� ������ y, z ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� y, z ��ǥ�� �ٿ�� �ڽ��� �ּ����� y, z ��ǥ
					//���� ������ x ��ǥ�� �����̹Ƿ� �������� x ��ǥ�� �ٿ�� �ڽ��� �ִ����� x ��ǥ
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					//���� ������ x, z ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� x, z ��ǥ�� �ٿ�� �ڽ��� �ִ����� x, z ��ǥ
					//���� ������ y ��ǥ�� ����̹Ƿ� �������� y ��ǥ�� �ٿ�� �ڽ��� �ּ����� y ��ǥ�̴�.
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMax.z;
				}
			}
			else
			{
				if (vNormal.z >= 0.0f)
				{
					//���� ������ x, y ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� x, y ��ǥ�� �ٿ�� �ڽ��� �ִ����� x, y ��ǥ
					//���� ������ z ��ǥ�� ����̹Ƿ� �������� z ��ǥ�� �ٿ�� �ڽ��� �ּ����� z ��ǥ�̴�.
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMax.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					//���� ������ x, y, z ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� �ٿ�� �ڽ��� �ִ����̴�.
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMax.y;
					vNearPoint.z = vMax.z;
				}
			}
		}
		if (MathHelper::GetInstance( )->Float3ToDot( vNormal, vNearPoint ) + m_FrustumPlanes[i].w > 0.0f)
			return false;
	}
	return true;
}

bool CCamera::IsInFrustum( AABB *pAABB )
{
	return IsInFrustum( pAABB->m_vMin, pAABB->m_vMax );
}

void CCamera::SetViewport( ID3D11DeviceContext *pd3dDeviceContext, DWORD xTopLeft, DWORD yTopLeft, DWORD nWidth, DWORD nHeight, float fMinZ, float fMaxZ )
{
	m_d3dViewport.TopLeftX = float( xTopLeft );
	m_d3dViewport.TopLeftY = float( yTopLeft );
	m_d3dViewport.Width = float( nWidth );
	m_d3dViewport.Height = float( nHeight );
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
	pd3dDeviceContext->RSSetViewports( 1, &m_d3dViewport );
}


// ī�޶� ��ȯ����� �����ϴ� �Լ�
// ī�޶��� ��ġ����, ī�޶� �ٶ󺸴� ����, ī�޶��� Up���͸� ����Ͽ� �����
void CCamera::GenerateViewMatrix( )
{
	XMVECTOR EyePosition = XMLoadFloat3( &m_vPosition );
	XMVECTOR FocusPosition = XMLoadFloat3( &m_pPlayer->GetPosition( ) );
	XMVECTOR UpDirection = XMLoadFloat3( &m_vUp );

	XMMATRIX tmp = XMMatrixLookAtLH( EyePosition, FocusPosition, UpDirection );

	XMStoreFloat4x4( &m_mtxView, tmp );
}

// ī�޶��� z���� �������� ī�޶� ��ǥ����� �����ϵ��� ī�޶� ��ȯ����� �����Ѵ�.
void CCamera::RegenerateViewMatrix( )
{
	// ī�޶��� z�� ���͸� ����ȭ
	m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
	// ī�޶��� z��� y�࿡ ������ ���͸� x������ ����
	m_vRight = MathHelper::GetInstance( )->CrossFloat3( m_vUp, m_vLook );
	// ī�޶��� x�� ���͸� ����ȭ
	m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
	// ī�޶��� z��� x�࿡ ������ ���͸� y������ �����Ѵ�.
	m_vUp = MathHelper::GetInstance( )->CrossFloat3( m_vLook, m_vRight );
	// ī�޶��� y�� ���͸� ����ȭ
	m_vUp = MathHelper::GetInstance( )->NormalizeFloat( m_vUp );

	m_mtxView._11 = m_vRight.x; m_mtxView._12 = m_vUp.x; m_mtxView._13 = m_vLook.x;
	m_mtxView._21 = m_vRight.y; m_mtxView._22 = m_vUp.y; m_mtxView._23 = m_vLook.y;
	m_mtxView._31 = m_vRight.z; m_mtxView._32 = m_vUp.z; m_mtxView._33 = m_vLook.z;
	m_mtxView._41 = -( MathHelper::GetInstance( )->Float3ToDot( m_vPosition, m_vRight ) );
	m_mtxView._42 = -( MathHelper::GetInstance( )->Float3ToDot( m_vPosition, m_vUp ) );
	m_mtxView._43 = -( MathHelper::GetInstance( )->Float3ToDot( m_vPosition, m_vLook ) );

	CalculateFrustumPlanes( );
}

void CCamera::GenerateProjectionMatrix( float fNearPlaneDist, float fFarPlaneDist, float fAspectRatio, float fFOVAngle )
{
	XMMATRIX temp = XMMatrixPerspectiveFovLH( (float)XMConvertToRadians( fFOVAngle ), fAspectRatio, fNearPlaneDist, fFarPlaneDist );
	XMStoreFloat4x4( &m_mtxProjection, temp );

	float screenWidth = CAppManager::GetInstance( )->m_pFrameWork->m_nWndClientWidth;
	float screenHeight = CAppManager::GetInstance( )->m_pFrameWork->m_nWndClientHeight;

	temp = XMMatrixOrthographicLH( screenWidth, screenHeight, 0.1f, 1000.0f );
	XMStoreFloat4x4( &m_mtxOrtho, temp );
}

void CCamera::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	D3D11_BUFFER_DESC bd;
	::ZeroMemory( &bd, sizeof( D3D11_BUFFER_DESC ) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( VS_CB_CAMERA );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer( &bd, NULL, &m_pd3dcbCamera );
}

void CCamera::UpdateShaderVariables( ID3D11DeviceContext *pd3dDeviceContext )
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	// ��� ������ �޸� �ּҸ� �����ͼ� ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ����
	// ���̴����� ����� ��� ���� �ٲ�� �Ϳ� ����
	pd3dDeviceContext->Map( m_pd3dcbCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	
	// d3dMappedResource ��� ���� �ּҸ� ������
	VS_CB_CAMERA *pcbViewProjection = (VS_CB_CAMERA*)d3dMappedResource.pData;
	pcbViewProjection->m_mtxView = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxView );
	pcbViewProjection->m_mtxProjection = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxProjection );
	pd3dDeviceContext->Unmap( m_pd3dcbCamera, 0 );

	// ��� ���۸� ���Կ� ����
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_CAMERA, 1, &m_pd3dcbCamera );
}

void CCamera::UpdateShaderVariablesOrtho( ID3D11DeviceContext *pd3dDeviceContext )
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	// ��� ������ �޸� �ּҸ� �����ͼ� ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ����
	// ���̴����� ����� ��� ���� �ٲ�� �Ϳ� ����
	pd3dDeviceContext->Map( m_pd3dcbCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );

	// d3dMappedResource ��� ���� �ּҸ� ������
	VS_CB_CAMERA *pcbViewProjection = (VS_CB_CAMERA*)d3dMappedResource.pData;
	pcbViewProjection->m_mtxView = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxView );
	pcbViewProjection->m_mtxProjection = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxOrtho );
	pd3dDeviceContext->Unmap( m_pd3dcbCamera, 0 );

	// ��� ���۸� ���Կ� ����
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_CAMERA, 1, &m_pd3dcbCamera );
}

CSpaceShipCamera::CSpaceShipCamera( CCamera *pCamera ) : CCamera( pCamera )
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
	
	// �������� �����ϵ�
	if (m_pPlayer && (x !=0.0f))
	{
		// �÷��̾��� ���� x�࿡ ���� x������ ȸ�� ����� ����Ѵ�.
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetRightVector( ), x );
		// ī�޶��� ���� x,y,z���� ȸ���Ѵ�.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
		// ī�޶��� ��ġ ���Ϳ��� �÷��̾��� ��ġ���͸� ����
		// ����� �÷��̾� ��ġ�� �������� �� ī�޶��� ��ġ����
		m_vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
		// �÷��̾��� ��ġ�� �߽����� ī�޶��� ��ġ ���͸� ȸ���Ѵ�
		m_vPosition = MathHelper::GetInstance( )->Vector3TransformCoord( m_vPosition, mtxRotate );
		// ȸ����Ų ī�޶��� ��ġ ���Ϳ� �÷��̾��� ��ġ�� ���Ѵ�.
		m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
	}
	if (m_pPlayer && ( y != 0.0f ) )
	{
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetRightVector( ), y );
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
		m_vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
		m_vPosition = MathHelper::GetInstance( )->Vector3TransformCoord( m_vPosition, mtxRotate );
		m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
	}
	if (m_pPlayer && ( z != 0.0f ))
	{
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetRightVector( ), z );
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
		m_vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
		m_vPosition = MathHelper::GetInstance( )->Vector3TransformCoord( m_vPosition, mtxRotate );
		m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
	}
}

CFirstPersonCamera::CFirstPersonCamera( CCamera *pCamera ) : CCamera( pCamera )
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		/*1��Ī ī�޶�� �����ϱ� ������ ī�޶� �����̽�-�� ī�޶��̸� ī�޶��� Up ���͸� ������ǥ�� y-���� �ǵ��� �Ѵ�.
		�̰��� �����̽�-�� ī�޶��� ���� y-�� ���Ͱ� � �����̵��� 1��Ī ī�޶�(��κ� ����� ���)��
		���� y-�� ���Ͱ� ������ǥ�� y-���� �ǵ��� ��, �ȹٷ� ���ִ� ���·� �����Ѵٴ� �ǹ��̴�.
		�׸��� ���� x-�� ���Ϳ� ���� z-�� ������ y-��ǥ�� 0.0f�� �ǵ��� �Ѵ�.
		�̰��� <�׸� 8>�� ���� ���� x-�� ���Ϳ� ���� z-�� ���͸� xz-���(����)���� �����ϴ� ���� �ǹ��Ѵ�.
		��, 1��Ī ī�޶��� ���� x-�� ���Ϳ� ���� z-�� ���ʹ� xz-��鿡 �����ϴ�.*/
		if (pCamera->GetMode( ) == SPACESHIP_CAMERA)
		{
			m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
			m_vRight.y = 0.0f;
			m_vLook.y = 0.0f;
			m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
			m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
		}
	}
}

void CFirstPersonCamera::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
	if (m_pPlayer && x != 0.0f)
	{
		// ī�޶��� ���� x ���� �������� ȸ���ϴ� ����� ����
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vRight, x );
		// ī�޶��� ���� x,y,z���� ȸ���Ѵ�.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
	}
	if (m_pPlayer && y != 0.0f)
	{
		// �÷��̾��� ���� y���� �������� ȸ���ϴ� ����� ����
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetUpVector(), y );
		// ī�޶��� ���� x,y,z���� ȸ���Ѵ�.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
	}
	if (m_pPlayer && z != 0.0f)
	{
		// �÷��̾��� ���� z���� �������� ȸ���ϴ� ����� ����
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetLookVector(), z );
		// ī�޶��� ��ġ ���͸� �÷��̾� ��ǥ��� ǥ��
		m_vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
		// ������ ���͸� ȸ��
		m_vPosition = MathHelper::GetInstance( )->Vector3TransformCoord( m_vPosition, mtxRotate );
		// ī�޶��� ���� x,y,z���� ȸ���Ѵ�.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
	}
}

CThirdPersonCamera::CThirdPersonCamera( CCamera *pCamera ) : CCamera( pCamera )
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		/*3��Ī ī�޶�� �����ϱ� ������ ī�޶� �����̽�-�� ī�޶��̸� ī�޶��� Up ���͸� ������ǥ�� y-���� �ǵ��� �Ѵ�.
		�̰��� �����̽�-�� ī�޶��� ���� y-�� ���Ͱ� � �����̵��� 3��Ī ī�޶�(��κ� ����� ���)�� ���� y-�� ���Ͱ� ������ǥ�� y-���� �ǵ��� ��, �ȹٷ� ���ִ� ���·� �����Ѵٴ� �ǹ��̴�.
		�׸��� ���� x-�� ���Ϳ� ���� z-�� ������ y-��ǥ�� 0.0f�� �ǵ��� �Ѵ�.
		�̰��� ���� x-�� ���Ϳ� ���� z-�� ���͸� xz-���(����)���� �����ϴ� ���� �ǹ��Ѵ�.
		��, 3��Ī ī�޶��� ���� x-�� ���Ϳ� ���� z-�� ���ʹ� xz-��鿡 �����ϴ�.*/
		if (pCamera->GetMode( ) == SPACESHIP_CAMERA)
		{
			m_vUp = XMFLOAT3( 0.0f, 1.0f, 0.0f );
			m_vRight.y = 0.0f;
			m_vLook.y = 0.0f;
			m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
			m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
		}
	}
}

void CThirdPersonCamera::Update( XMFLOAT3& vLookAt, float fTimeElapsed )
{
	// �÷��̾��� ȸ���� ���� 3��Ī ī�޶� ȸ���ؾ� �Ѵ�.
	if (m_pPlayer)
	{
		XMFLOAT4X4 mtxRotate;
		mtxRotate = MathHelper::GetInstance( )->GetMatrixIdentity( );
		XMFLOAT3 vRight = m_pPlayer->GetRightVector( );
		XMFLOAT3 vUp = m_pPlayer->GetUpVector();
		XMFLOAT3 vLook = m_pPlayer->GetLookVector();

		// �÷��̾��� ���� x,y,z�� ���ͷκ��� ȸ������� ����
		mtxRotate._11 = vRight.x; mtxRotate._21 = vUp.x; mtxRotate._31 = vLook.x;
		mtxRotate._12 = vRight.y; mtxRotate._22 = vUp.y; mtxRotate._32 = vLook.y;
		mtxRotate._13 = vRight.z; mtxRotate._23 = vUp.z; mtxRotate._33 = vLook.z;
		
		XMFLOAT3 vOffset;
		vOffset = MathHelper::GetInstance( )->Vector3TransformCoord( m_vOffset, mtxRotate );
		// ȸ���� ī�޶��� ��ġ�� �÷��̾��� ��ġ�� ȸ���� ī�޶� ������ ���͸� ���� ��
		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_pPlayer->GetPosition( ), vOffset );
//		m_vOffset = vOffset;
//		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_pPlayer->GetPosition( ), m_vOffset );
//		m_vPosition = vPosition;
		//// ������ ī�޶��� ��ġ���� ȸ���� ī�޶��� ��ġ������ ����
		XMFLOAT3 vDirection = MathHelper::GetInstance( )->Float3MinusFloat3( vPosition, m_vPosition );
		float fLength = MathHelper::GetInstance( )->Float3ToLength( vDirection );
		vDirection = MathHelper::GetInstance( )->NormalizeFloat( vDirection );
		// 3��Ī ī�޶��� ���״� �÷��̾ ȸ���ϴ��� ī�޶� ���ÿ� ���� ȸ������ �ʰ� �ణ�� ������ �ΰ� ȸ���ϴ� ȿ���� �����ϱ� ���Ѱ�
		// m_fTimeLag�� 1���� ũ�� fTimeLagScale�� �۾����� ���� ȸ���� ���� �Ͼ ���̴�.
		float fTimeLagScale = ( m_fTimeLag ) ? fTimeElapsed * ( 1.0f / m_fTimeLag ) : 1.0f;
		float fDistance = fLength * fTimeLagScale;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance >= 0)
		{
			m_vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_vPosition, MathHelper::GetInstance()->Float3MulFloat( vDirection, fDistance) );
			SetLookAt( vLookAt, m_pPlayer->GetUpVector( ) );
		}
	}
}

void CThirdPersonCamera::SetLookAt( XMFLOAT3& vLookAt, XMFLOAT3 vUp )
{
	XMFLOAT4X4 mtxLookAt = MathHelper::GetInstance( )->MatrixLookAtLH( m_vPosition, vLookAt, vUp );

	m_vRight = XMFLOAT3( mtxLookAt._11, mtxLookAt._21, mtxLookAt._31 );
	m_vUp = XMFLOAT3( mtxLookAt._12, mtxLookAt._22, mtxLookAt._32 );
	m_vLook = XMFLOAT3( mtxLookAt._13, mtxLookAt._23, mtxLookAt._33 );
}

void CThirdPersonCamera::Rotate( float fPitch, float fYaw, float fRoll )
{
	XMFLOAT4X4 mtxRotate;
	m_nMode;

	/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�.
	�׷��Ƿ� x-���� �߽����� ȸ���ϴ� ������ -89.0~+89.0�� ���̷� �����Ѵ�.
	x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ���� Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
	if (fPitch != 0.0f)
	{
		m_fPitch += fPitch;
		if (m_fPitch > +89.0f) { fPitch -= ( m_fPitch - 89.0f ); m_fPitch = 89.0f; }
		if (m_fPitch < -89.0f) { fPitch -= ( m_fPitch + 89.0f ); m_fPitch = -89.0f; }
	}
	// ���� y���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����
	if (fYaw != 0.0f)
	{
		m_fYaw += fYaw;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵȴ�.
	z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������ ������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
	if (fRoll != 0.0f)
	{
		m_fRoll += fRoll;
		if (m_fRoll > 20.0f) { fRoll -= ( m_fRoll - 20.0f ); m_fRoll = 20.0f; }
		if (m_fRoll < -20.0f) { fRoll -= ( m_fRoll + 20.0f ); m_fRoll = -20.0f; }
	}
	/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��.
	�÷��̾��� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��.
	�⺻������ Up ���͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
	if (fYaw != 0.0f)
	{
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, fYaw );
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