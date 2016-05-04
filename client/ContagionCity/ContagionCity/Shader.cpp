#include "stdafx.h"
#include "Shader.h"

// ���� ��ȯ ����� ���� ��� ���۴� ���̴� ��ü�� ���� ������ ����̴�.
// ����ƽ ��� ������ ������ �̷��� �����Ѵ�.
ID3D11Buffer *CShader::m_pd3dcbWorldMatrix = NULL;
ID3D11Buffer *CIlluminatedShader::m_pd3dcbMaterial = NULL;
ID3D11Buffer *CPlayerShader::m_pd3dcbOffsetMatrix = NULL;

CShader::CShader( )
{
	m_ppObjects = NULL;
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
	if (m_ppObjects)
	{
		for (int i = 0; i < m_nObjects; i++)
			if (m_ppObjects[i])
				delete m_ppObjects[i];
		delete[ ] m_ppObjects;
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
		// �����ϵ� ���̴� �ڵ��� �޸� �ּҿ��� ���� ���̴��� ����
		pd3dDevice->CreateVertexShader( pd3dShaderBlob->GetBufferPointer( ), pd3dShaderBlob->GetBufferSize( ), NULL, ppd3dVertexShader );
		// �����ϵ� ���̴� �ڵ��� �޸� �ּҿ� �Է� ���̾ƿ����� ���� ���̾ƿ��� ����
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
		// �����ϵ� ���̴� �ڵ��� �޸� �ּҿ��� �ȼ� ���̴��� ����
		pd3dDevice->CreatePixelShader( pd3dShaderBlob->GetBufferPointer( ), pd3dShaderBlob->GetBufferSize( ), NULL, ppd3dPixelShader );
		pd3dShaderBlob->Release( );
	}
}

void CShader::CreateShader( ID3D11Device *pd3dDevice )
{
	/*IA �ܰ迡 ������ �Է�-���̾ƿ��� �����Ѵ�.
	���� ������ �� ���Ұ� CVertex Ŭ������ ��� ����(D3DXVECTOR3 ��, �Ǽ� �� ��)�̹Ƿ�
	�Է�-���̾ƿ��� �Ǽ�(32-��Ʈ) 3���� �����Ǹ� �ø�ƽ�� ��POSITION���̰� ���� ���������� ǥ���Ѵ�.*/
	//�ϳ��� ������ ��ġ ����(�Ǽ� 3��)�� ����(�Ǽ� 4��)�� �����Ѵ�. 
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputLayout );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VS", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PS", "ps_5_0", &m_pd3dPixelShader );
	// ���� ��ȯ ����� ���� ������� ����
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
	// ���� ��ȯ ����� ���� ��� ���۸� ����
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
	// ���� ��ȯ ����� ��� ���ۿ� ����
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbWorldMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	VS_CB_WORLD_MATRIX *pcbWorldMatrix = (VS_CB_WORLD_MATRIX*)d3dMappedResource.pData;
	pcbWorldMatrix->m_mtxWorld =  MathHelper::GetInstance( )->TransposeFloat4x4( *pmtxWorld );
	pd3dDeviceContext->Unmap( m_pd3dcbWorldMatrix, 0 );
	
	// ��� ���۸� ����̽��� ���Կ� ����
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix );
}

CPlayerShader::CPlayerShader( )
{
}

CPlayerShader::~CPlayerShader( )
{
}
void CPlayerShader::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	// �ִϸ��̼� ��ȯ ����� ���� ��� ���۸� ����
	D3D11_BUFFER_DESC bd;
	::ZeroMemory( &bd, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( VS_CB_OFFSET_MATRIX );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer( &bd, NULL, &m_pd3dcbOffsetMatrix );
}

void CPlayerShader::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, std::vector<XMFLOAT4X4> pmtxWorld )
{
	// ���� ��ȯ ����� ��� ���ۿ� ����
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbOffsetMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	VS_CB_OFFSET_MATRIX *pcbOffsetMatrix = (VS_CB_OFFSET_MATRIX*)d3dMappedResource.pData;
	for (int i = 0; i < pmtxWorld.size( ); i++){
		pcbOffsetMatrix->m_mtxOffsets[i] = MathHelper::GetInstance( )->TransposeFloat4x4( pmtxWorld[i] );
	}
	pd3dDeviceContext->Unmap( m_pd3dcbOffsetMatrix, 0 );

	// ��� ���۸� ����̽��� ���Կ� ����
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_OFFSET_MATRIX, 1, &m_pd3dcbOffsetMatrix );
}

void CPlayerShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// �������� �ϸ� �̰� �ʿ����
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "SkinnedVS", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "SkinnedPS", "ps_5_0", &m_pd3dPixelShader );

	CreateShaderVariables( pd3dDevice );
//	CShader::CreateShader( pd3dDevice );
}

void CPlayerShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes )
{
	m_nObjects = meshes.size();
	m_ppObjects = new CGameObject*[m_nObjects];

	CPlayer *pPlayer = new CPlayer( );

	for (int i = 0; i < m_nObjects; i++)
	{
		int textureCount = meshes[i].m_pTextures.size( );
		CAnimatedMesh *pPlayerMesh = new CAnimatedMesh( pd3dDevice, meshes[i], 1 );
		pPlayerMesh->FindMinMax( );		// AABB ���� �� ����
		for (int j = 0; j < textureCount; j++)
			pPlayerMesh->OnChangeTexture( pd3dDevice, meshes[i].m_pTextures[j], j );
		pPlayer->SetMesh( pPlayerMesh, i );
	}
	pPlayer->CreateShaderVariables( pd3dDevice );
	pPlayer->ChangeCamera( pd3dDevice, THIRD_PERSON_CAMERA, 0.0f );
	pPlayer->GetCamera( )->Rotate( 0.0f, 180.0f, 0.0f );
	pPlayer->SetPosition( XMFLOAT3( 0.0f, 1.0f, 0.0f ) );

	CMaterial *pPlayerMaterial = new CMaterial;
	pPlayer->SetMaterial( pPlayerMaterial );

	m_ppObjects[0] = pPlayer;
}

void CPlayerShader::AnimateObjects( float fTimeElapsed )
{
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->Animate( fTimeElapsed );
	}
}

bool CPlayerShader::CollisionCheck( CGameObject* pObject )
{
	return false;
}

void CPlayerShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	
	// 3��Ī ī�޶��� �� �÷��̾ ������
	DWORD nCameraMode = ( pCamera ) ? pCamera->GetMode( ) : 0x00;

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		UpdateShaderVariable( pd3dDeviceContext, m_ppObjects[0]->m_pmtxFinalTransforms);
		CShader::Render( pd3dDeviceContext, pCamera );
	}
}

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
	m_nObjects = meshes[0].m_skinnedData.BoneCount( ); 
	m_ppObjects = new CGameObject*[m_nObjects];

	m_skinnedData = meshes[0].m_skinnedData;

	for (int i = 0; i < m_nObjects; i++)
	{
		CCubeMeshDiffused *pBoneMesh = new CCubeMeshDiffused( pd3dDevice, 5, 5, 5, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 0.0f ) );
		CGameObject *pBone = new CGameObject( 1 );

		pBone->SetMesh( pBoneMesh, 0 );
		m_ppObjects[i] = pBone;
	}
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
	// 3��Ī ī�޶��� �� �÷��̾ ������
	DWORD nCameraMode = ( pCamera ) ? pCamera->GetMode( ) : 0x00;

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		CShader::Render( pd3dDeviceContext, pCamera );
	}
}

CBackgroundShader::CBackgroundShader( )
{

}
CBackgroundShader::~CBackgroundShader( )
{

}

void CBackgroundShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// �������� �ϸ� �̰� �ʿ����
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader );
}

void CBackgroundShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex )
{
	m_nObjects = vertex.size( );
	m_ppObjects = new CGameObject*[m_nObjects];

	for (int i = 0; i < m_nObjects; i++)
	{
		/*switch (vertex[i].m_iType)
		{
			case BACK_GROUND:
			{*/
				int textureCount = vertex[i].m_pTextures.size( );
				ObjectInfo *pGround = new ObjectInfo( pd3dDevice, vertex[i] );
				CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], textureCount );		// ��ָ����� ���� �ؽ�ó 2�� ���
				pGroundMesh->FindMinMax( );		// AABB �� ����
				for (int j = 0; j < textureCount; j++)
					pGroundMesh->OnChangeTexture( pd3dDevice, vertex[i].m_pTextures[j], j );
				pGround->SetMesh( pGroundMesh, 0 );
				pGround->m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( pGround->m_mtxWorld, XMFLOAT4X4( 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 ) );
				m_ppObjects[i] = pGround;

				CMaterial *pGroundMaterial = new CMaterial;
				m_ppObjects[i]->SetMaterial( pGroundMaterial );
			//	break;
			//}
			//case BACK_SHOP:
			//{
			//	ObjectInfo *pShop = new ObjectInfo( pd3dDevice, vertex[i] );
			//	CObjectMesh *pShopMesh = new CObjectMesh( pd3dDevice, vertex[i], 2 );		// ��ָ����� ���� �ؽ�ó 2�� ���
			//	pShopMesh->FindMinMax( );		// AABB �� ����
			//	pShopMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_texture.dds" ), 0 );
			//	pShopMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_normal.dds" ), 1 );
			//	pShop->SetMesh( pShopMesh, 0 );
			//	m_ppObjects[i] = pShop;

			//	CMaterial *pFenceMaterial = new CMaterial;
			//	m_ppObjects[i]->SetMaterial( pFenceMaterial );
			//	break;
			//}
			//case BACK_FENCE:
			//{
			//	ObjectInfo *pFence = new ObjectInfo( pd3dDevice, vertex[i] );
			//	CObjectMesh *pFenceMesh = new CObjectMesh( pd3dDevice, vertex[i], 2 );		// ��ָ����� ���� �ؽ�ó 2�� ���
			//	pFenceMesh->FindMinMax( );		// AABB �� ����
			//	pFenceMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_texture.dds" ), 0 );
			//	pFenceMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_normal.dds" ), 1 );
			//	pFence->SetMesh( pFenceMesh, 0 );
			//	m_ppObjects[i] = pFence;

			//	CMaterial *pFenceMaterial = new CMaterial;
			//	m_ppObjects[i]->SetMaterial( pFenceMaterial );
			//	break;
			//}
			//case BACK_SHALTER:
			//{
			//	ObjectInfo *pShalter = new ObjectInfo( pd3dDevice, vertex[i] );
			//	CObjectMesh *pShelterMesh = new CObjectMesh( pd3dDevice, vertex[i], 2 );		// ��ָ����� ���� �ؽ�ó 2�� ���
			//	pShelterMesh->FindMinMax( );		// AABB �� ����
			//	pShelterMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_texture.dds" ), 0 );
			//	pShelterMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_normal.dds" ), 1 );
			//	pShalter->SetMesh( pShelterMesh, 0 );
			//	m_ppObjects[i] = pShalter;

			//	CMaterial *pFenceMaterial = new CMaterial;
			//	m_ppObjects[i]->SetMaterial( pFenceMaterial );
			//	break;
			//}
			//case BACK_WALL:
			//{
			//	ObjectInfo *pWall = new ObjectInfo( pd3dDevice, vertex[i] );
			//	CObjectMesh *pWallMesh = new CObjectMesh( pd3dDevice, vertex[i], 2 );		// ��ָ����� ���� �ؽ�ó 2�� ���
			//	pWallMesh->FindMinMax( );		// AABB �� ����
			//	pWallMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_texture.dds" ), 0 );
			//	pWallMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_0314_normal.dds" ), 1 );
			//	pWall->SetMesh( pWallMesh, 0 );
			//	m_ppObjects[i] = pWall;

			//	CMaterial *pFenceMaterial = new CMaterial;
			//	m_ppObjects[i]->SetMaterial( pFenceMaterial );
			//	break;
			//}
		//	default:
		//		// ����ó��
		//		break;
		//}
		

		
	}
}

void CBackgroundShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::Render( pd3dDeviceContext, pCamera );
}

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

CSkyBoxShader::CSkyBoxShader( )
{
}

CSkyBoxShader::~CSkyBoxShader( )
{
}

void CSkyBoxShader::BuildObjects( ID3D11Device *pd3dDevice )
{
	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	CSkyBox *pSkyBox = new CSkyBox( pd3dDevice );
	m_ppObjects[0] = pSkyBox;
}

void CSkyBoxShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::OnPrepareRender( pd3dDeviceContext );

	for (int i = 0; i < m_nObjects; i++)
		m_ppObjects[i]->Render( pd3dDeviceContext, pCamera );
}