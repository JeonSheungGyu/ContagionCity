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
	// ���� ��ü�� ���ߴ� �ֺ� ������ ����
	m_pLights->m_cGlobalAmbient = D3DXCOLOR( 0.1f, 0.1f, 0.1f, 1.0f );
	m_pLights->m_FogColor = D3DXCOLOR( 0.7f, 0.7f, 0.7f, 1.0f );
	m_pLights->m_FogStart = 100.0;
	m_pLights->m_FogRange = 200.0;

	// �¾�
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
	// ���� �ε� �� ���
	LoadingSoundResource( );
	SoundManager::GetInstance( )->Play( BGM );

	// �ؽ�ó ���ο� ����� ���÷� ���� ��ü�� ����
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

	// fbx loader�� ���� fbx�ε�
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

	// ���� ���̴� ������ fbx������ ���� �� ���̴��� �����Ͽ� ����
	m_nShaders = 2;		// 0�� ��ī�̹ڽ� , 1�� ���, 2�� ���ǽ�, 3�� �� 
	m_ppShaders = new CShader*[m_nShaders];

	// ù��°�� �׸� ��ü�� ��ī�̹ڽ�
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	// �ι�°�� ���
	m_ppShaders[1] = new CBackgroundShader( );
	m_ppShaders[1]->CreateShader( pd3dDevice );
	( (CBackgroundShader*)m_ppShaders[1] )->BuildObjects( pd3dDevice, background );



	
	CreateShaderVariables( pd3dDevice );
}

void CScene::LoadFBXs( )
{
	// fbx ���� �ε�
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
	/*���콺�� ĸ�������� ���콺�� �󸶸�ŭ �̵��Ͽ��� ���� ����Ѵ�.
	���콺 ���� �Ǵ� ������ ��ư�� ������ ���� �޽���(WM_LBUTTONDOWN, WM_RBUTTONDOWN)�� ó���� �� ���콺�� ĸ���Ͽ���.
	�׷��Ƿ� ���콺�� ĸ�ĵ� ���� ���콺 ��ư�� ������ ���¸� �ǹ��Ѵ�.
	���콺�� �¿� �Ǵ� ���Ϸ� �����̸� �÷��̾ x-�� �Ǵ� y-������ ȸ���Ѵ�.*/
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
			/*cxDelta�� y-���� ȸ���� ��Ÿ���� cyDelta�� x-���� ȸ���� ��Ÿ����.
			������ ���콺 ��ư�� ������ ��� cxDelta�� z-���� ȸ���� ��Ÿ����.*/
			// ���콺 Ŭ���� ���� �ִϸ��̼��� ���������� �ؾ���
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
		/*�÷��̾ dwDirection �������� �̵��Ѵ�(�����δ� �ӵ� ���͸� �����Ѵ�).
		�̵� �Ÿ��� �ð��� ����ϵ��� �Ѵ�. �÷��̾��� �̵� �ӷ��� (50/��)�� �����Ѵ�.
		���� �÷��̾��� �̵� �ӷ��� �ִٸ� �� ���� ����Ѵ�.*/
		if (dwDirection)
		{
			// ���� �÷��̾��� AABB �ڽ��� y��ǥ �ּҰ� -0.5��. ���� 0���� �����Ƿ� �ٴڰ� ��ħ, �׷� ���� ��������
			// �浹üũ ��ü�� ����� �ǰ� ������ �÷��̾��� ��ġ�� ����
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
		// ���� ī�޶��� ��ġ ���͸� ������ ��Ÿ���� ��� ���ۿ� ����
		XMFLOAT3 vCameraPosition = m_pCamera->GetPosition( );
		m_pLights->m_vCameraPosition = MathHelper::GetInstance( )->MakeFloat4( vCameraPosition, 1.0f );
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects( fTimeElapsed );
	}
}

//ī�޶� ���̴� ��ü�鸸 �������� �� �ֵ��� ������ ���̱� �����̴�(�̰��� ī�޶� ����ü �ø�(Frustum Culling)�̶�� �Ѵ�).
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
	// 2���� ���� ���� ���������� ��ȯ
	XMMATRIX p = XMLoadFloat4x4( &( m_pCamera->GetProjectionMatrix( ) ) );
	D3D11_VIEWPORT d3dViewport = m_pCamera->GetViewport( );

	XMFLOAT3 vPickPosition;
	vPickPosition.x = ( ( ( 2.0f * ( x - d3dViewport.TopLeftX ) ) / d3dViewport.Width ) - 1 ) / p( 0, 0 );
	vPickPosition.y = -( ( ( 2.0f * ( y - d3dViewport.TopLeftY ) ) / d3dViewport.Height ) - 1 ) / p( 1, 1 );

	XMVECTOR rayOrigin = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
	XMVECTOR rayDir = XMVectorSet( vPickPosition.x, vPickPosition.y, 1.0, 0.0f );

	// 2���� ���� �þ߰��������� ��ȯ
	XMMATRIX v = XMLoadFloat4x4( &( m_pCamera->GetViewMatrix( ) ) );
	XMMATRIX invView = XMMatrixInverse( &XMMatrixDeterminant( v ), v );		// determinant = ��Ľ�

	// �� ��ü�� �浹üũ
	float fHitDist = FLT_MAX, fNearDist = FLT_MAX;
	XMFLOAT3 vHitPos, vNearPos;
	bool bIntersection = false;
	CGameObject *IntersectionObject = NULL;		// �浹�� ������Ʈ ������ ����
	// �������� ���̴��� ���鼭 �˻���
	for (int i = 1; i < m_nShaders; i++)		// 0��° ���̴��� ��ī�̹ڽ��� �׸��ǰ� �˻��� �ʿ䰡 ����
	{
		for (int j = 0; j < m_ppShaders[i]->getObjectCount( ); j++)
		{
			CGameObject* tempObj = m_ppShaders[i]->getObjects( )[j];

			XMMATRIX w = XMLoadFloat4x4( &( tempObj->m_mtxWorld ) );
			XMMATRIX invWorld = XMMatrixInverse( &XMMatrixDeterminant( w ), w );

			XMMATRIX toLocal = XMMatrixMultiply( invView, invWorld );

			rayOrigin = XMVector3TransformCoord( rayOrigin, toLocal );	// XMVector3TransformCoord �Լ��� ������ 4��° ������ 1�̶�� �����ϰ� ���, ���� ���� ��ȯ�� �� ���
			rayDir = XMVector3TransformNormal( rayDir, toLocal );			// XMVector3TransformNormal �Լ��� ������ 4��° ������ 0�̶�� �����ϰ� ���, ���� ���͸� ��ȯ�� �� ���

			rayDir = XMVector3Normalize( rayDir );	// ���� ������ ���� ������ ���⺤�͸� �������̷� ����ȭ

			// ���� ��� �浹�� ���̹Ƿ� ���� ���� ����� ���� �浹�� �Ͱ� �˻�
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
		// vNearPos�� ������Ʈ�� ������ǥ���̹Ƿ� �̸� ������ǥ��� ��ȯ
		vPickPos = MathHelper::GetInstance( )->Vector3TransformNormal( vPickPos, pPickedObject->m_mtxWorld );
	}
	return bIntersection;
}


bool CScene::CollisionCheck( )
{
	// �÷��̾�� ������ ������Ʈ���� �浹üũ�� �˻�
	// �� ���̴��� �ִ� ������Ʈ��� �˻��Ͽ� �浹�Ǹ� ��ġ �̵� �Ұ�
	// �÷��̾ �̵��� �� �˻��ؾ���

	// ���̴� ��ü �˻�
	// ���̴��� 0��°�� �׻� ��ī�̹ڽ�, ��ī�̹ڽ��ʹ� �浹üũ�� �ʿ䰡 ����
	for (int i = 1; i <m_nShaders; i++)
	{
		// �� ���̴��� ������ ������Ʈ �˻�
		int objCount = m_ppShaders[i]->getObjectCount( );
		for (int j = 0; j <objCount; j++)
		{
			// ������Ʈ�� ������ �˻�, �ϳ��� �浹�ϸ� true�� ������
			if (CollisionCheck( m_pPlayer, m_ppShaders[i]->getObjects( )[j] ) )
				return true;
		}
	}
	// �ϳ��� �浹���� ���� ��� false ����
	return false;
}

bool CScene::CollisionCheck( CGameObject *pObj1, CGameObject *pObj2 )
{
	// �浹�Ǹ� true ��ȯ
	AABB playerBox = pObj1->m_bcMeshBoundingCube;
	AABB objBox = pObj2->m_bcMeshBoundingCube;

	// AABB�� �ش� ������Ʈ�� �°� ��ȯ		-> ��ü�� ������ �� ��ġ�� ����� update���� �ʾƵ� �Ǿ� �ϴµ� �ڲ� �ʱ�ȭ��
	playerBox.Update( &( pObj1->m_mtxWorld ) );
	objBox.Update( &( pObj2->m_mtxWorld ) );

	// AABB �⵹ �˻�
	if (playerBox.m_vMax.x < objBox.m_vMin.x) return true;
	if (playerBox.m_vMax.y < objBox.m_vMin.y) return true;
	if (playerBox.m_vMax.z < objBox.m_vMin.z) return true;
	if (playerBox.m_vMin.x > objBox.m_vMax.x) return true;
	if (playerBox.m_vMin.y > objBox.m_vMax.y) return true;
	if (playerBox.m_vMin.z > objBox.m_vMax.z) return true;

	// �ƹ��� üũ�� ���� ������ �浹���� ���� ��
	return false;
}

bool CScene::LoadingSoundResource( )
{
	SoundManager::GetInstance( )->Loading( "Sound/stage1bgm2.mp3", FMOD_LOOP_NORMAL, BGM );
	SoundManager::GetInstance( )->Loading( "Sound/attack.wav", FMOD_DEFAULT, ATTACK );

	return false;
}
