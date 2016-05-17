#include "stdafx.h"
#include "Texture2D.h"
#include "Object.h"

CTexture2D::CTexture2D( ) : CMeshTextured( NULL )
{

}
CTexture2D::CTexture2D( ID3D11Device *pd3dDevice, const TCHAR *pFileName, float x, float y ) : CMeshTextured( pd3dDevice )
{
	m_x = x;
	m_y = y;

	m_nVertices = 4;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_vPositions.resize( m_nVertices );
	XMFLOAT2 *pvTexCoords = new XMFLOAT2[m_nVertices];

	int i = 0;
	m_vPositions[i] = XMFLOAT3( -x, +y, +0 );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +x, +y, +0 );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +x, -y, +0 );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( -x, -y, +0 );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = &m_vPositions[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer );

	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT2 ) * m_nVertices;
	d3dBufferData.pSysMem = pvTexCoords;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer );

	delete[ ] pvTexCoords;

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof( XMFLOAT3 ), sizeof( XMFLOAT2 ) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer( 2, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	m_nIndices = 4;
	m_vnIndices.resize( m_nIndices );

	m_vnIndices[0] = 0;
	m_vnIndices[1] = 3;
	m_vnIndices[2] = 1;
	m_vnIndices[3] = 2;

	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = &m_vnIndices[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer );

	ID3D11SamplerState *pd3dSamplerState = NULL;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory( &d3dSamplerDesc, sizeof( D3D11_SAMPLER_DESC ) );
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState( &d3dSamplerDesc, &pd3dSamplerState );

	m_pMeshTexture = new CTexture( 1, 1, 0, 0 );
	m_pMeshTexture->SetSampler( 0, pd3dSamplerState );
	pd3dSamplerState->Release( );
	m_pMeshTexture->AddRef( );

	_TCHAR pstrTextureName[80];
	ID3D11ShaderResourceView *pd3dsrvTexture = NULL;

	_stprintf_s( pstrTextureName, pFileName, 0, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 0, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	CreateRasterizerState( pd3dDevice );
}


CTexture2D::~CTexture2D( )
{
}

void CTexture2D::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets );
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );
	pd3dDeviceContext->RSSetState( m_pd3dRasterizerState );

	m_pMeshTexture->UpdateTextureShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->DrawIndexed( m_nIndices, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( NULL, 1 );
}

CTexture2DObject::CTexture2DObject( ID3D11Device *pd3dDevice, const TCHAR *pFileName, float x, float y ) : CGameObject( 1 )
{
	CTexture2D *pLoadingTexture = new CTexture2D( pd3dDevice, pFileName, x, y );
	SetMesh( pLoadingTexture, 0 );

	SetPosition( 0.f, 0.f, 0.f );
	
}
CTexture2DObject::~CTexture2DObject( )
{
}

CTexture2DShader::CTexture2DShader( ID3D11Device *pd3dDevice, const TCHAR *pFileName, float x, float y )
{
	CreateShader( pd3dDevice );

	CTexture2DObject *m_pLoadingTexture = new CTexture2DObject( pd3dDevice, pFileName, x,y );
	m_ppObjects.push_back( m_pLoadingTexture );
	m_nObjects = 1;
}

CTexture2DShader::~CTexture2DShader( )
{
}

void CTexture2DShader::MakeTransformToCamera( CCamera *pCamera, XMFLOAT3 pos )
{
//	m_ppObjects[0]->SetPosition( pos );
//	XMFLOAT4X4 mtxLookAt = MathHelper::GetInstance( )->MatrixLookAtLH( m_ppObjects[0]->GetPosition( ), pCamera->GetPosition( ), XMFLOAT3(0.0f, 1.0f, 0.0f) );

	XMFLOAT4X4 matView = pCamera->GetViewMatrix( );
	D3DXMATRIXA16   matBillboard; // 빌보드에 사용될 행렬
	D3DXMatrixIdentity( &matBillboard ); // 단위 행렬로 만든다
	
	matBillboard._11 = matView._11; // cosθ
	matBillboard._13 = matView._13; // -sinθ
	matBillboard._31 = matView._31; // sinθ
	matBillboard._33 = matView._33; // cosθ

	D3DXMatrixInverse( &matBillboard, NULL, &matBillboard );


	XMFLOAT4X4 mtx;
	mtx._11 = matBillboard._11; mtx._12 = matBillboard._12; mtx._13 = matBillboard._13; mtx._14 = matBillboard._14;
	mtx._21 = matBillboard._21; mtx._22 = matBillboard._22; mtx._23 = matBillboard._23; mtx._24 = matBillboard._24;
	mtx._31 = matBillboard._31; mtx._32 = matBillboard._32; mtx._33 = matBillboard._33; mtx._34 = matBillboard._34;
	mtx._41 = matBillboard._41; mtx._42 = matBillboard._42; mtx._43 = matBillboard._43; mtx._44 = matBillboard._44;

	m_ppObjects[0]->m_mtxWorld = mtx;
	m_ppObjects[0]->SetPosition( pos );
}