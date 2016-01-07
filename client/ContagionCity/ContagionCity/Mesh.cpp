#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"

CMesh::CMesh( ID3D11Device *pd3dDevice )
{
	m_pvPositions = NULL;
	m_pnIndices = NULL;

	m_nBuffers = 0;
	m_pd3dPositionBuffer = NULL;
	m_ppd3dVertexBuffers = NULL;

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nSlot = 0;
	m_nStartVertex = 0;

	m_pd3dIndexBuffer = NULL;
	m_nIndices = 0;
	m_nStartIndex = 0;
	m_nIndexOffset = 0;
	m_dxgiIndexFormat = DXGI_FORMAT_R32_UINT;

	m_pd3dRasterizerState = NULL;

	m_bcBoundingCube.m_vMin = XMFLOAT3( +FLT_MAX, +FLT_MAX, +FLT_MAX );
	m_bcBoundingCube.m_vMax = XMFLOAT3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	m_nReferences = 0;
}


CMesh::~CMesh( )
{
	if (m_pvPositions)	delete[ ] m_pvPositions;
	if (m_pnIndices)	delete[ ] m_pnIndices;
	
	if (m_pd3dRasterizerState)	m_pd3dRasterizerState->Release( );
	if (m_pd3dPositionBuffer)	m_pd3dPositionBuffer->Release( );
	if (m_pd3dIndexBuffer)		m_pd3dIndexBuffer->Release( );

	if (m_ppd3dVertexBuffers)	delete[ ] m_ppd3dVertexBuffers;
	if (m_pnVertexOffsets)		delete[ ] m_pnVertexOffsets;
	if (m_pnVertexStrides)		delete[ ] m_pnVertexStrides;
}

// 정점 버퍼, 색깔 버퍼, 포지션 버퍼 배열로 합침
void CMesh::AssembleToVertexBuffer( int nBuffers, ID3D11Buffer **ppd3dBuffers, UINT *pnBufferStrides, UINT * pnBufferOffsets )
{
	ID3D11Buffer **ppd3dNewVertexBuffers = new ID3D11Buffer*[m_nBuffers + nBuffers];
	UINT *pnNewVertexStrides = new UINT[m_nBuffers + nBuffers];
	UINT *pnNewVertexOffsets = new UINT[m_nBuffers + nBuffers];
	
	if (m_nBuffers > 0)
	{
		for (int i = 0; i < m_nBuffers; i++)
		{
			ppd3dNewVertexBuffers[i] = m_ppd3dVertexBuffers[i];
			pnNewVertexStrides[i] = m_pnVertexStrides[i];
			pnNewVertexOffsets[i] = m_pnVertexOffsets[i];
		}
		if (m_ppd3dVertexBuffers)	delete[ ] m_ppd3dVertexBuffers;
		if (m_pnVertexStrides)		delete[ ] m_pnVertexStrides;
		if (m_pnVertexOffsets)		delete[ ] m_pnVertexOffsets;
	}
	
	for (int i = 0; i < nBuffers; i++)
	{
		ppd3dNewVertexBuffers[m_nBuffers + i ] = ppd3dBuffers[i];
		pnNewVertexStrides[m_nBuffers + i] = pnBufferStrides[i];
		pnNewVertexOffsets[m_nBuffers + i] = pnBufferOffsets[i];
	}

	m_nBuffers += nBuffers;
	m_ppd3dVertexBuffers = ppd3dNewVertexBuffers;
	m_pnVertexStrides = pnNewVertexStrides; 
	m_pnVertexOffsets = pnNewVertexOffsets;
}

void CMesh::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	// 메시의 정점은 여러개의 정점 버퍼로 표현
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets );
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );
	pd3dDeviceContext->RSSetState( m_pd3dRasterizerState );

	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexed( m_nIndices, m_nStartIndex, m_nBaseVertex );
	else
		pd3dDeviceContext->Draw( m_nVertices, m_nStartVertex );
}

void CMesh::RenderInstanced( ID3D11DeviceContext *pd3dDeviceContext, int nInstances, int nStartInstance )
{
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets );
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );
	pd3dDeviceContext->RSSetState( m_pd3dRasterizerState );

	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexedInstanced( m_nIndices, nInstances, m_nStartIndex, m_nBaseVertex, nStartInstance );
	else
		pd3dDeviceContext->DrawInstanced( m_nVertices, nInstances, m_nStartVertex, nStartInstance );
}

// AABB

void AABB::Union( XMFLOAT3& vMin, XMFLOAT3& vMax )
{
	if (vMin.x < m_vMin.x) m_vMin.x = vMin.x;
	if (vMin.y < m_vMin.y) m_vMin.y = vMin.y;
	if (vMin.z < m_vMin.z) m_vMin.z = vMin.z;
	if (vMax.x > m_vMax.x) m_vMax.x = vMax.x;
	if (vMax.y > m_vMax.y) m_vMax.y = vMax.y;
	if (vMax.z > m_vMax.z) m_vMax.z = vMax.z;
}

void AABB::Union( AABB *pAABB )
{
	Union( pAABB->m_vMin, pAABB->m_vMax );
}

void AABB::Update( XMFLOAT4X4 *pmtxTransform )
{
	XMFLOAT3 vVertices[8];
	vVertices[0] = XMFLOAT3( m_vMin.x, m_vMin.y, m_vMin.z );
	vVertices[1] = XMFLOAT3( m_vMin.x, m_vMin.y, m_vMax.z );
	vVertices[2] = XMFLOAT3( m_vMax.x, m_vMin.y, m_vMax.z );
	vVertices[3] = XMFLOAT3( m_vMax.x, m_vMin.y, m_vMin.z );
	vVertices[4] = XMFLOAT3( m_vMin.x, m_vMax.y, m_vMin.z );
	vVertices[5] = XMFLOAT3( m_vMin.x, m_vMax.y, m_vMax.z );
	vVertices[6] = XMFLOAT3( m_vMax.x, m_vMax.y, m_vMax.z );
	vVertices[7] = XMFLOAT3( m_vMax.x, m_vMax.y, m_vMin.z );
	m_vMin = XMFLOAT3( +FLT_MAX, +FLT_MAX, +FLT_MAX );
	m_vMax = XMFLOAT3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	// 8개의 정점에서 x,y,z 좌표의 최소값 최대값을 구한다.
	for (int i = 0; i < 8; i++)
	{
		vVertices[i] = MathHelper::GetInstance( )->Vector3TransformCoord( vVertices[i], *pmtxTransform );
		if (vVertices[i].x < m_vMin.x) m_vMin.x = vVertices[i].x;
		if (vVertices[i].y < m_vMin.y) m_vMin.y = vVertices[i].y;
		if (vVertices[i].z < m_vMin.z) m_vMin.z = vVertices[i].z;
		if (vVertices[i].x > m_vMax.x) m_vMax.x = vVertices[i].x;
		if (vVertices[i].y > m_vMax.y) m_vMax.y = vVertices[i].y;
		if (vVertices[i].z > m_vMax.z) m_vMax.z = vVertices[i].z;
	}
}

CMeshDiffused::CMeshDiffused( ID3D11Device *pd3dDevice ) : CMesh( pd3dDevice )
{
	m_pd3dColorBuffer = NULL;
}

CMeshDiffused::~CMeshDiffused( )
{
	if (m_pd3dColorBuffer)
		m_pd3dColorBuffer->Release( );
}

CCubeMeshDiffused::CCubeMeshDiffused( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, XMCOLOR color ) : CMeshDiffused( pd3dDevice )
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	float fx = fWidth *0.5f, fy = fHeight * 0.5f, fz = fDepth *0.5f;

	// 직육면체 메시는 2개의 정점 버퍼로 구성
	// 직육면체 메시의 정점 버퍼 생성
	m_pvPositions = new XMFLOAT3[m_nVertices];
	m_pvPositions[0] = XMFLOAT3( -fx, +fy, -fz );
	m_pvPositions[1] = XMFLOAT3( +fx, +fy, -fz );
	m_pvPositions[2] = XMFLOAT3( +fx, +fy, +fz );
	m_pvPositions[3] = XMFLOAT3( -fx, +fy, +fz );
	m_pvPositions[4] = XMFLOAT3( -fx, -fy, -fz );
	m_pvPositions[5] = XMFLOAT3( +fx, -fy, -fz );
	m_pvPositions[6] = XMFLOAT3( +fx, -fy, +fz );
	m_pvPositions[7] = XMFLOAT3( -fx, -fy, +fz );

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pvPositions;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer );

	// 직육면체 메시의 정점 버퍼 생성
	XMCOLOR pColors[8];
	for (int i = 0; i < 8; i++)
		pColors[i] = color + RANDOM_COLOR;

	d3dBufferDesc.ByteWidth = sizeof( XMCOLOR ) * m_nVertices;
	d3dBufferData.pSysMem = pColors;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer );

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof( XMFLOAT3 ), sizeof( XMCOLOR ) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer( 2, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	m_nIndices = 18;
	m_pnIndices = new UINT[m_nIndices];
	m_pnIndices[0] = 5;		// 5 6 4 cw
	m_pnIndices[1] = 6;		// 6 4 7 ccw
	m_pnIndices[2] = 4;		// 4 7 0 cw
	m_pnIndices[3] = 7;		// 7 0 3 ccw
	m_pnIndices[4] = 0;		// 0 3 1 cw
	m_pnIndices[5] = 3;		// 3 1 2 ccw
	m_pnIndices[6] = 1;		// 1 2 2 cw
	m_pnIndices[7] = 2;		// 2 2 3 ccw
	m_pnIndices[8] = 2;		// 2 3 3 cw
	m_pnIndices[9] = 3;		// 3 3 7 ccw
	m_pnIndices[10] = 3;	// 3 7 2 cw
	m_pnIndices[11] = 7;	// 7 2 6 ccw
	m_pnIndices[12] = 2;	// 2 6 1 cw
	m_pnIndices[13] = 6;	// 6 1 5 ccw
	m_pnIndices[14] = 1;	// 1 5 0 cw
	m_pnIndices[15] = 5;	// 5 0 4 ccw
	m_pnIndices[16] = 0;
	m_pnIndices[17] = 4;

	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer );
	
	CreateRasterizerState( pd3dDevice );

	m_bcBoundingCube.m_vMin = XMFLOAT3( -fx, -fy, -fz );
	m_bcBoundingCube.m_vMax = XMFLOAT3( +fx, +fy, +fz );
}

CCubeMeshDiffused::~CCubeMeshDiffused( )
{

}

void CCubeMeshDiffused::CreateRasterizerState( ID3D11Device *pd3dDevice )
{
	D3D11_RASTERIZER_DESC d3dRastersizerDesc;
	::ZeroMemory( &d3dRastersizerDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	d3dRastersizerDesc.CullMode = D3D11_CULL_BACK;
	// 솔리드와 와이어 설정할 수 있음
	d3dRastersizerDesc.FillMode = D3D11_FILL_SOLID;
	pd3dDevice->CreateRasterizerState( &d3dRastersizerDesc, &m_pd3dRasterizerState );
}

void CCubeMeshDiffused::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	CMesh::Render( pd3dDeviceContext );
}

CMeshIlluminated::CMeshIlluminated( ID3D11Device *pd3dDevice ) : CMesh( pd3dDevice )
{
	m_pd3dNormalBuffer = NULL;
}

CMeshIlluminated::~CMeshIlluminated( )
{
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release( );
}

void CMeshIlluminated::CalculateVertexNormal( XMFLOAT3 *pvNormals )
{
	switch (m_d3dPrimitiveTopology)
	{
		/*프리미티브가 삼각형 리스트일 때 인덱스 버퍼가 있는 경우와 없는 경우를 구분하여 정점의 법선 벡터를 계산한다.t
		인덱스 버퍼를 사용하지 않는 경우 각 정점의 법선 벡터는 그 정점이 포함된 삼각형의 법선 벡터로 계산한다.
		인덱스 버퍼를 사용하는 경우 각 정점의 법선 벡터는 그 정점이 포함된 삼각형들의 법선 벡터의 평균으로(더하여) 계산한다.*/
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
			if (m_pnIndices)
				SetAverageVertexNormal( pvNormals, ( m_nIndices / 3 ), 3, false );
			else
				SetTriangleListVertexNormal( pvNormals );
			break;
			/*프리미티브가 삼각형 스트립일 때 각 정점의 법선 벡터는 그 정점이 포함된 삼각형들의 법선 벡터의 평균으로(더하여) 계산한다.*/
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
			SetAverageVertexNormal( pvNormals, ( m_nIndices ) ? ( m_nIndices - 2 ) : ( m_nVertices - 2 ), 1, true );
			break;
		default:
			break;
	}
}

// 인덱스 버퍼를 사용하지 않는 삼각형 리스트에 대하여 정점의 법선 벡터를 계산
void CMeshIlluminated::SetTriangleListVertexNormal( XMFLOAT3 *pvNormals )
{
	// 삼각형의 개수를 구하고 각 삼각형의 법선 벡터를 계산하고 삼각형을 구성하는 각 정점의 법선 벡터로 지정
	int nPrimitives = m_nVertices / 3;
	for (int i = 0; i < nPrimitives; i++)
	{
		XMFLOAT3 vNormal = CalculateTriangleNormal( ( i * 3 + 0 ), ( i * 3 + 1 ), ( i * 3 + 2 ) );
		pvNormals[i * 3 + 0] = pvNormals[i * 3 + 1] = pvNormals[i * 3 + 2] = vNormal;
	}
}

XMFLOAT3 CMeshIlluminated::CalculateTriangleNormal( UINT nIndex0, UINT nIndex1, UINT nIndex2 )
{
	XMFLOAT3 vNormal;
	XMFLOAT3 vP0 = m_pvPositions[nIndex0];
	XMFLOAT3 vP1 = m_pvPositions[nIndex1];
	XMFLOAT3 vP2 = m_pvPositions[nIndex2];
	XMFLOAT3 vEdge1 = MathHelper::GetInstance( )->Float3MinusFloat3( vP1, vP0 );
	XMFLOAT3 vEdge2 = MathHelper::GetInstance( )->Float3MinusFloat3( vP2, vP0 );

	vNormal = MathHelper::GetInstance( )->CrossFloat3( vEdge1, vEdge2 );
	vNormal = MathHelper::GetInstance( )->NormalizeFloat3( vNormal );

	return vNormal;
}

// 프리미티브가 인덱스 버퍼를 사용하는 삼각형 리스트 또는 스트립인 경우 정점의 법선 벡터는 그 정점을 포함하는 법선 벡터들의 평균으로 계산
void CMeshIlluminated::SetAverageVertexNormal( XMFLOAT3 *pvNormals, int nPrimitives, int nOffset, bool bStrip )
{
	for (int i = 0; i < m_nVertices; i++)
	{
		XMFLOAT3 vSumOfNormal = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		for (int j = 0; j < nPrimitives; j++)
		{
			UINT nIndex0 = ( bStrip ) ? ( ( ( j % 2 ) == 0 ) ? ( j*nOffset + 0 ) : ( j*nOffset + 1 ) ) : ( j*nOffset + 0 );
			if (m_pnIndices)
				nIndex0 = m_pnIndices[nIndex0];
			UINT nIndex1 = ( bStrip ) ? ( ( ( j % 2 ) == 0 ) ? ( j*nOffset + 1 ) : ( j*nOffset + 0 ) ) : ( j*nOffset + 1 );
			if (m_pnIndices)
				nIndex1 = m_pnIndices[nIndex1];
			UINT nIndex2 = ( m_pnIndices ) ? m_pnIndices[j*nOffset + 2] : ( j*nOffset + 2 );
			if (nIndex0 == i || nIndex1 == i || nIndex2 == i)
				MathHelper::GetInstance( )->Float3PlusFloat3( vSumOfNormal, CalculateTriangleNormal( nIndex0, nIndex1, nIndex2 ));
		}
		vSumOfNormal = MathHelper::GetInstance( )->NormalizeFloat3( vSumOfNormal );
		pvNormals[i] = vSumOfNormal;
	}
}

CCubeMeshIlluminated::CCubeMeshIlluminated( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth ) : CMeshIlluminated( pd3dDevice )
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth *0.5f, fy = fHeight*0.5f, fz = fDepth * 0.5f;

	m_pvPositions = new XMFLOAT3[m_nVertices];

	m_pvPositions[0] = XMFLOAT3( -fx, +fy, -fz );
	m_pvPositions[1] = XMFLOAT3( +fx, +fy, -fz );
	m_pvPositions[2] = XMFLOAT3( +fx, +fy, +fz );
	m_pvPositions[3] = XMFLOAT3( -fx, +fy, +fz );
	m_pvPositions[4] = XMFLOAT3( -fx, -fy, -fz );
	m_pvPositions[5] = XMFLOAT3( +fx, -fy, -fz );
	m_pvPositions[6] = XMFLOAT3( +fx, -fy, +fz );
	m_pvPositions[7] = XMFLOAT3( -fx, -fy, +fz );

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pvPositions;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer );

	m_nIndices = 36;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 3;		m_pnIndices[1] = 1;		m_pnIndices[2] = 0;
	m_pnIndices[3] = 2;		m_pnIndices[4] = 1;		m_pnIndices[5] = 3;
	m_pnIndices[6] = 0;		m_pnIndices[7] = 5;		m_pnIndices[8] = 4;
	m_pnIndices[9] = 1;		m_pnIndices[10] = 5;	m_pnIndices[11] = 0;
	m_pnIndices[12] = 3;	m_pnIndices[13] = 4;	m_pnIndices[14] = 7;
	m_pnIndices[15] = 0;	m_pnIndices[16] = 4;	m_pnIndices[17] = 3;
	m_pnIndices[18] = 1;	m_pnIndices[19] = 6;	m_pnIndices[20] = 5;
	m_pnIndices[21] = 2;	m_pnIndices[22] = 6;	m_pnIndices[23] = 1;
	m_pnIndices[24] = 2;	m_pnIndices[25] = 7;	m_pnIndices[26] = 6;
	m_pnIndices[27] = 3;	m_pnIndices[28] = 7;	m_pnIndices[29] = 2;
	m_pnIndices[30] = 6;	m_pnIndices[31] = 4;	m_pnIndices[32] = 5;
	m_pnIndices[33] = 7;	m_pnIndices[34] = 4;	m_pnIndices[35] = 6;

	XMFLOAT3 pvNormals[8];
	CalculateVertexNormal( pvNormals );

	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * m_nVertices;
	d3dBufferData.pSysMem = pvNormals;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer );

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer };
	UINT pnBufferStirdes[2] = { sizeof( XMFLOAT3 ), sizeof( XMFLOAT3 ) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer( 2, pd3dBuffers, pnBufferStirdes, pnBufferOffsets );

	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer );

	m_bcBoundingCube.m_vMin = XMFLOAT3( -fx, -fy, -fz );
	m_bcBoundingCube.m_vMax = XMFLOAT3( +fx, +fy, +fz );
}

CCubeMeshIlluminated::~CCubeMeshIlluminated( )
{

}


CMeshTextured::CMeshTextured( ID3D11Device *pd3dDevice ) : CMesh( pd3dDevice )
{
	m_pd3dTexCoordBuffer = NULL;
}

CMeshTextured::~CMeshTextured( )
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release( );
}

CCubeMeshTextured::CCubeMeshTextured( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth ) : CMeshTextured( pd3dDevice )
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth *0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	m_pvPositions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 pvTexCoords[36];
	int i = 0;

	// 직육면체의 각 면에 하나의 텍스처 이미지 전체가 맵핑되도록 텍스처 좌표 설정
	m_pvPositions[i] = XMFLOAT3( -fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( -fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( +fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );

	m_pvPositions[i] = XMFLOAT3( +fx, +fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, +fz );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fy, -fz );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pvPositions;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer );

	d3dBufferData.pSysMem = pvTexCoords;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer );

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof( XMFLOAT3 ), sizeof( XMFLOAT2 ) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer( 2, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	/*직육면체의 각 면에 텍스쳐를 맵핑하려면 인덱스를 사용할 수 없으므로 인덱스 버퍼는 생성하지 않는다.*/

	m_bcBoundingCube.m_vMin = XMFLOAT3( -fx, -fy, -fz );
	m_bcBoundingCube.m_vMax = XMFLOAT3( +fx, +fy, +fz );
}

CCubeMeshTextured::~CCubeMeshTextured( )
{

}

CSkyBoxMesh::CSkyBoxMesh( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth ) : CMeshTextured( pd3dDevice )
{
	//스카이 박스는 6개의 면(사각형), 사각형은 정점 4개, 그러므로 24개의 정점이 필요하다.
	m_nVertices = 24;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pvPositions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *pvTexCoords = new XMFLOAT2[m_nVertices];

	int i = 0;
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;
	// Front Quad 
	m_pvPositions[i] = XMFLOAT3( -fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Back Quad
	m_pvPositions[i] = XMFLOAT3( +fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Left Quad
	m_pvPositions[i] = XMFLOAT3( -fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Right Quad
	m_pvPositions[i] = XMFLOAT3( +fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Top Quad
	m_pvPositions[i] = XMFLOAT3( -fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Bottom Quad
	m_pvPositions[i] = XMFLOAT3( -fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_pvPositions[i] = XMFLOAT3( +fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_pvPositions[i] = XMFLOAT3( -fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pvPositions;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer );

	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT2 ) * m_nVertices;
	d3dBufferData.pSysMem = pvTexCoords;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer );

	delete[ ] pvTexCoords;

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof( XMFLOAT3 ), sizeof( XMFLOAT2 ) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer( 2, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	// 삼각형 스트립으로 사각형 1개를 그리기 위해서는 인덱스 4개가 필요
	m_nIndices = 4;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 0;
	m_pnIndices[1] = 1;
	m_pnIndices[2] = 3;
	m_pnIndices[3] = 2;

	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer );

	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory( &d3dDepthStencilDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	// 스카이 박스 사각형들은 깊이 버퍼 알고리즘을 적용하지 않고 깊이 버퍼를 변경하지 않음
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.StencilReadMask = 0xFF;
	d3dDepthStencilDesc.StencilWriteMask = 0xFF;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pd3dDevice->CreateDepthStencilState( &d3dDepthStencilDesc, &m_pd3dDepthStencilState );

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

	m_pSkyboxTexture = new CTexture( 6, 1, 1, 0 );
	m_pSkyboxTexture->SetSampler( 0, pd3dSamplerState );
	pd3dSamplerState->Release( );
	m_pSkyboxTexture->AddRef( );

	OnChangeSkyBoxTextures( pd3dDevice, 1 );
}

CSkyBoxMesh::~CSkyBoxMesh( )
{
	if (m_pd3dDepthStencilState)
		m_pd3dDepthStencilState->Release( );
	if (m_pSkyboxTexture)
		m_pSkyboxTexture->Release( );
}

void CSkyBoxMesh::OnChangeSkyBoxTextures( ID3D11Device *pd3dDevice, int nIndex )
{
	// 6개의 스카이 박스 텍스처를 생성하여 CTexture 객체에 연결
	_TCHAR pstrTextureName[80];
	ID3D11ShaderResourceView *pd3dsrvTexture = NULL;

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Front_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pSkyboxTexture->SetTexture( 0, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Back_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pSkyboxTexture->SetTexture( 1, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Left_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pSkyboxTexture->SetTexture( 2, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Right_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pSkyboxTexture->SetTexture( 3, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Top_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pSkyboxTexture->SetTexture( 4, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Bottom_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pSkyboxTexture->SetTexture( 5, pd3dsrvTexture );
	pd3dsrvTexture->Release( );
}

void CSkyBoxMesh::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );

	// 스카이 박스를 그리기 위한 샘플러 상태 객체와 깊이 스텐실 상태 객체를 설정
	m_pSkyboxTexture->UpdateSamplerShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( m_pd3dDepthStencilState, 1 );

	// 스카이 박스의 6개의 면을 순서대로 그린다.
	for (int i = 0; i < 6; i++)
	{
		// 스카이 박스의 각 면을 그릴 때 사용할 텍스처를 설정
		m_pSkyboxTexture->UpdateTextureShaderVariable( pd3dDeviceContext, 1, 0 );
		pd3dDeviceContext->DrawIndexed( 4, 0, i * 4 );
	}
	pd3dDeviceContext->OMSetDepthStencilState( NULL, 1 );
}

