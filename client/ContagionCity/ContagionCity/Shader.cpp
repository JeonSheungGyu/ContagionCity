#include "stdafx.h"
#include "Shader.h"

// 월드 변환 행렬을 위한 상수 버퍼는 셰이더 객체의 정적 데이터 멤버이다.
// 스테틱 멤버 변수는 무조건 이렇게 선언한다.
ID3D11Buffer *CShader::m_pd3dcbWorldMatrix = NULL;
ID3D11Buffer *CIlluminatedShader::m_pd3dcbMaterial = NULL;

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
	pcbWorldMatrix->m_mtxWorld = MathHelper::GetInstance( )->TransposeFloat4x4( *pmtxWorld );
	pd3dDeviceContext->Unmap( m_pd3dcbWorldMatrix, 0 );
	
	// 상수 버퍼를 디바이스의 슬롯에 연결
	pd3dDeviceContext->VSSetConstantBuffers( VS_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix );
}

CDiffusedShader::CDiffusedShader( )
{
}

CDiffusedShader::~CDiffusedShader( )
{
}

void CDiffusedShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSDiffusedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSDiffusedColor", "ps_5_0", &m_pd3dPixelShader );
}

CPlayerShader::CPlayerShader( )
{
}

CPlayerShader::~CPlayerShader( )
{
}

void CPlayerShader::CreateShader( ID3D11Device *pd3dDevice )
{
	CShader::CreateShader( pd3dDevice );
}

void CPlayerShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes )
{
	m_nObjects = meshes.size();
	m_ppObjects = new CGameObject*[m_nObjects];

	CPlayer *pPlayer = new CPlayer( );

	for (int i = 0; i < m_nObjects; i++)
	{
		CObjectMesh *pPlayerMesh = new CObjectMesh( pd3dDevice, meshes[i], 1 );
		pPlayerMesh->OnChangeTexture( pd3dDevice, _T( "./SkyBox/SkyBox_Top_1.jpg" ), 0 );
		pPlayer->SetMesh( pPlayerMesh, i );
	}
	pPlayer->SetPosition( 0.0f, 0.0f, 50.0f );
	pPlayer->CreateShaderVariables( pd3dDevice );
	pPlayer->ChangeCamera( pd3dDevice, THIRD_PERSON_CAMERA, 0.0f );
	pPlayer->GetCamera( )->Rotate( 0.0f, 180.0f, 0.0f );
	m_ppObjects[0] = pPlayer;
}

void CPlayerShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	// 3인칭 카메라일 때 플레이어를 렌더링
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
	CTexturedShader::CreateShader( pd3dDevice );
}

void CBackgroundShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> vertex )
{
	m_nObjects = vertex.size();
	m_ppObjects = new CGameObject*[m_nObjects];

	for (int i = 0; i < m_nObjects; i++)
	{
		ObjectInfo *pGround = new ObjectInfo( pd3dDevice, vertex[i] );
		CObjectMesh *pGroundMesh = new CObjectMesh( pd3dDevice, vertex[i], 1 );
		pGroundMesh->OnChangeTexture( pd3dDevice, _T( "./res/city_base_texture.jpg" ), 0 );
		pGround->SetMesh( pGroundMesh, 0 );
		m_ppObjects[i] = pGround;
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

CDetailTexturedShader::CDetailTexturedShader( )
{
}

CDetailTexturedShader::~CDetailTexturedShader( )
{
}

void CDetailTexturedShader::CreateShader( ID3D11Device *pd3dDevice )
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[ ] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE( d3dInputElements );
	CreateVertexShaderFromFile( pd3dDevice, L"Effect.fx", "VSDetailTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout );
	CreatePixelShaderFromFile( pd3dDevice, L"Effect.fx", "PSDetailTexturedColor", "ps_5_0", &m_pd3dPixelShader );
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