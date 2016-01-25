#include "stdafx.h"
#include "Scene.h"

CScene::CScene( )
{
	m_ppShaders = NULL;
	m_nShaders = 0;

	m_pCamera = NULL;

	m_pLights = NULL;
	m_pd3dcbLights = NULL;

	m_pFbxLoader = new FBXManager;
	m_nFbxCount = 0;
	m_pMeshes = NULL;
	m_pMeshCount = NULL;
}

CScene::~CScene( )
{
	delete m_pFbxLoader;
	if (m_pMeshCount)
		delete[ ] m_pMeshCount;
	if (m_pMeshes)
		delete[ ] m_pMeshes;
}

void CScene::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	m_pLights = new LIGHTS;
	::ZeroMemory( m_pLights, sizeof( LIGHTS ) );
	// ���� ��ü�� ���ߴ� �ֺ� ������ ����
	m_pLights->m_cGlobalAmbient = XMCOLOR( 0.1f, 0.1f, 0.1f, 1.0f );

	// �¾�
	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[0].m_cAmbient = XMCOLOR( 0.2f, 0.2f, 0.2f, 1.0f );
	m_pLights->m_pLights[0].m_cDiffuse = XMCOLOR( 0.4f, 0.4f, 0.4f, 1.0f );
	m_pLights->m_pLights[0].m_cSpecular = XMCOLOR( 0.0f, 0.0f, 0.0f, 0.0f );
	m_pLights->m_pLights[0].m_vDirection = XMFLOAT3( 0.0f, -1.0f, 0.0f );

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( d3dBufferDesc ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof( LIGHTS );
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_SUBRESOURCE_DATA ) );
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
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	LIGHTS *pcbLights = (LIGHTS *)d3dMappedResource.pData;
	memcpy( pcbLights, pLights, sizeof( LIGHTS ) );
	pd3dDeviceContext->Unmap( m_pd3dcbLights, 0 );
	pd3dDeviceContext->PSSetConstantBuffers( PS_SLOT_LIGHT, 1, &m_pd3dcbLights );
}

void CScene::BuildObjects( ID3D11Device *pd3dDevice )
{
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

	// �ؽ�ó ���ҽ��� ����
	/*ID3D11ShaderResourceView *pd3dsrvTexture = NULL;
	CTexture *pStoneTexture = new CTexture( 1, 1, 0, 0 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, _T( "Stone01.jpg" ), NULL, NULL, &pd3dsrvTexture, NULL );
	pStoneTexture->SetTexture( 0, pd3dsrvTexture );
	pStoneTexture->SetSampler( 0, pd3dSamplerState );
	pd3dsrvTexture->Release( );
	pd3dSamplerState->Release( );*/

	// fbx loader�� ���� fbx�ε�
	LoadFBXs( );

	// ���� ���̴� ������ fbx������ ���� �� ���̴��� �����Ͽ� ����
	m_nShaders = 1 + m_nFbxCount;
	m_ppShaders = new CShader*[m_nShaders];

	// ù��°�� �׸� ��ü�� ��ī�̹ڽ�
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	for (int i = 1; i < m_nShaders; i++)
	{
		// fbx �׸� �غ��ϱ�
		m_ppShaders[i] = new CShader( );
		m_ppShaders[i]->CreateShader( pd3dDevice );
		m_ppShaders[i]->BuildObjects( pd3dDevice );
		///// ���� �Ž��� ���� ���س��Ƽ� ȭ�鿡 �ƹ��͵� �ȳ�Ÿ��
	}

	CreateShaderVariables( pd3dDevice );
}

void CScene::LoadFBXs( )
{
	// fbx ���� �ε�
	m_pFbxLoader->LoadFBX( "City_Base.FBX" );
	m_nFbxCount = m_pFbxLoader->getMeshCount( );

	// ���� ��ȯ
	m_pMeshes = new CMesh[m_nFbxCount];
	m_pMeshCount = new int[m_nFbxCount];

	// pMeshes�� ���� ��ǥ����, pMeshCount�� ������ �������� ����
	m_pFbxLoader->LoadVertex( m_pMeshes, m_pMeshCount );
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
	return false;
}

bool CScene::OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	return false;
}

bool CScene::ProcessInput( )
{
	return false;
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