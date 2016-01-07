#include "stdafx.h"
#include "Scene.h"

CScene::CScene( )
{
	m_ppShaders = NULL;
	m_nShaders = 0;

	m_pCamera = NULL;

	m_pLights = NULL;
	m_pd3dcbLights = NULL;
}

CScene::~CScene( )
{
}

void CScene::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	m_pLights = new LIGHTS;
	::ZeroMemory( m_pLights, sizeof( LIGHTS ) );
	// 월드 전체를 비추는 주변 조명을 설정
	m_pLights->m_cGlobalAmbient = XMCOLOR( 0.1f, 0.1f, 0.1f, 1.0f );

	// 태양
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

	// 텍스처 리소스를 생성
	/*ID3D11ShaderResourceView *pd3dsrvTexture = NULL;
	CTexture *pStoneTexture = new CTexture( 1, 1, 0, 0 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, _T( "Stone01.jpg" ), NULL, NULL, &pd3dsrvTexture, NULL );
	pStoneTexture->SetTexture( 0, pd3dsrvTexture );
	pStoneTexture->SetSampler( 0, pd3dSamplerState );
	pd3dsrvTexture->Release( );
	pd3dSamplerState->Release( );*/

	m_nShaders = 1;
	m_ppShaders = new CShader*[m_nShaders];

	// 첫번째로 그릴 객체는 스카이박스
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	CreateShaderVariables( pd3dDevice );
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