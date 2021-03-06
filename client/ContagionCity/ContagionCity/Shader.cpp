#include "stdafx.h"
#include "Shader.h"
#include "EnemyObject.h"

#define PI 3.14

// 월드 변환 행렬을 위한 상수 버퍼는 셰이더 객체의 정적 데이터 멤버이다.
// 스테틱 멤버 변수는 무조건 이렇게 선언한다.
ID3D11Buffer *CShader::m_pd3dcbWorldMatrix = NULL;
ID3D11Buffer *CIlluminatedShader::m_pd3dcbMaterial = NULL;
ID3D11Buffer *CPlayerShader::m_pd3dcbOffsetMatrix = NULL;

CShader::CShader( )
{
	m_nObjects = 0;

	m_pd3dVertexShader = NULL;
	m_pd3dVertexLayout = NULL;
	m_pd3dPixelShader = NULL;
}


CShader::~CShader( )
{
	if (m_pd3dVertexShader) m_pd3dVertexShader->Release( );
	if (m_pd3dVertexLayout) m_pd3dVertexLayout->Release( );
	if (m_pd3dPixelShader) m_pd3dPixelShader->Release( );
}

void CShader::BuildObjects( ID3D11Device *pd3dDevice )
{

}

void CShader::ReleaseObject( )
{
	if (m_ppObjects.size())
	{
		for (int i = 0; i < m_nObjects; i++)
			if (m_ppObjects[i])
				delete m_ppObjects[i];
	}
}

void CShader::AnimateObjects( float fTimeElapsed )
{
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->Animate( fTimeElapsed );
	}
}

void CShader::OnPrepareRender( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->IASetInputLayout( m_pd3dVertexLayout );
	pd3dDeviceContext->VSSetShader( m_pd3dVertexShader, NULL, 0 );
	pd3dDeviceContext->PSSetShader( m_pd3dPixelShader, NULL, 0 );
}

void CShader::CreateVertexShaderFromFile( ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputLayout, UINT nElements, ID3D11InputLayout **ppd3dVertexLayout )
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dShaderBlob = NULL, *pd3dErrorBlob = NULL;
	if (SUCCEEDED( hResult = D3DX11CompileFromFile( pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dShaderBlob, &pd3dErrorBlob, NULL ) ))
	{
		// 컴파일된 셰이더 코드의 메모리 주소에서 정점 셰이더를 생성
		pd3dDevice->CreateVertexShader( pd3dShaderBlob->GetBufferPointer( ), pd3dShaderBlob->GetBufferSize( ), NULL, ppd3dVertexShader );
		// 컴파일된 셰이더 코드의 메모리 주소와 입력 레이아웃에서 정점 레이아웃을 생성
		pd3dDevice->CreateInputLayout( pd3dInputLayout, nElements, pd3dShaderBlob->GetBufferPointer( ), pd3dShaderBlob->GetBufferSize( ), ppd3dVertexLayout );
		pd3dShaderBlob->Release( );
	}
}

void CShader::CreatePixelShaderFromFile( ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader )
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dShaderBlob = NULL, *pd3dErrorBlob = NULL;
	if (SUCCEEDED( hResult = D3DX11CompileFromFile( pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dShaderBlob, &pd3dErrorBlob, NULL ) ))
	{
		// 컴파일된 셰이더 코드의 메모리 주소에서 픽셀 셰이더를 생성
		pd3dDevice->CreatePixelShader( pd3dShaderBlob->GetBufferPointer( ), pd3dShaderBlob->GetBufferSize( ), NULL, ppd3dPixelShader );
		pd3dShaderBlob->Release( );
	}
}

void CShader::CreateShader( ID3D11Device *pd3dDevice )
{
	/*IA 단계에 설정할 입력-레이아웃을 정의한다.
	정점 버퍼의 한 원소가 CVertex 클래스의 멤버 변수(D3DXVECTOR3 즉, 실수 세 개)이므로
	입력-레이아웃은 실수(32-비트) 3개로 구성되며 시멘틱이 “POSITION”이고 정점 데이터임을 표현한다.*/
	//하나의 정점은 위치 벡터(실수 3개)와 색상(실수 4개)을 포함한다. 
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputLayout );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VS", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PS", "ps_5_0", &m_pd3dPixelShader );
	// 월드 변환 행렬을 위한 상수버퍼 생성
	CreateShaderVariables( pd3dDevice );
}

void CShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	OnPrepareRender( pd3dDeviceContext );

	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppObjects[i])
		{
			if (m_ppObjects[i]->IsVisible( pCamera ))
			{
				m_ppObjects[i]->Render( pd3dDeviceContext, pCamera );
			}
		}
	}
}

void CShader::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	// 월드 변환 행렬을 위한 상수 버퍼를 생성
	D3D11_BUFFER_DESC bd;
	::ZeroMemory( &bd, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( VS_CB_WORLD_MATRIX );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer( &bd, NULL, &m_pd3dcbWorldMatrix );
}

void CShader::ReleaseShaderVariables( )
{
	if (m_pd3dcbWorldMatrix) m_pd3dcbWorldMatrix->Release();
}

void CShader::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 *pmtxWorld )
{
	// 월드 변환 행렬을 상수 버퍼에 복사
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbWorldMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	VS_CB_WORLD_MATRIX *pcbWorldMatrix = (VS_CB_WORLD_MATRIX*)d3dMappedResource.pData;
	pcbWorldMatrix->m_mtxWorld =  MathHelper::GetInstance( )->TransposeFloat4x4( *pmtxWorld );
	pd3dDeviceContext->Unmap( m_pd3dcbWorldMatrix, 0 );
	
	// 상수 버퍼를 디바이스의 슬롯에 연결
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix );
}

// 애니메이션 할 일이 있는 셰이더를 위한 추상 클래스
CAnimatedObjShader::CAnimatedObjShader( )
{

}
CAnimatedObjShader::~CAnimatedObjShader( )
{
}

void CAnimatedObjShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// 범프매핑 하면 이거 필요없음
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "SkinnedVS", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "SkinnedPS", "ps_5_0", &m_pd3dPixelShader );

	CreateShaderVariables( pd3dDevice );
}

void CAnimatedObjShader::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	// 애니메이션 변환 행렬을 위한 상수 버퍼를 생성
	D3D11_BUFFER_DESC bd;
	::ZeroMemory( &bd, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( VS_CB_OFFSET_MATRIX );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer( &bd, NULL, &m_pd3dcbOffsetMatrix );
}
void CAnimatedObjShader::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, std::vector<XMFLOAT4X4> pmtxWorld )
{
	// 월드 변환 행렬을 상수 버퍼에 복사
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbOffsetMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	VS_CB_OFFSET_MATRIX *pcbOffsetMatrix = (VS_CB_OFFSET_MATRIX*)d3dMappedResource.pData;
	for (int i = 0; i < pmtxWorld.size( ); i++){
		pcbOffsetMatrix->m_mtxOffsets[i] = MathHelper::GetInstance( )->TransposeFloat4x4( pmtxWorld[i] );
	}
	pd3dDeviceContext->Unmap( m_pd3dcbOffsetMatrix, 0 );

	// 상수 버퍼를 디바이스의 슬롯에 연결
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_OFFSET_MATRIX, 1, &m_pd3dcbOffsetMatrix );
}

///////////////////////////////// 적들을 그릴 클래스
CEnemyShader::CEnemyShader( )
{
}
CEnemyShader::~CEnemyShader( )
{
}

void CEnemyShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// 범프매핑 하면 이거 필요없음
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
//		{ "INSTANCEPOS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
//		{ "INSTANCEPOS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
//		{ "INSTANCEPOS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "SkinnedVS", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "SkinnedPS", "ps_5_0", &m_pd3dPixelShader );

	CreateShaderVariables( pd3dDevice );
}

ID3D11Buffer *CEnemyShader::CreateInstanceBuffer( ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData )
{
	ID3D11Buffer *pd3dInstanceBuffer = NULL;
	D3D11_BUFFER_DESC d3dBufferDesc;	
	ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	/*버퍼의 초기화 데이터가 없으면 동적 버퍼로 생성한다. 즉, 나중에 매핑을 하여 내용을 채우거나 변경한다.*/
	d3dBufferDesc.Usage = ( pBufferData ) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = nBufferStride * nObjects;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = ( pBufferData ) ? 0 : D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = pBufferData;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, ( pBufferData ) ? &d3dBufferData : NULL, &pd3dInstanceBuffer );
	return( pd3dInstanceBuffer );
}

void CEnemyShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes )
{
	if (meshes.size( ) == 0)
		return;

	m_nObjects = 10;

	m_nInstanceBufferStride = sizeof( XMFLOAT4X4 );
	m_nInstanceBufferOffset = 0;

	for (int i = 0; i < meshes.size( ); i++)
	{
		switch (meshes[i].m_iType)
		{
			case ENEMY_ZOMBIE_MAN:
			{
				// 텍스처개수
				int textureCnt = meshes[i].m_pTextures.size( );

				// 몬스터 메시 생성
				CAnimatedMesh* pEnemyMesh = new CAnimatedMesh( pd3dDevice, meshes[0], textureCnt );

				for (int Cnt = 0; Cnt < m_nObjects / meshes.size( ); Cnt++)
				{
					// 몬스터 오브젝트 생성
					CEnemyObject* pEnemy = new CEnemyObject( meshes[i],1 );
					// 텍스처 등 값 세팅
					pEnemyMesh->FindMinMax( );
					for (int j = 0; j < textureCnt; j++)
						pEnemyMesh->OnChangeTexture( pd3dDevice, meshes[i].m_pTextures[j], j );
					CMaterial *pGroundMaterial = new CMaterial;
					pEnemy->SetMaterial( pGroundMaterial );

					// 오브젝트에 메시 세팅
					pEnemy->SetMesh( pEnemyMesh );
					pEnemy->CreateShaderVariables( pd3dDevice );
					pEnemy->SetPosition( XMFLOAT3( rand( ) % 4000, 0.0f, rand( ) % 2000+100 ) );

					// 몬스터 오브젝트를 셰이더에 저장
					m_ppObjects.push_back( pEnemy );
				}
				break;
			}
			case ENEMY_ZOMBIE_WOMAN:
			{
				// 텍스처개수
				int textureCnt = meshes[i].m_pTextures.size( );

				// 몬스터 메시 생성
				CAnimatedMesh* pEnemyMesh = new CAnimatedMesh( pd3dDevice, meshes[i], textureCnt );

				for (int Cnt = 0; Cnt < m_nObjects / 2; Cnt++)
				{
					// 몬스터 오브젝트 생성
					AnimatedObjectInfo* pEnemy = new AnimatedObjectInfo( meshes[i] );
					// 텍스처 등 값 세팅
					pEnemyMesh->FindMinMax( );
					for (int j = 0; j < textureCnt; j++)
						pEnemyMesh->OnChangeTexture( pd3dDevice, meshes[i].m_pTextures[j], j );
					CMaterial *pGroundMaterial = new CMaterial;
					pEnemy->SetMaterial( pGroundMaterial );

					// 오브젝트에 메시 세팅
					pEnemy->SetMesh( pEnemyMesh );
					pEnemy->CreateShaderVariables( pd3dDevice );
					pEnemy->SetPosition( XMFLOAT3( rand( ) % 4000, 0.0f, rand( ) % 2000 + 100 ) );

					// 몬스터 오브젝트를 셰이더에 저장
					m_ppObjects.push_back( pEnemy );
				}
				break;
			}
		}
	}
//	m_pd3dInstanceBuffer = CreateInstanceBuffer( pd3dDevice, m_nObjects, m_nInstanceBufferStride, NULL );
//	pEnemyMesh->AssembleToVertexBuffer( 1, &m_pd3dInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset );
}

void CEnemyShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::Render( pd3dDeviceContext, pCamera );
	//D3D11_MAPPED_SUBRESOURCE d3dMappedResource;

	//int nEnemyInstances = 0;
	//pd3dDeviceContext->Map( m_pd3dInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	//VS_VB_INSTANCE *pEnemyInstances = (VS_VB_INSTANCE *)d3dMappedResource.pData;

	//for (int i = 0; i < m_nObjects; i++)
	//{
	//	if (m_ppObjects[i])
	//	{
	//		if (m_ppObjects[i]->IsVisible( pCamera ))
	//		{
	//			UpdateShaderVariable( pd3dDeviceContext, ( (AnimatedObjectInfo*)m_ppObjects[i] )->m_pmtxFinalTransforms );
	//			pEnemyInstances[nEnemyInstances++].m_mtxTransform = MathHelper::GetInstance( )->TransposeFloat4x4( m_ppObjects[i]->m_mtxWorld );
	//		}
	//	}
	//}
	//pd3dDeviceContext->Unmap( m_pd3dInstanceBuffer, 0 );

	//CMesh *pEnemyMesh = m_ppObjects[m_nObjects - 1]->GetMesh( );
	//pEnemyMesh->RenderInstanced( pd3dDeviceContext, nEnemyInstances, 0 );
}

///////////////////////플레이어를 그리기 위한 셰이더 클래스
CPlayerShader::CPlayerShader( )
{
}

CPlayerShader::~CPlayerShader( )
{
}

void CPlayerShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes )
{
	for (int i = 0; i < meshes.size( ); i++)
	{
		switch (meshes[i].m_iType)
		{
			case PLAYER_MAN:
			case PLAYER_WAMEN:
			{
				CPlayer *pPlayer = new CPlayer( meshes[0] );
				pPlayer->m_iType = meshes[i].m_iType;
				pPlayer->m_iLayer = meshes[i].m_iLayer;

				int textureCount = meshes[i].m_pTextures.size( );
				CAnimatedMesh *pPlayerMesh = new CAnimatedMesh( pd3dDevice, meshes[i], textureCount );
				pPlayerMesh->FindMinMax( );		// AABB 상자 값 세팅
				for (int j = 0; j < textureCount; j++)
					pPlayerMesh->OnChangeTexture( pd3dDevice, meshes[i].m_pTextures[j], j );
				pPlayer->SetMesh( pPlayerMesh, 0 );
				pPlayer->CreateShaderVariables( pd3dDevice );
				pPlayer->ChangeCamera( pd3dDevice, THIRD_PERSON_CAMERA, 0.0f );
				pPlayer->SetPosition( XMFLOAT3( 0.0f, 1.0f, 0.0f ) );

				CMaterial *pPlayerMaterial = new CMaterial;
				pPlayer->SetMaterial( pPlayerMaterial );

				m_ppObjects.push_back( pPlayer );

				break;
			}
			case PLAYER_WEAPON:
			{
				CGameObject *pWeapon = new CGameObject( 1 );
				pWeapon->m_iType = meshes[i].m_iType;
				pWeapon->m_iLayer = meshes[i].m_iLayer;

				for (int j = 0; j < meshes[0].m_skinnedData.mBoneHierarchy.size( ); j++)
				{
					if (meshes[0].m_skinnedData.mBoneHierarchy[j].boneName == "Bip001 R Hand")
					{
						m_rightHandIdx = j;
						break;
					}
				}
				int textureCount = meshes[i].m_pTextures.size( );
				CAnimatedMesh *pWeaponMesh = new CAnimatedMesh( pd3dDevice, meshes[i], textureCount );
				pWeaponMesh->FindMinMax( );		// AABB 상자 값 세팅

				for (int j = 0; j < textureCount; j++)
					pWeaponMesh->OnChangeTexture( pd3dDevice, meshes[i].m_pTextures[j], j );
				pWeapon->SetMesh( pWeaponMesh, 0);

				CMaterial *pPlayerMaterial = new CMaterial;
				pWeapon->SetMaterial( pPlayerMaterial );
				pWeapon->Rotate( 0.0f, 0.0f, 0.0f );
//				pWeapon->SetPosition( 0.0f, 40.0f, 0.0f );
				m_ppObjects.push_back( pWeapon );

				break;
			}
		}
	}
	m_nObjects = m_ppObjects.size( );
}
bool CPlayerShader::CollisionCheck( CGameObject* pObject )
{
	return false;
}

void CPlayerShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	// 3인칭 카메라일 때 플레이어를 렌더링
	DWORD nCameraMode = ( pCamera ) ? pCamera->GetMode( ) : 0x00;

	for (int i = 1; i < m_nObjects; i++)
	{
		m_ppObjects[i]->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( m_ppObjects[i]->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 ) );
		m_ppObjects[i]->m_mtxWorld = ( (AnimatedObjectInfo *)m_ppObjects[0] )->m_pmtxFinalTransforms[m_rightHandIdx];
		m_ppObjects[i]->SetPosition( m_ppObjects[0]->GetPosition( ) );
	}

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		UpdateShaderVariable( pd3dDeviceContext, ((AnimatedObjectInfo*)m_ppObjects[0])->m_pmtxFinalTransforms);
		CShader::Render( pd3dDeviceContext, pCamera );
	}
}

/////////////////////////////// 플레이어의 뼈대를 그리는 클래스
CPlayerBoneShader::CPlayerBoneShader( )
{
	m_TimePos = 0.0f;
}

CPlayerBoneShader::~CPlayerBoneShader( )
{

}

void CPlayerBoneShader::CreateShader( ID3D11Device *pd3dDevice )
{
	CShader::CreateShader( pd3dDevice );
}

void CPlayerBoneShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes )
{
	m_skinnedData = meshes[0].m_skinnedData;
	for (int i = 0; i < m_nObjects; i++)
	{
		CCubeMeshDiffused *pBoneMesh = new CCubeMeshDiffused( pd3dDevice, 5, 5, 5, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 0.0f ) );
		CGameObject *pBone = new CGameObject( 1 );

		pBone->SetMesh( pBoneMesh, 0 );
		m_ppObjects.push_back(pBone);
	}
	m_nObjects = m_ppObjects.size( );
}

void CPlayerBoneShader::AnimateObjects( float fTimeElapsed )
{
	std::vector<XMFLOAT4X4> mtx(m_nObjects);
	m_TimePos += fTimeElapsed;

	m_skinnedData.GetFinalTransforms( 0, m_TimePos, mtx );

	if (m_TimePos > m_skinnedData.GetClipEndTime( 0 ))
		m_TimePos = 0.0f;

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->m_mtxWorld = mtx[i];
	}
}

void CPlayerBoneShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	// 3인칭 카메라일 때 플레이어를 렌더링
	DWORD nCameraMode = ( pCamera ) ? pCamera->GetMode( ) : 0x00;

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		CShader::Render( pd3dDeviceContext, pCamera );
	}
}
CVilligeBackgroundShader::CVilligeBackgroundShader( )
{

}
CVilligeBackgroundShader::~CVilligeBackgroundShader( )
{

}

void CVilligeBackgroundShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader );
}

void CVilligeBackgroundShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex )
{
	for (int i = 0; i < vertex.size( ); i++)
	{
		switch (vertex[i].m_iType)
		{
			case BACK_GROUND:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		

				for (int j = 0; j < 2; j++)
				{
					ObjectInfo *pGround = new ObjectInfo( vertex[i] );
					pGround->m_iType = vertex[i].m_iType;
					pGround->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int textureidx = 0; textureidx < textureCount; textureidx++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[textureidx], textureidx );

					pGround->SetMesh( pGroundMesh, 0 );
					pGround->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pGround->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pGround->SetMaterial( pGroundMaterial );
					pGround->SetPosition( XMFLOAT3( 0.0f - j* 7000.f, 0.0 + j*2.f, 0.0f ) );
					m_ppObjects.push_back( pGround );
				}
				ObjectInfo *pGround = new ObjectInfo( vertex[i] );
				pGround->m_iType = vertex[i].m_iType;
				pGround->m_iLayer = vertex[i].m_iLayer;

				pGroundMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pGround->SetMesh( pGroundMesh, 0 );
				pGround->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pGround->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pGround->SetMaterial( pGroundMaterial );
				pGround->SetPosition( XMFLOAT3( 5700.0f, -2.f, -6000.0f ) );
				m_ppObjects.push_back( pGround );
				break;
			}
			case BACK_HOSPITAL:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );	

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->Rotate( 0.0f, 0.0f, 180.0f );
				pObject->SetPosition( XMFLOAT3( 7000.0f, -2.f, -6600.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}
			case BACK_STRUCTURE:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->SetPosition( XMFLOAT3( -2000.0f, 0.f, 2000.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}

			case BACK_SHELTER:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->SetPosition( XMFLOAT3( 1700.0f, 0.f, 1700.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}

			case BACK_STORE:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->Rotate( 0.0f, 0.0f, 90.0f );
				pObject->SetPosition( XMFLOAT3( 0.0f, 0.f, -2500.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}

			case BACK_PROTECTWALL:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				for (int j = 0; j < 3; j++)
				{
					ObjectInfo *pObject = new ObjectInfo( vertex[i] );
					pObject->m_iType = vertex[i].m_iType;
					pObject->m_iLayer = vertex[i].m_iLayer;

					pObjectMesh->FindMinMax( );		// AABB 값 세팅
					for (int textureidx = 0; textureidx < textureCount; textureidx++)
						pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[textureidx], textureidx );

					pObject->SetMesh( pObjectMesh, 0 );
					pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pObject->SetMaterial( pGroundMaterial );
					pObject->Rotate( 0.0f, 0.0f, -45.0f );
					pObject->SetPosition( XMFLOAT3( 2000.0f + j*700.0f, 0.f, -3000.0f + j*700.0f ) );
					m_ppObjects.push_back( pObject );
				}
				break;
			}
			case BACK_WALL:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				for (int j = -2; j < 21; j++)
				{
					ObjectInfo *pObject = new ObjectInfo( vertex[i] );
					pObject->m_iType = vertex[i].m_iType;
					pObject->m_iLayer = vertex[i].m_iLayer;

					pObjectMesh->FindMinMax( );		// AABB 값 세팅
					for (int textureidx = 0; textureidx < textureCount; textureidx++)
						pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[textureidx], textureidx );

					pObject->SetMesh( pObjectMesh, 0 );
					pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pObject->SetMaterial( pGroundMaterial );
					pObject->Rotate( &XMFLOAT3( 0.0f, 0.0f, 1.0f ), j*(360/25) );
					pObject->SetPosition( XMFLOAT3( -4000 * sin( PI *( j * ( 360 / 25 ) ) / 180 ), 0.f, -4000 * cos( PI *( j * ( 360 / 25 ) ) / 180 ) ) );
					m_ppObjects.push_back( pObject );
				}
				break;
			}
			case BACK_RUIN_1:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->Rotate( 0.0f, 0.0f, 90.0f );
				pObject->SetPosition( XMFLOAT3( 0.0f, 1000.f,1000.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}
			case BACK_RUIN_2:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->Rotate( 0.0f, 0.0f, 90.0f );
				pObject->SetPosition( XMFLOAT3( 3000.0f, 100.f, 0.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}
			case BACK_RUIN_3:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pObjectMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );

				ObjectInfo *pObject = new ObjectInfo( vertex[i] );
				pObject->m_iType = vertex[i].m_iType;
				pObject->m_iLayer = vertex[i].m_iLayer;

				pObjectMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pObjectMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pObject->SetMesh( pObjectMesh, 0 );
				pObject->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pObject->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pObject->SetMaterial( pGroundMaterial );
				pObject->Rotate( 0.0f, 0.0f, 90.0f );
				pObject->SetPosition( XMFLOAT3( -3000.0f, 100.f, -1000.0f ) );
				m_ppObjects.push_back( pObject );

				break;
			}
			default:
				// 에러처리
				break;
		}
	}
	m_nObjects = m_ppObjects.size( );
}
void CVilligeBackgroundShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::Render( pd3dDeviceContext, pCamera );
}

// 배경에 관련된 것들을 그리는 클래스
CStage1BackgroundShader::CStage1BackgroundShader( )
{

}
CStage1BackgroundShader::~CStage1BackgroundShader( )
{

}

void CStage1BackgroundShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// 범프매핑 하면 이거 필요없음
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader );
}

void CStage1BackgroundShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex )
{
	for (int i = 0; i < vertex.size(); i++)
	{
		switch (vertex[i].m_iType)
		{
			case BACK_ENTERANCE:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				ObjectInfo *pWall = new ObjectInfo( vertex[i] );
				pWall->m_iType = vertex[i].m_iType;
				pWall->m_iLayer = vertex[i].m_iLayer;

				pGroundMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pWall->SetMesh( pGroundMesh, 0 );
				pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
				pWall->Rotate( 0.0f, 0.0f, 180.0f );
				pWall->SetPosition( XMFLOAT3( -1500.f - 3000.f, 0.0f, 49.9f ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pWall->SetMaterial( pGroundMaterial );

				m_ppObjects.push_back( pWall );
				break;
			}
			case BACK_GROUND:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				for (int ObjCnt = 0; ObjCnt < 1; ObjCnt++)
				{
					ObjectInfo *pGround = new ObjectInfo( vertex[i] );
					pGround->m_iType = vertex[i].m_iType;
					pGround->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int j = 0; j < textureCount; j++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

					pGround->SetMesh( pGroundMesh, 0 );
					pGround->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pGround->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pGround->SetMaterial( pGroundMaterial );
					pGround->SetPosition( XMFLOAT3( 0.0f, 470.0f*ObjCnt, 0.0f ) );
					m_ppObjects.push_back( pGround );
				}
				break;
			}
			case BACK_CEIL:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				for (int ObjCnt = 0; ObjCnt < 1; ObjCnt++)
				{
					ObjectInfo *pGround = new ObjectInfo( vertex[i] );
					pGround->m_iType = vertex[i].m_iType;
					pGround->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int j = 0; j < textureCount; j++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

					pGround->SetMesh( pGroundMesh, 0 );
					pGround->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pGround->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pGround->SetMaterial( pGroundMaterial );
					pGround->SetPosition( XMFLOAT3( 0.0f, 430.0f*(ObjCnt+1), 0.0f ) );
					m_ppObjects.push_back( pGround );
				}
				break;
			}
			case BACK_BENCH:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				for (int j = 0; j < 6; j++)
				{
					ObjectInfo *pGround = new ObjectInfo( vertex[i] );
					pGround->m_iType = vertex[i].m_iType;
					pGround->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int tCount = 0; tCount < textureCount; tCount++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

					pGround->SetMesh( pGroundMesh, 0 );
					pGround->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pGround->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pGround->SetMaterial( pGroundMaterial );
					pGround->Rotate( 0.0f, 0.0f, rand()%90 );
					pGround->Rotate( -90.0f, 0.0f, 0.0f );
					pGround->SetPosition( XMFLOAT3( rand()%20000 - 10000.0f, 40.0f, rand()%3000 + 2000.0f ) );

					m_ppObjects.push_back( pGround );
				}
				break;
			}
			case BACK_FRONTDESK:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				ObjectInfo *pWall = new ObjectInfo( vertex[i] );
				pWall->m_iType = vertex[i].m_iType;
				pWall->m_iLayer = vertex[i].m_iLayer;

				pGroundMesh->FindMinMax( );		// AABB 값 세팅
				for (int j = 0; j < textureCount; j++)
					pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );

				pWall->SetMesh( pGroundMesh, 0 );
				pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
				pWall->SetPosition( XMFLOAT3(-8500.0f, 0.0f, 4000.0f  ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pWall->SetMaterial( pGroundMaterial );

				m_ppObjects.push_back( pWall );
				break;
			}
			case BACK_CHAIR:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				for (int j = 0; j < 10; j++)
				{
					ObjectInfo *pWall = new ObjectInfo( vertex[i] );
					pWall->m_iType = vertex[i].m_iType;
					pWall->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int tCount = 0; tCount < textureCount; tCount++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

					pWall->SetMesh( pGroundMesh, 0 );
					pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
					if (j % 2 == 0)
						pWall->Rotate( -90.0f, 0.0f, 0.0f );
					pWall->SetPosition( XMFLOAT3( rand( ) % 20000 - 10000.0f, 40.0f, rand( ) % 3000 + 2000.0f ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pWall->SetMaterial( pGroundMaterial );

					m_ppObjects.push_back( pWall );
				}
				for (int j = 0; j < 10; j++)
				{
					ObjectInfo *pWall = new ObjectInfo( vertex[i] );
					pWall->m_iType = vertex[i].m_iType;
					pWall->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int tCount = 0; tCount < textureCount; tCount++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

					pWall->SetMesh( pGroundMesh, 0 );
					pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
					if (j % 2 == 0)
						pWall->Rotate( -90.0f, 0.0f, 0.0f );
					pWall->SetPosition( XMFLOAT3( rand( ) % 10000, 40.0f, rand( ) % 3000 - 5000.0f ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pWall->SetMaterial( pGroundMaterial );

					m_ppObjects.push_back( pWall );
				}
				break;
			}
			case BACK_BED:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				for (int j = 1; j < 4; j++)
				{
					ObjectInfo *pWall = new ObjectInfo( vertex[i] );
					pWall->m_iType = vertex[i].m_iType;
					pWall->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int tCount = 0; tCount < textureCount; tCount++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

					pWall->SetMesh( pGroundMesh, 0 );
					pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
					pWall->SetPosition( XMFLOAT3( 8500.f - 1000.f*( j + 5 ), 50.0f, 49.9f ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pWall->SetMaterial( pGroundMaterial );

					m_ppObjects.push_back( pWall );
				}
				break;
				break;
			}
			case BACK_DOOR:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				for (int j = 1; j < 3; j++)
				{
					ObjectInfo *pWall = new ObjectInfo( vertex[i] );
					pWall->m_iType = vertex[i].m_iType;
					pWall->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int tCount = 0; tCount < textureCount; tCount++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

					pWall->SetMesh( pGroundMesh, 0 );
					pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
					pWall->SetPosition( XMFLOAT3( 8500.f - 1000.f*( j + 5 ), 0.f, 49.9f ) );

					CMaterial *pGroundMaterial = new CMaterial;
					pWall->SetMaterial( pGroundMaterial );

					m_ppObjects.push_back( pWall );
				}
				break;
			}
			case BACK_DOORWALL:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용


				ObjectInfo *pWall = new ObjectInfo( vertex[i] );
				pWall->m_iType = vertex[i].m_iType;
				pWall->m_iLayer = vertex[i].m_iLayer;

				pGroundMesh->FindMinMax( );		// AABB 값 세팅
				for (int tCount = 0; tCount < textureCount; tCount++)
					pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

				pWall->SetMesh( pGroundMesh, 0 );
				pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
				//		pWall->Rotate( 0.0f, 180.0f, 0.0f );
				pWall->SetPosition( XMFLOAT3( 8500.f - 1000.f, 0.f, 49.9f ) );

				CMaterial *pGroundMaterial = new CMaterial;
				pWall->SetMaterial( pGroundMaterial );

				m_ppObjects.push_back( pWall );

				pWall = new ObjectInfo( vertex[i] );
				pWall->m_iType = vertex[i].m_iType;
				pWall->m_iLayer = vertex[i].m_iLayer;

				pGroundMesh->FindMinMax( );		// AABB 값 세팅
				for (int tCount = 0; tCount < textureCount; tCount++)
					pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

				pWall->SetMesh( pGroundMesh, 0 );
				pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
				//		pWall->Rotate( 0.0f, 180.0f, 0.0f );
				pWall->SetPosition( XMFLOAT3( 8500.f - 6000.f, 0.f, 49.9f ) );

				pGroundMaterial = new CMaterial;
				pWall->SetMaterial( pGroundMaterial );

				m_ppObjects.push_back( pWall );

				break;
			}
			case BACK_WALL:
			{
				int textureCount = vertex[i].m_pTextures.size( );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// 노멀매핑을 위해 텍스처 2개 사용

				// 1층 병실 벽
				for (int j = 0; j < 10; j++)
				{
					if (j != 2 && j != 7)
					{
						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount < textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
				//		pWall->Rotate( 0.0f, 180.0f, 0.0f );
						pWall->SetPosition( XMFLOAT3( 8500.f - 1000.f*( j - 1 ), 225.f, 49.9f ) );

						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
				}
				// 병실 가운데 벽
				for (int j = 0; j < 5; j++)
				{
					ObjectInfo *pWall = new ObjectInfo( vertex[i] );
					pWall->m_iType = vertex[i].m_iType;
					pWall->m_iLayer = vertex[i].m_iLayer;

					pGroundMesh->FindMinMax( );		// AABB 값 세팅
					for (int tCount = 0; tCount < textureCount; tCount++)
						pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

					pWall->SetMesh( pGroundMesh, 0 );
					pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
					pWall->Rotate( 0.0f, 0.0f, 90.0f );
					pWall->SetPosition( XMFLOAT3( 4709.8f, 225.f, 3502.f - 1000.f * ( j - 1 )) );


					CMaterial *pGroundMaterial = new CMaterial;
					pWall->SetMaterial( pGroundMaterial );

					m_ppObjects.push_back( pWall );
				}

				// 1층과 2층 구분
				for (int k = 0; k < 1; k++)
				{
					// 앞면
					for (int j = 0; j < 10; j++)
					{
						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount < textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
		//				pWall->Rotate( 0.0f, 180.0f, 0.0f );
						pWall->SetPosition( XMFLOAT3( 8500.f - 1000.f*( j - 1 ), 225.f + (450.0f * k),-5000.f ) );

						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
					for (int j = 0; j < 10; j++)
					{
						if (j == 4)
							continue;

						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount < textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
		//				pWall->Rotate( 0.0f, 180.0f, 0.0f );
						pWall->SetPosition( XMFLOAT3( -1500.f - 1000.f*( j - 1 ), 225.f + ( 450.0f * k ), 49.9f ) );

						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
					// 뒷면
					for (int j = 0; j < 20; j++)
					{
						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount < textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
						pWall->SetPosition( XMFLOAT3( 8500.f - 1000.f*( j - 1 ), 225.f + ( 450.0f * k ), +5000.f ) );

						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
					// 오른쪽면
					for (int j = 0; j < 10; j++)
					{
						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount < textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
						pWall->Rotate( 180.0f, 0.0f, 90.0f );
						pWall->SetPosition( XMFLOAT3( 10000.f, 225.f + ( 450.0f * k ), 3500.0f - 1000.f * ( j - 1 ) ) );

						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
					// 왼쪽면
					for (int j = 0; j < 5; j++)
					{
						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount< textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
						pWall->Rotate( 0.0f, 0.0f, 90.0f );
						pWall->SetPosition( XMFLOAT3( -10000.f, 225.f + ( 450.0f * k ), 3500.0f - 1000.f * ( j - 1 ) ) );


						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
					for (int j = 0; j < 5; j++)
					{
						ObjectInfo *pWall = new ObjectInfo( vertex[i] );
						pWall->m_iType = vertex[i].m_iType;
						pWall->m_iLayer = vertex[i].m_iLayer;

						pGroundMesh->FindMinMax( );		// AABB 값 세팅
						for (int tCount = 0; tCount < textureCount; tCount++)
							pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[tCount], tCount );

						pWall->SetMesh( pGroundMesh, 0 );
						pWall->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pWall->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
						pWall->Rotate( 0.0f, 0.0f, 90.0f );
						pWall->SetPosition( XMFLOAT3( -49.8f, 225.f + ( 450.0f * k ), -1502.f - 1000.f * ( j - 1 ) ) );


						CMaterial *pGroundMaterial = new CMaterial;
						pWall->SetMaterial( pGroundMaterial );

						m_ppObjects.push_back( pWall );
					}
				}
				break;
			}
			default:
				// 에러처리
				break;
		}
	}
	m_nObjects = m_ppObjects.size( );
}

void CStage1BackgroundShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::Render( pd3dDeviceContext, pCamera );
}

////////////////////////////////// 빛과관련된 클래스
CIlluminatedShader::CIlluminatedShader( )
{
}

CIlluminatedShader::~CIlluminatedShader( )
{
}
void CIlluminatedShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSLightingColor", "ps_5_0", &m_pd3dPixelShader );
}

void CIlluminatedShader::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	d3dBufferDesc.ByteWidth = sizeof( MATERIAL );
	pd3dDevice->CreateBuffer( &d3dBufferDesc, NULL, &m_pd3dcbMaterial );
}

void CIlluminatedShader::ReleaseShaderVariables( )
{
	if (m_pd3dcbMaterial) m_pd3dcbMaterial->Release( );
}

void CIlluminatedShader::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, MATERIAL *pMaterial )
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	MATERIAL *pcbMaterial = (MATERIAL *)d3dMappedResource.pData;
	memcpy( pcbMaterial, pMaterial, sizeof( MATERIAL ) );
	pd3dDeviceContext->Unmap( m_pd3dcbMaterial, 0 );
	pd3dDeviceContext->PSSetConstantBuffers( PS_SLOT_MATERIAL, 1, &m_pd3dcbMaterial );
}

/////////////////////////////////////////////// 텍스처매핑이 필요한 객체를 위한 클래스
CTexturedShader::CTexturedShader( )
{
}

CTexturedShader::~CTexturedShader( )
{
}

void CTexturedShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSTexturedColor", "ps_5_0", &m_pd3dPixelShader );
}

////////////////////////////////스카이박스를 그릴 클래스
CSkyBoxShader::CSkyBoxShader( )
{
}

CSkyBoxShader::~CSkyBoxShader( )
{
}

void CSkyBoxShader::BuildObjects( ID3D11Device *pd3dDevice )
{
	m_nObjects = 1;

	CSkyBox *pSkyBox = new CSkyBox( pd3dDevice );
	m_ppObjects.push_back( pSkyBox );
}

void CSkyBoxShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::OnPrepareRender( pd3dDeviceContext );

	for (int i = 0; i < m_nObjects; i++)
		m_ppObjects[i]->Render( pd3dDeviceContext, pCamera );
}