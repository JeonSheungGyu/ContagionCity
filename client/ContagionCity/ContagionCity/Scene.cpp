#include "stdafx.h"
#include "Scene.h"

CScene::CScene( )
{
	m_ppShaders = NULL;
	m_nShaders = 0;

	m_pCamera = NULL;

	m_pLights = NULL;
	m_pd3dcbLights = NULL;
	vPickPos = XMFLOAT3( 0.0f, 0.0f, 0.0f );
}

CScene::~CScene( )
{

}

void CScene::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	m_pLights = new LIGHTS;
	::ZeroMemory( m_pLights, sizeof( LIGHTS ) );
	// 월드 전체를 비추는 주변 조명을 설정
	m_pLights->m_cGlobalAmbient = D3DXCOLOR( 0.1f, 0.1f, 0.1f, 1.0f );
	m_pLights->m_FogColor = D3DXCOLOR( 0.7f, 0.7f, 0.7f, 1.0f );
	m_pLights->m_FogStart = 100.0;
	m_pLights->m_FogRange = 200.0;

	// 태양
	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[0].m_cAmbient = D3DXCOLOR( 0.2f, 0.2f, 0.2f, 1.0f );
	m_pLights->m_pLights[0].m_cDiffuse = D3DXCOLOR( 0.4f, 0.4f, 0.4f, 1.0f );
	m_pLights->m_pLights[0].m_cSpecular = D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.0f );
	m_pLights->m_pLights[0].m_vDirection = XMFLOAT3( 0.0f, -1.0f, 0.0f );

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( d3dBufferDesc ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof( LIGHTS );
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pLights;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights );
}

void CScene::ReleaseShaderVariables( )
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release( );
}

void CScene::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights )
{
	XMFLOAT3 temp = m_pCamera->GetPosition( );
	pLights->m_vCameraPosition = MathHelper::GetInstance( )->MakeFloat4( temp.x, temp.y, temp.z, 0.0f );
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	LIGHTS *pcbLights = (LIGHTS *)d3dMappedResource.pData;
	memcpy( pcbLights, pLights, sizeof( LIGHTS ) );
	pd3dDeviceContext->Unmap( m_pd3dcbLights, 0 );
	pd3dDeviceContext->PSSetConstantBuffers( PS_SLOT_LIGHT, 1, &m_pd3dcbLights );
}

void CScene::BuildObjects( ID3D11Device *pd3dDevice )
{
	// 사운드 로딩 및 출력
	LoadingSoundResource( );
	SoundManager::GetInstance( )->Play( BGM );

	// 텍스처 맵핑에 사용할 샘플러 상태 객체를 생성
	ID3D11SamplerState *pd3dSamplerState = NULL;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory( &d3dSamplerDesc, sizeof( D3D11_SAMPLER_DESC ) );
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState( &d3dSamplerDesc, &pd3dSamplerState );

	// fbx loader를 통한 fbx로딩
	LoadFBXs( );
	std::vector<CFbxMesh> background;
	std::vector<CFbxMesh> enemy;
	std::vector<CFbxMesh> npc;

	for (int i = 0; i < m_nFbxCount; i++)
	{
		CFbxMesh tempMesh = FBXManager::GetInstance( )->m_pMeshes[i];
		int layer = tempMesh.m_iLayer;
		switch (layer)
		{
			case ObjectLayer::LAYER_BACKGROUND:	
			{
				background.push_back( tempMesh );
				break;
			}
			case ObjectLayer::LAYER_ENEMY:
			{
				enemy.push_back( tempMesh );
				break;
			}
			case ObjectLayer::LAYER_NPC:
			{
				npc.push_back( tempMesh );
				break;
			}
			default:
				break;
		}
	}
	FBXManager::GetInstance( )->ClearMeshes( );

	// 현재 셰이더 개수에 fbx개수를 더한 뒤 셰이더를 생성하여 만듬
	m_nShaders = 2;		// 0은 스카이박스 , 1은 배경, 2는 엔피시, 3은 적 
	m_ppShaders = new CShader*[m_nShaders];

	// 첫번째로 그릴 객체는 스카이박스
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	// 두번째는 배경
	m_ppShaders[1] = new CBackgroundShader( );
	m_ppShaders[1]->CreateShader( pd3dDevice );
	( (CBackgroundShader*)m_ppShaders[1] )->BuildObjects( pd3dDevice, background );



	
	CreateShaderVariables( pd3dDevice );
}

void CScene::LoadFBXs( )
{
	// fbx 파일 로딩
	FBXManager::GetInstance( )->LoadFBX( "res/City_Base_0225.FBX", LAYER_BACKGROUND, BACK_GROUND, 2, _T( "./res/city_base_0314_texture.dds" ), _T( "./res/city_base_0314_normal.dds" ) );
//u	FBXManager::GetInstance( )->LoadFBX( "res/city_wall_0411.FBX", LAYER_BACKGROUND, BACK_FENCE, 2, _T( "./res/city_base_0314_texture.dds" ), _T( "./res/city_base_0314_normal.dds" ) );

	m_nFbxCount = FBXManager::GetInstance( )->m_pMeshes.size( );
}

void CScene::ReleaseObjects( )
{
	ReleaseShaderVariables( );

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i]) m_ppShaders[i]->ReleaseObject( );
		if (m_ppShaders[i]) delete m_ppShaders[i];
	}
	if (m_ppShaders) delete[ ] m_ppShaders;
}

bool CScene::OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_LBUTTONDOWN:
			SetCapture( hWnd );
			GetCursorPos( &m_ptOldCursorPos );
			break;
		case WM_RBUTTONDOWN:
			SetCapture( hWnd );
			GetCursorPos( &m_ptOldCursorPos );
			Picking( LOWORD( lParam ), HIWORD( lParam ) );
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture( );
			break;
		case WM_MOUSEMOVE:
			break;
		default:
			break;
	}

	return false;
}

bool CScene::OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	return false;
}

bool CScene::ProcessInput( HWND hWnd, CGameTimer timer )
{
	static UCHAR pKeyBuffer[256];
	DWORD dwDirection = 0;
	if (GetKeyboardState( pKeyBuffer ))
	{
		if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
	}
	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다.
	마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의 메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다.
	그러므로 마우스가 캡쳐된 것은 마우스 버튼이 눌려진 상태를 의미한다.
	마우스를 좌우 또는 상하로 움직이면 플레이어를 x-축 또는 y-축으로 회전한다.*/
	if (GetCapture( ) == hWnd)
	{
		SetCursor( NULL );
		GetCursorPos( &ptCursorPos );
		cxDelta = (float)( ptCursorPos.x - m_ptOldCursorPos.x ) / 5.0f;
		cyDelta = (float)( ptCursorPos.y - m_ptOldCursorPos.y ) / 50.0f;
		SetCursorPos( m_ptOldCursorPos.x, m_ptOldCursorPos.y );
	}
	if (dwDirection != 0 || cxDelta != 0.0f || cyDelta != 0.0f)
	{
		if (cxDelta || cyDelta)
		{
			/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다.
			오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
			// 마우스 클릭시 공격 애니메이션이 행해지도록 해야함
			if (pKeyBuffer[VK_RBUTTON] & 0xF0)
				m_pPlayer->Rotate( cyDelta, 0.0f, -cxDelta );
			else
				m_pPlayer->Rotate( cyDelta, cxDelta, 0.0f );

			//if (pKeyBuffer[VK_LBUTTON] & 0xF0)
			//{
			//	//m_pPlayer->m_iAnimState = static_cast<int>( AnimationState::ANIM_LATTACK1 );
			//	SoundManager::GetInstance( )->Play( ATTACK );
			//}
		}
		/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다).
		이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다.
		만약 플레이어의 이동 속력이 있다면 그 값을 사용한다.*/
		if (dwDirection)
		{
			// 현재 플레이어의 AABB 박스의 y좌표 최소가 -0.5임. 따라서 0보다 작으므로 바닥과 겹침, 그로 인해 못움직임
			// 충돌체크 자체는 제대로 되고 있으나 플레이어의 위치가 문제
			if (!CollisionCheck( ))
				m_pPlayer->Move( dwDirection, 100.0f * timer.GetTimeElapsed( ), false );
		}
	}
	//	if (!CollisionCheck( ))
	m_pPlayer->Update( timer.GetTimeElapsed( ), vPickPos );

	return true;
}

void CScene::AnimateObjects( float fTimeElapsed )
{
	if (m_pLights && m_pd3dcbLights)
	{
		// 현재 카메라의 위치 벡터를 조명을 나타내는 상수 버퍼에 설정
		XMFLOAT3 vCameraPosition = m_pCamera->GetPosition( );
		m_pLights->m_vCameraPosition = MathHelper::GetInstance( )->MakeFloat4( vCameraPosition, 1.0f );
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects( fTimeElapsed );
	}
}

//카메라에 보이는 객체들만 렌더링할 수 있도록 변경할 것이기 때문이다(이것을 카메라 절두체 컬링(Frustum Culling)이라고 한다).
void CScene::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	if (m_pLights && m_pd3dcbLights)
		UpdateShaderVariable( pd3dDeviceContext, m_pLights );

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render( pd3dDeviceContext, pCamera );
	}
}

bool CScene::Picking( int x, int y )
{
	// 2차원 점의 투영 공간으로의 변환
	XMMATRIX p = XMLoadFloat4x4( &( m_pCamera->GetProjectionMatrix( ) ) );
	D3D11_VIEWPORT d3dViewport = m_pCamera->GetViewport( );

	XMFLOAT3 vPickPosition;
	vPickPosition.x = ( ( ( 2.0f * ( x - d3dViewport.TopLeftX ) ) / d3dViewport.Width ) - 1 ) / p( 0, 0 );
	vPickPosition.y = -( ( ( 2.0f * ( y - d3dViewport.TopLeftY ) ) / d3dViewport.Height ) - 1 ) / p( 1, 1 );

	XMVECTOR rayOrigin = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
	XMVECTOR rayDir = XMVectorSet( vPickPosition.x, vPickPosition.y, 1.0, 0.0f );

	// 2차원 점의 시야공간으로의 변환
	XMMATRIX v = XMLoadFloat4x4( &( m_pCamera->GetViewMatrix( ) ) );
	XMMATRIX invView = XMMatrixInverse( &XMMatrixDeterminant( v ), v );		// determinant = 행렬식

	// 각 물체와 충돌체크
	float fHitDist = FLT_MAX, fNearDist = FLT_MAX;
	XMFLOAT3 vHitPos, vNearPos;
	bool bIntersection = false;
	CGameObject *IntersectionObject = NULL;		// 충돌된 오브젝트 정보를 가짐
	// 여러개의 셰이더를 돌면서 검사함
	for (int i = 1; i < m_nShaders; i++)		// 0번째 셰이더는 스카이박스만 그리므고 검사할 필요가 없음
	{
		for (int j = 0; j < m_ppShaders[i]->getObjectCount( ); j++)
		{
			CGameObject* tempObj = m_ppShaders[i]->getObjects( )[j];

			XMMATRIX w = XMLoadFloat4x4( &( tempObj->m_mtxWorld ) );
			XMMATRIX invWorld = XMMatrixInverse( &XMMatrixDeterminant( w ), w );

			XMMATRIX toLocal = XMMatrixMultiply( invView, invWorld );

			rayOrigin = XMVector3TransformCoord( rayOrigin, toLocal );	// XMVector3TransformCoord 함수는 벡터의 4번째 성분이 1이라고 가정하고 계산, 따라서 점을 변환할 때 사용
			rayDir = XMVector3TransformNormal( rayDir, toLocal );			// XMVector3TransformNormal 함수는 벡터의 4번째 성분이 0이라고 가정하고 계산, 따라서 벡터를 변환할 때 사용

			rayDir = XMVector3Normalize( rayDir );	// 교차 판정을 위해 반직선 방향벡터를 단위길이로 정규화

			// 참일 경우 충돌된 것이므로 현재 가장 가까운 곳에 충돌된 것과 검사
			if (tempObj->CheckRayIntersection( &rayOrigin, &rayDir, &fHitDist, &vHitPos ))
			{
				bIntersection = true;
				if (fNearDist > fHitDist)
				{
					fNearDist = fHitDist;
					IntersectionObject = tempObj;
					vNearPos = vHitPos;
				}
			}
		}
	}

	if (IntersectionObject != NULL)
	{
		pPickedObject = IntersectionObject;
		vPickPos = vNearPos;
		// vNearPos는 오브젝트의 로컬좌표계이므로 이를 월드좌표계로 변환
		vPickPos = MathHelper::GetInstance( )->Vector3TransformNormal( vPickPos, pPickedObject->m_mtxWorld );
	}
	return bIntersection;
}


bool CScene::CollisionCheck( )
{
	// 플레이어와 각각의 오브젝트들의 충돌체크를 검사
	// 각 셰이더에 있는 오브젝트들과 검사하여 충돌되면 위치 이동 불가
	// 플레이어가 이동할 때 검사해야함

	// 세이더 전체 검색
	// 셰이더의 0번째는 항상 스카이박스, 스카이박스와는 충돌체크할 필요가 없음
	for (int i = 1; i <m_nShaders; i++)
	{
		// 각 세이더가 가지는 오브젝트 검색
		int objCount = m_ppShaders[i]->getObjectCount( );
		for (int j = 0; j <objCount; j++)
		{
			// 오브젝트와 일일히 검사, 하나라도 충돌하면 true를 리턴함
			if (CollisionCheck( m_pPlayer, m_ppShaders[i]->getObjects( )[j] ) )
				return true;
		}
	}
	// 하나도 충돌하지 않은 경우 false 리턴
	return false;
}

bool CScene::CollisionCheck( CGameObject *pObj1, CGameObject *pObj2 )
{
	// 충돌되면 true 반환
	AABB playerBox = pObj1->m_bcMeshBoundingCube;
	AABB objBox = pObj2->m_bcMeshBoundingCube;

	// AABB를 해당 오브젝트에 맞게 변환		-> 물체를 생성할 때 위치를 잡고나면 update하지 않아도 되야 하는데 자꾸 초기화됨
	playerBox.Update( &( pObj1->m_mtxWorld ) );
	objBox.Update( &( pObj2->m_mtxWorld ) );

	// AABB 출돌 검사
	if (playerBox.m_vMax.x < objBox.m_vMin.x) return true;
	if (playerBox.m_vMax.y < objBox.m_vMin.y) return true;
	if (playerBox.m_vMax.z < objBox.m_vMin.z) return true;
	if (playerBox.m_vMin.x > objBox.m_vMax.x) return true;
	if (playerBox.m_vMin.y > objBox.m_vMax.y) return true;
	if (playerBox.m_vMin.z > objBox.m_vMax.z) return true;

	// 아무런 체크도 되지 않으면 충돌하지 않은 것
	return false;
}

bool CScene::LoadingSoundResource( )
{
	SoundManager::GetInstance( )->Loading( "Sound/stage1bgm2.mp3", FMOD_LOOP_NORMAL, BGM );
	SoundManager::GetInstance( )->Loading( "Sound/attack.wav", FMOD_DEFAULT, ATTACK );

	return false;
}
