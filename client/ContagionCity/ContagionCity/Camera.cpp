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
		// 카메라가 있는 경우
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
		// 카메라가 없는 경우
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

	// 절두체의 왼쪽 평면
	m_FrustumPlanes[0].x = -( mtxViewProjection._14 + mtxViewProjection._11 );
	m_FrustumPlanes[0].y = -( mtxViewProjection._24 + mtxViewProjection._21 );
	m_FrustumPlanes[0].z = -( mtxViewProjection._34 + mtxViewProjection._31 );
	m_FrustumPlanes[0].w = -( mtxViewProjection._44 + mtxViewProjection._41 );
	// 절두체의 오른쪽 평면
	m_FrustumPlanes[1].x = -( mtxViewProjection._14 - mtxViewProjection._11 );
	m_FrustumPlanes[1].y = -( mtxViewProjection._24 - mtxViewProjection._21 );
	m_FrustumPlanes[1].z = -( mtxViewProjection._34 - mtxViewProjection._31 );
	m_FrustumPlanes[1].w = -( mtxViewProjection._44 - mtxViewProjection._41 );
	// 절두체의 위쪽 평면
	m_FrustumPlanes[2].x = -( mtxViewProjection._14 - mtxViewProjection._12 );
	m_FrustumPlanes[2].y = -( mtxViewProjection._24 - mtxViewProjection._22 );
	m_FrustumPlanes[2].z = -( mtxViewProjection._34 - mtxViewProjection._32 );
	m_FrustumPlanes[2].w = -( mtxViewProjection._44 - mtxViewProjection._42 );
	// 절두체의 아래쪽 평면
	m_FrustumPlanes[3].x = -( mtxViewProjection._14 + mtxViewProjection._12 );
	m_FrustumPlanes[3].y = -( mtxViewProjection._24 + mtxViewProjection._22 );
	m_FrustumPlanes[3].z = -( mtxViewProjection._34 + mtxViewProjection._32 );
	m_FrustumPlanes[3].w = -( mtxViewProjection._44 + mtxViewProjection._42 );
	// 절두체의 근평면
	m_FrustumPlanes[4].x = -( mtxViewProjection._13 );
	m_FrustumPlanes[4].y = -( mtxViewProjection._23 );
	m_FrustumPlanes[4].z = -( mtxViewProjection._33 );
	m_FrustumPlanes[4].w = -( mtxViewProjection._43 );
	// 절두체의 원평면
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
		// 절두체의 각 평면에 대하여 바운딩 박스의 근접점 계산
		// 근점접의 x,y,z 좌표는 법선 벡터의 각 요소가 음수이면 바운딩 박스의 최대점이고, 그렇지 않으면 최소점이 된다
		vNormal = XMFLOAT3( m_FrustumPlanes[i].x, m_FrustumPlanes[i].y, m_FrustumPlanes[i].z );
		if (vNormal.x >= 0.0f)
		{
			if (vNormal.y > 0.0f)
			{
				if (vNormal.z >= 0.0f)
				{
					// 법선벡터의 x,y,z 좌표의 부호가 모두 양수이므로 근접점은 바운딩 박스의 최소점
					vNearPoint.x = vMin.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					// 법선벡터의 x,y,z 좌표의 부호가 모두 양수이므로 근접점의 x,y좌표는 바운딩 박스의 최소점의 x,y 좌표이다
					//법선 벡터의 z좌표가 음수이므로 근접점의 z좌표는 바운딩 박스의 최대점의 z좌표
					vNearPoint.x = vMin.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMax.z;
				}
			}
			else
			{
				if (vNormal.z >= 0.0f)
				{
					// 법선벡터의 x,z 좌표의 부호가 양수이므로 근접점의 x,z좌표는 바운딩 박스의 최소점의 x,z좌표
					// 법선벡터의 y좌표가 음수이므로 근접점의 y좌표는 바운딩 박스의 최대점의 y 좌표
					vNearPoint.x = vMin.x; 
					vNearPoint.y = vMax.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					// 법선벡터의 y,z 좌표의 부호가 모두 음수이므로 근접점의 y,z좌표는 바운딩 박스의 최대점의 y,z좌표
					// 법선벡터의 x좌표가 양수이므로 근접점의 x좌표는 바운딩 박스의 최소점의 x 좌표
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
					//법선 벡터의 y, z 좌표의 부호가 모두 양수이므로 근접점의 y, z 좌표는 바운딩 박스의 최소점의 y, z 좌표
					//법선 벡터의 x 좌표가 음수이므로 근접점의 x 좌표는 바운딩 박스의 최대점의 x 좌표
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					//법선 벡터의 x, z 좌표의 부호가 모두 음수이므로 근접점의 x, z 좌표는 바운딩 박스의 최대점의 x, z 좌표
					//법선 벡터의 y 좌표가 양수이므로 근접점의 y 좌표는 바운딩 박스의 최소점의 y 좌표이다.
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMin.y;
					vNearPoint.z = vMax.z;
				}
			}
			else
			{
				if (vNormal.z >= 0.0f)
				{
					//법선 벡터의 x, y 좌표의 부호가 모두 음수이므로 근접점의 x, y 좌표는 바운딩 박스의 최대점의 x, y 좌표
					//법선 벡터의 z 좌표가 양수이므로 근접점의 z 좌표는 바운딩 박스의 최소점의 z 좌표이다.
					vNearPoint.x = vMax.x;
					vNearPoint.y = vMax.y;
					vNearPoint.z = vMin.z;
				}
				else
				{
					//법선 벡터의 x, y, z 좌표의 부호가 모두 음수이므로 근접점은 바운딩 박스의 최대점이다.
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


// 카메라 변환행렬을 생성하는 함수
// 카메라의 위치벡터, 카메라가 바라보는 지점, 카메라의 Up벡터를 사용하여 만든다
void CCamera::GenerateViewMatrix( )
{
	XMVECTOR EyePosition = XMLoadFloat3( &m_vPosition );
	XMVECTOR FocusPosition = XMLoadFloat3( &m_pPlayer->GetPosition( ) );
	XMVECTOR UpDirection = XMLoadFloat3( &m_vUp );

	XMMATRIX tmp = XMMatrixLookAtLH( EyePosition, FocusPosition, UpDirection );

	XMStoreFloat4x4( &m_mtxView, tmp );
}

// 카메라의 z축을 기준으로 카메라 좌표축들이 직교하도록 카메라 변환행렬을 갱신한다.
void CCamera::RegenerateViewMatrix( )
{
	// 카메라의 z축 벡터를 정규화
	m_vLook = MathHelper::GetInstance( )->NormalizeFloat( m_vLook );
	// 카메라의 z축과 y축에 수직인 벡터를 x축으로 설정
	m_vRight = MathHelper::GetInstance( )->CrossFloat3( m_vUp, m_vLook );
	// 카메라의 x축 벡터를 정규화
	m_vRight = MathHelper::GetInstance( )->NormalizeFloat( m_vRight );
	// 카메라의 z축과 x축에 수직인 벡터를 y축으로 설정한다.
	m_vUp = MathHelper::GetInstance( )->CrossFloat3( m_vLook, m_vRight );
	// 카메라의 y축 벡터를 정규화
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
	// 상수 버퍼의 메모리 주소를 가져와서 카메라 변환 행렬과 투영 변환 행렬을 복사
	// 셰이더에서 행렬의 행과 열이 바뀌는 것에 주의
	pd3dDeviceContext->Map( m_pd3dcbCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	
	// d3dMappedResource 상수 버퍼 주소를 가져옴
	VS_CB_CAMERA *pcbViewProjection = (VS_CB_CAMERA*)d3dMappedResource.pData;
	pcbViewProjection->m_mtxView = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxView );
	pcbViewProjection->m_mtxProjection = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxProjection );
	pd3dDeviceContext->Unmap( m_pd3dcbCamera, 0 );

	// 상수 버퍼를 슬롯에 설정
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_CAMERA, 1, &m_pd3dcbCamera );
}

void CCamera::UpdateShaderVariablesOrtho( ID3D11DeviceContext *pd3dDeviceContext )
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	// 상수 버퍼의 메모리 주소를 가져와서 카메라 변환 행렬과 투영 변환 행렬을 복사
	// 셰이더에서 행렬의 행과 열이 바뀌는 것에 주의
	pd3dDeviceContext->Map( m_pd3dcbCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );

	// d3dMappedResource 상수 버퍼 주소를 가져옴
	VS_CB_CAMERA *pcbViewProjection = (VS_CB_CAMERA*)d3dMappedResource.pData;
	pcbViewProjection->m_mtxView = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxView );
	pcbViewProjection->m_mtxProjection = MathHelper::GetInstance( )->TransposeFloat4x4( m_mtxOrtho );
	pd3dDeviceContext->Unmap( m_pd3dcbCamera, 0 );

	// 상수 버퍼를 슬롯에 설정
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_CAMERA, 1, &m_pd3dcbCamera );
}

CSpaceShipCamera::CSpaceShipCamera( CCamera *pCamera ) : CCamera( pCamera )
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate( float x, float y, float z )
{
	XMFLOAT4X4 mtxRotate;
	
	// 에러나면 여기일듯
	if (m_pPlayer && (x !=0.0f))
	{
		// 플레이어의 로컬 x축에 대한 x각도의 회전 행렬을 계산한다.
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetRightVector( ), x );
		// 카메라의 로컬 x,y,z축을 회전한다.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
		// 카메라의 위치 벡터에서 플레이어의 위치벡터를 뺀다
		// 결과는 플레이어 위치를 기준으로 한 카메라의 위치벡터
		m_vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
		// 플레이어의 위치를 중심으로 카메라의 위치 벡터를 회전한다
		m_vPosition = MathHelper::GetInstance( )->Vector3TransformCoord( m_vPosition, mtxRotate );
		// 회전시킨 카메라의 위치 벡터에 플레이어의 위치를 더한다.
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
		/*1인칭 카메라로 변경하기 이전의 카메라가 스페이스-쉽 카메라이면 카메라의 Up 벡터를 월드좌표의 y-축이 되도록 한다.
		이것은 스페이스-쉽 카메라의 로컬 y-축 벡터가 어떤 방향이든지 1인칭 카메라(대부분 사람인 경우)의
		로컬 y-축 벡터가 월드좌표의 y-축이 되도록 즉, 똑바로 서있는 형태로 설정한다는 의미이다.
		그리고 로컬 x-축 벡터와 로컬 z-축 벡터의 y-좌표가 0.0f가 되도록 한다.
		이것은 <그림 8>과 같이 로컬 x-축 벡터와 로컬 z-축 벡터를 xz-평면(지면)으로 투영하는 것을 의미한다.
		즉, 1인칭 카메라의 로컬 x-축 벡터와 로컬 z-축 벡터는 xz-평면에 평행하다.*/
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
		// 카메라의 로컬 x 축을 기준으로 회전하는 행렬을 생성
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vRight, x );
		// 카메라의 로컬 x,y,z축을 회전한다.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
	}
	if (m_pPlayer && y != 0.0f)
	{
		// 플레이어의 로컬 y축을 기준으로 회전하는 행렬을 생성
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetUpVector(), y );
		// 카메라의 로컬 x,y,z축을 회전한다.
		m_vRight = MathHelper::GetInstance( )->Vector3TransformNormal( m_vRight, mtxRotate );
		m_vUp = MathHelper::GetInstance( )->Vector3TransformNormal( m_vUp, mtxRotate );
		m_vLook = MathHelper::GetInstance( )->Vector3TransformNormal( m_vLook, mtxRotate );
	}
	if (m_pPlayer && z != 0.0f)
	{
		// 플레이어의 로컬 z축을 기준으로 회전하는 행렬을 생성
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_pPlayer->GetLookVector(), z );
		// 카메라의 위치 벡터를 플레이어 좌표계로 표현
		m_vPosition = MathHelper::GetInstance( )->Float3MinusFloat3( m_vPosition, m_pPlayer->GetPosition( ) );
		// 오프셋 벡터를 회전
		m_vPosition = MathHelper::GetInstance( )->Vector3TransformCoord( m_vPosition, mtxRotate );
		// 카메라의 로컬 x,y,z축을 회전한다.
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
		/*3인칭 카메라로 변경하기 이전의 카메라가 스페이스-쉽 카메라이면 카메라의 Up 벡터를 월드좌표의 y-축이 되도록 한다.
		이것은 스페이스-쉽 카메라의 로컬 y-축 벡터가 어떤 방향이든지 3인칭 카메라(대부분 사람인 경우)의 로컬 y-축 벡터가 월드좌표의 y-축이 되도록 즉, 똑바로 서있는 형태로 설정한다는 의미이다.
		그리고 로컬 x-축 벡터와 로컬 z-축 벡터의 y-좌표가 0.0f가 되도록 한다.
		이것은 로컬 x-축 벡터와 로컬 z-축 벡터를 xz-평면(지면)으로 투영하는 것을 의미한다.
		즉, 3인칭 카메라의 로컬 x-축 벡터와 로컬 z-축 벡터는 xz-평면에 평행하다.*/
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
	// 플레이어의 회전에 따라 3인칭 카메라도 회전해야 한다.
	if (m_pPlayer)
	{
		XMFLOAT4X4 mtxRotate;
		mtxRotate = MathHelper::GetInstance( )->GetMatrixIdentity( );
		XMFLOAT3 vRight = m_pPlayer->GetRightVector( );
		XMFLOAT3 vUp = m_pPlayer->GetUpVector();
		XMFLOAT3 vLook = m_pPlayer->GetLookVector();

		// 플레이어의 로컬 x,y,z축 벡터로부터 회전행렬을 생성
		mtxRotate._11 = vRight.x; mtxRotate._21 = vUp.x; mtxRotate._31 = vLook.x;
		mtxRotate._12 = vRight.y; mtxRotate._22 = vUp.y; mtxRotate._32 = vLook.y;
		mtxRotate._13 = vRight.z; mtxRotate._23 = vUp.z; mtxRotate._33 = vLook.z;
		
		XMFLOAT3 vOffset;
		vOffset = MathHelper::GetInstance( )->Vector3TransformCoord( m_vOffset, mtxRotate );
		// 회전한 카메라의 위치는 플레이어의 위치에 회전한 카메라 오프셋 벡터를 더한 것
		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_pPlayer->GetPosition( ), vOffset );
//		m_vOffset = vOffset;
//		XMFLOAT3 vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( m_pPlayer->GetPosition( ), m_vOffset );
//		m_vPosition = vPosition;
		//// 현재의 카메라의 위치에서 회전한 카메라의 위치까지의 벡터
		XMFLOAT3 vDirection = MathHelper::GetInstance( )->Float3MinusFloat3( vPosition, m_vPosition );
		float fLength = MathHelper::GetInstance( )->Float3ToLength( vDirection );
		vDirection = MathHelper::GetInstance( )->NormalizeFloat( vDirection );
		// 3인칭 카메라의 래그는 플레이어가 회전하더라도 카메라가 동시에 따라서 회전하지 않고 약간의 시차를 두고 회전하는 효과를 구현하기 위한것
		// m_fTimeLag가 1보다 크면 fTimeLagScale이 작아지고 실제 회전이 적게 일어날 것이다.
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

	/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다.
	그러므로 x-축을 중심으로 회전하는 각도는 -89.0~+89.0도 사이로 제한한다.
	x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음 Pitch가 +89도 보다 크거나 -89도 보다 작으면 m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
	if (fPitch != 0.0f)
	{
		m_fPitch += fPitch;
		if (m_fPitch > +89.0f) { fPitch -= ( m_fPitch - 89.0f ); m_fPitch = 89.0f; }
		if (m_fPitch < -89.0f) { fPitch -= ( m_fPitch + 89.0f ); m_fPitch = -89.0f; }
	}
	// 로컬 y축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없음
	if (fYaw != 0.0f)
	{
		m_fYaw += fYaw;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된다.
	z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다 작으면 m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
	if (fRoll != 0.0f)
	{
		m_fRoll += fRoll;
		if (m_fRoll > 20.0f) { fRoll -= ( m_fRoll - 20.0f ); m_fRoll = 20.0f; }
		if (m_fRoll < -20.0f) { fRoll -= ( m_fRoll + 20.0f ); m_fRoll = -20.0f; }
	}
	/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다.
	플레이어의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다.
	기본적으로 Up 벡터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
	if (fYaw != 0.0f)
	{
		mtxRotate = MathHelper::GetInstance( )->MatrixRotationAxis( m_vUp, fYaw );
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