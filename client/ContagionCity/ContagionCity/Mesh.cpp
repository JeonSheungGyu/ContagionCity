#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"
#include "MyFunction.h"

Keyframe::Keyframe( )
	: TimePos( 0.0f ),
	Translation( 0.0f, 0.0f, 0.0f ),
	Scale( 1.0f, 1.0f, 1.0f ),
	RotationQuat( 0.0f, 0.0f, 0.0f, 1.0f )
{
}

Keyframe::~Keyframe( )
{
}

float BoneAnimation::GetStartTime( )const
{
	return Keyframes.front( ).TimePos;
}

float BoneAnimation::GetEndTime( )const
{
	float f = Keyframes.back( ).TimePos;

	return f;
}

void BoneAnimation::Interpolate( float t, XMFLOAT4X4& M )
{
	if (t <= Keyframes.front( ).TimePos)
	{
		XMVECTOR S = XMLoadFloat3( &Keyframes.front( ).Scale );
		XMVECTOR P = XMLoadFloat3( &Keyframes.front( ).Translation );
		XMVECTOR Q = XMLoadFloat4( &Keyframes.front( ).RotationQuat );

		XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
		XMStoreFloat4x4( &M, XMMatrixAffineTransformation( S, zero, Q, P ) );
	}
	else if (t >= Keyframes.back( ).TimePos)
	{
		XMVECTOR S = XMLoadFloat3( &Keyframes.back( ).Scale );
		XMVECTOR P = XMLoadFloat3( &Keyframes.back( ).Translation );
		XMVECTOR Q = XMLoadFloat4( &Keyframes.back( ).RotationQuat );

		XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
		XMStoreFloat4x4( &M, XMMatrixAffineTransformation( S, zero, Q, P ) );
	}
	else
	{
		for (UINT i = 0; i < Keyframes.size( ) - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerpPercent = ( t - Keyframes[i].TimePos ) / ( Keyframes[i + 1].TimePos - Keyframes[i].TimePos );

				XMVECTOR s0 = XMLoadFloat3( &Keyframes[i].Scale );
				XMVECTOR s1 = XMLoadFloat3( &Keyframes[i + 1].Scale );

				XMVECTOR p0 = XMLoadFloat3( &Keyframes[i].Translation );
				XMVECTOR p1 = XMLoadFloat3( &Keyframes[i + 1].Translation );

				XMVECTOR q0 = XMLoadFloat4( &Keyframes[i].RotationQuat );
				XMVECTOR q1 = XMLoadFloat4( &Keyframes[i + 1].RotationQuat );

				XMVECTOR S = XMVectorLerp( s0, s1, lerpPercent );
				XMVECTOR P = XMVectorLerp( p0, p1, lerpPercent );
				XMVECTOR Q = XMQuaternionSlerp( q0, q1, lerpPercent );

				XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
				XMStoreFloat4x4( &M, XMMatrixAffineTransformation( S, zero, Q, P ) );

				break;
			}
		}
	}
}

BoneAnimation* AnimationClip::FindBone( int index )
{
	for (UINT i = 0; i < BoneAnimations.size( ); i++)
	{
		BoneAnimations[i].BoneIndex == index;
		return &BoneAnimations[i];
	}
}

BoneAnimation* AnimationClip::FindBone( std::string name )
{
	for (UINT i = 0; i < BoneAnimations.size( ); i++)
	{
		BoneAnimations[i].BoneName == name;
		return &BoneAnimations[i];
	}
}

float AnimationClip::GetClipStartTime( )const
{
	float t = 100000.f;
	for (UINT i = 0; i < BoneAnimations.size( ); ++i)
	{
		t = MathHelper::Min( t, BoneAnimations[i].GetStartTime( ) );
	}

	return t;
}

float AnimationClip::GetClipEndTime( )const
{
	float t = 0.0f;
	for (UINT i = 0; i < BoneAnimations.size( ); ++i)
	{
		t = MathHelper::Max( t, BoneAnimations[i].GetEndTime( ) );
	}

	return t;
}

void AnimationClip::Interpolate( float t, std::vector<XMFLOAT4X4>& boneTransforms )
{
	for (UINT i = 0; i < BoneAnimations.size( ); ++i)
	{
		BoneAnimations[i].Interpolate( t, boneTransforms[i] );
	}
}

float SkinnedData::GetClipStartTime( const int& clipName )const
{
	auto clip = mAnimations.find( clipName );
	return clip->second.GetClipStartTime( );
}

float SkinnedData::GetClipEndTime( const int& clipName )const
{
	auto clip = mAnimations.find( clipName );
	return clip->second.GetClipEndTime( );
}

UINT SkinnedData::BoneCount( )const
{
	return mBoneHierarchy.size( );
}

void SkinnedData::Set( std::vector<Bone>& boneHierarchy,
	std::vector<XMFLOAT4X4>& boneOffsets,
	std::map<int, AnimationClip>& animations )
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets = boneOffsets;
	mAnimations = animations;
}

void SkinnedData::GetMatrixByTime( const int& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms )
{
	// 이 클립의 모든 뼈대를 주어진 시간에 맞게 보간
	auto clip = mAnimations.find( clipName );
	clip->second.Interpolate( timePos, finalTransforms );
}

void SkinnedData::GetFinalTransforms( const int& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms )
{
	UINT numBones = mBoneOffsets.size( );

	std::vector<XMFLOAT4X4> toParentTransforms( numBones );

	// 이 클립의 모든 뼈대를 주어진 시간에 맞게 보간
	auto clip = mAnimations.find( clipName );
	clip->second.Interpolate( timePos, toParentTransforms );

	// 뼈대 계층구조를 훑으면서 모든 뼈대를 뿌리공간으로 변환
	std::vector<XMFLOAT4X4> toRootTransforms( numBones );

	toRootTransforms[0] = toParentTransforms[0];

	// 자식 뼈대들의 뿌리변환을 구함
	for (UINT i = 1; i < numBones; ++i)
	{
		// 루트노드인 경우
		XMMATRIX toParent = XMLoadFloat4x4( &toParentTransforms[i] );

		// 하이라키가 제대로 완성되면 수정 필요, 부모로 가는 인덱스가 필요
		int parentIndex = mBoneHierarchy[i].parentIdx;
		XMMATRIX parentToRoot = XMLoadFloat4x4( &toRootTransforms[parentIndex] );

		XMMATRIX toRoot = XMMatrixMultiply( toParent, parentToRoot );

		XMStoreFloat4x4( &toRootTransforms[i], toRoot );
	}

	// 뼈대 오프셋 변환을 곱해 최종변환을 구함
	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4( &mBoneOffsets[i] );
		XMMATRIX toRoot = XMLoadFloat4x4( &toRootTransforms[i] );
		XMStoreFloat4x4( &finalTransforms[i], XMMatrixMultiply( offset, toRoot ) );

		// 변환행렬이 오른손 좌표계이므로 이를 왼손좌표계로 변경
		finalTransforms[i] = MathHelper::GetInstance( )->Float4x4MulFloat4x4( finalTransforms[i], XMFLOAT4X4( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 ) );
	}
}

CMesh::CMesh( )
{
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
	m_nBaseVertex = 0;
}

CMesh::CMesh( ID3D11Device *pd3dDevice )
{
	//	m_vPositions = NULL;
	//	m_vnIndices = NULL;

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
	m_nBaseVertex = 0;
}


CMesh::~CMesh( )
{
	if (!( m_vPositions.empty( ) ))	m_vPositions.clear( );
	if (!( m_vnIndices.empty( ) ))	m_vnIndices.clear( );

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
		ppd3dNewVertexBuffers[m_nBuffers + i] = ppd3dBuffers[i];
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
void CMesh::SetBoundingCube( XMFLOAT3 max, XMFLOAT3 min )
{
	m_bcBoundingCube.m_vMax = max;
	m_bcBoundingCube.m_vMin = min;
}
void CMesh::SetBoundingCube( AABB boundingBox )
{
	m_bcBoundingCube = boundingBox;
}

void CMesh::FindMinMax( )
{
	float *arrX = new float[m_nVertices];
	float *arrY = new float[m_nVertices];
	float *arrZ = new float[m_nVertices];

	XMFLOAT3 min;
	XMFLOAT3 max;

	for (int i = 0; i < m_nVertices; i++)
	{
		arrX[i] = m_vPositions[i].x;
		arrY[i] = m_vPositions[i].y;
		arrZ[i] = m_vPositions[i].z;
	}

	QuickSort( arrX, 0, m_nVertices - 1 );
	QuickSort( arrY, 0, m_nVertices - 1 );
	QuickSort( arrZ, 0, m_nVertices - 1 );

	min.x = arrX[0];
	min.y = arrY[0];
	min.z = arrZ[0];

	max.x = arrX[m_nVertices - 1];
	max.y = arrY[m_nVertices - 1];
	max.z = arrZ[m_nVertices - 1];

	delete[ ] arrX;
	delete[ ] arrY;
	delete[ ] arrZ;

	m_bcBoundingCube.m_vMax = max;
	m_bcBoundingCube.m_vMin = min;
}

void CMesh::GetMinMax( XMFLOAT3* min, XMFLOAT3* max )
{
	min->x = m_bcBoundingCube.m_vMin.x;
	min->y = m_bcBoundingCube.m_vMin.y;
	min->z = m_bcBoundingCube.m_vMin.z;

	max->x = m_bcBoundingCube.m_vMax.x;
	max->y = m_bcBoundingCube.m_vMax.y;
	max->z = m_bcBoundingCube.m_vMax.z;
}

bool CMesh::CheckRayIntersection( XMVECTOR *pvRayPosition, XMVECTOR *pvRatDirection, float *pHitDist, XMFLOAT3 *pIntersectionPos )
{
	int iPickedTriangle = -1;
	float fHitDist = FLT_MAX, fNearDist = FLT_MAX;
	float fu, fv, fNearU, fNearV;
	bool bIntersection = false;

	for (int i = 0; i < m_nIndices / 3; i++)
	{
		UINT i0 = m_vnIndices[i * 3 + 0];
		UINT i1 = m_vnIndices[i * 3 + 1];
		UINT i2 = m_vnIndices[i * 3 + 2];

		D3DXVECTOR3 v0( m_vPositions[i0].x, m_vPositions[i0].y, m_vPositions[i0].z );
		D3DXVECTOR3 v1( m_vPositions[i1].x, m_vPositions[i1].y, m_vPositions[i1].z );
		D3DXVECTOR3 v2( m_vPositions[i2].x, m_vPositions[i2].y, m_vPositions[i2].z );

		XMFLOAT3 temp = MathHelper::GetInstance( )->VectorToFloat3( *pvRayPosition );
		D3DXVECTOR3 rayOrigin( temp.x, temp.y, temp.z);
		temp = MathHelper::GetInstance( )->VectorToFloat3( *pvRatDirection );
		D3DXVECTOR3 rayDir( temp.x, temp.y, temp.z );

		// 뻗어나온 반직선이 삼각형(v0,v1,v2)의 안에 있는지 검사하고 안에 있다면 정확한 위치를 계산한 뒤 거리를 rayOrigin과의 거리를 구함
		if (D3DXIntersectTri( &v0, &v1, &v2, &rayOrigin, &rayDir, &fu, &fv, &fHitDist ))
		{
			bIntersection = true;
			if (fHitDist < fNearDist)
			{
				fNearDist = fHitDist;
				fNearU = fu;
				fNearV = fv;
				iPickedTriangle = i;
			}
		}
	}

	// 삼각형의 찍힌 좌표 구하기
	if (iPickedTriangle != -1)
	{
		UINT i0 = m_vnIndices[iPickedTriangle * 3 + 0];
		UINT i1 = m_vnIndices[iPickedTriangle * 3 + 1];
		UINT i2 = m_vnIndices[iPickedTriangle * 3 + 2];

		XMVECTOR v0 = XMLoadFloat3( &m_vPositions[i0] );
		XMVECTOR v1 = XMLoadFloat3( &m_vPositions[i1] );
		XMVECTOR v2 = XMLoadFloat3( &m_vPositions[i2] );

		XMVECTOR pos = -v0 + -fNearU*( v1 - v0 ) + -fNearV * ( v2 - v0 );
		XMStoreFloat3( pIntersectionPos, pos );
	}
	*pHitDist = fNearDist;
	return bIntersection;
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

CMeshTextured::CMeshTextured( ID3D11Device *pd3dDevice ) : CMesh( pd3dDevice )
{
	m_pd3dTexCoordBuffer = NULL;
}

CMeshTextured::~CMeshTextured( )
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release( );
}

void CMeshTextured::CreateRasterizerState( ID3D11Device *pd3dDevice )
{
	D3D11_RASTERIZER_DESC d3dRastersizerDesc;
	::ZeroMemory( &d3dRastersizerDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	d3dRastersizerDesc.CullMode = D3D11_CULL_FRONT;
	// 솔리드와 와이어 설정할 수 있음
	d3dRastersizerDesc.FrontCounterClockwise = FALSE;
	d3dRastersizerDesc.FillMode = D3D11_FILL_SOLID;
	pd3dDevice->CreateRasterizerState( &d3dRastersizerDesc, &m_pd3dRasterizerState );
}

void CMeshTextured::ChangeRasterizerState( ID3D11Device* pd3dDevice, bool ClockWise, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode )
{
	if (m_pd3dRasterizerState)
		m_pd3dRasterizerState->Release( );

	D3D11_RASTERIZER_DESC d3dRastersizerDesc;
	::ZeroMemory( &d3dRastersizerDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	d3dRastersizerDesc.CullMode = CullMode;
	d3dRastersizerDesc.FrontCounterClockwise = ClockWise;
	d3dRastersizerDesc.FillMode = FillMode;
	pd3dDevice->CreateRasterizerState( &d3dRastersizerDesc, &m_pd3dRasterizerState );
}

void CMeshTextured::OnChangeTexture( ID3D11Device *pd3dDevice, _TCHAR *texturePath, int index )
{
	_TCHAR pstrTextureName[80];
	ID3D11ShaderResourceView *pd3dsrvTexture = NULL;

	// 그라운드 텍스처 지정
	_stprintf_s( pstrTextureName, texturePath, 0, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( index, pd3dsrvTexture );
	pd3dsrvTexture->Release( );
}

CSkyBoxMesh::CSkyBoxMesh( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth ) : CMeshTextured( pd3dDevice )
{
	//스카이 박스는 6개의 면(사각형), 사각형은 정점 4개, 그러므로 24개의 정점이 필요하다.
	m_nVertices = 24;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_vPositions.resize( m_nVertices );
	XMFLOAT2 *pvTexCoords = new XMFLOAT2[m_nVertices];

	int i = 0;
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;
	// Front Quad 
	m_vPositions[i] = XMFLOAT3( -fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( -fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Back Quad
	m_vPositions[i] = XMFLOAT3( +fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( -fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( -fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( +fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Left Quad
	m_vPositions[i] = XMFLOAT3( -fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( -fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( -fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( -fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Right Quad
	m_vPositions[i] = XMFLOAT3( +fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( +fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Top Quad
	m_vPositions[i] = XMFLOAT3( -fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, +fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( -fx, +fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 1.0f );
	// Bottom Quad
	m_vPositions[i] = XMFLOAT3( -fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 0.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, -fx, +fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 0.0f );
	m_vPositions[i] = XMFLOAT3( +fx, -fx, -fx );
	pvTexCoords[i++] = XMFLOAT2( 1.0f, 1.0f );
	m_vPositions[i] = XMFLOAT3( -fx, -fx, -fx );
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

	m_pMeshTexture = new CTexture( 6, 1, 0, 0 );
	m_pMeshTexture->SetSampler( 0, pd3dSamplerState );
	pd3dSamplerState->Release( );
	m_pMeshTexture->AddRef( );

	OnChangeSkyBoxTextures( pd3dDevice, 1 );
}

CSkyBoxMesh::~CSkyBoxMesh( )
{
	if (m_pd3dDepthStencilState)
		m_pd3dDepthStencilState->Release( );
	if (m_pMeshTexture)
		m_pMeshTexture->Release( );
}

void CSkyBoxMesh::OnChangeSkyBoxTextures( ID3D11Device *pd3dDevice, int nIndex )
{
	// 6개의 스카이 박스 텍스처를 생성하여 CTexture 객체에 연결
	_TCHAR pstrTextureName[80];
	ID3D11ShaderResourceView *pd3dsrvTexture = NULL;

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Front_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 0, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Back_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 1, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Left_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 2, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Right_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 3, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Top_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 4, pd3dsrvTexture );
	pd3dsrvTexture->Release( );

	_stprintf_s( pstrTextureName, _T( "./SkyBox/SkyBox_Bottom_%d.jpg" ), nIndex, 80 );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pstrTextureName, NULL, NULL, &pd3dsrvTexture, NULL );
	m_pMeshTexture->SetTexture( 5, pd3dsrvTexture );
	pd3dsrvTexture->Release( );
}

void CSkyBoxMesh::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets );
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );

	// 스카이 박스를 그리기 위한 샘플러 상태 객체와 깊이 스텐실 상태 객체를 설정
	m_pMeshTexture->UpdateSamplerShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( m_pd3dDepthStencilState, 1 );

	// 스카이 박스의 6개의 면을 순서대로 그린다.
	for (int i = 0; i < 6; i++)
	{
		// 스카이 박스의 각 면을 그릴 때 사용할 텍스처를 설정
		m_pMeshTexture->UpdateTextureShaderVariable( pd3dDeviceContext, i, 0 );
		pd3dDeviceContext->DrawIndexed( 4, 0, i * 4 );
	}
	pd3dDeviceContext->OMSetDepthStencilState( NULL, 1 );
}

void CSkyBoxMesh::ChangeRasterizerState( ID3D11Device* pd3dDevice, bool ClockWise, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode )
{
	if (m_pd3dRasterizerState)
		m_pd3dRasterizerState->Release( );

	D3D11_RASTERIZER_DESC d3dRastersizerDesc;
	::ZeroMemory( &d3dRastersizerDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	d3dRastersizerDesc.CullMode = CullMode;
	d3dRastersizerDesc.FrontCounterClockwise = ClockWise;
	d3dRastersizerDesc.FillMode = FillMode;
	pd3dDevice->CreateRasterizerState( &d3dRastersizerDesc, &m_pd3dRasterizerState );
}

CObjectMesh::CObjectMesh( ID3D11Device *pd3dDevice, CFbxMesh vertex, int TextureCount ) : CMeshTextured( pd3dDevice )
{
	m_nVertices = vertex.m_nVertexCount;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_vPositions.resize( m_nVertices );
	vector<XMFLOAT2> pvTexCoords( m_nVertices );
	vector<XMFLOAT3> pvNormals( m_nVertices );
	vector<XMFLOAT3> pvTangents( m_nVertices );

	for (int i = 0; i < vertex.m_pVertices.size( ); i++)
	{
		m_vPositions[i] = vertex.m_pVertices[i].m_position;
		pvTexCoords[i] = vertex.m_pVertices[i].m_textureUV;
		pvNormals[i] = XMFLOAT3( 0.0f, 0.0f, 1.0f );
		pvTangents[i] = XMFLOAT3( 1.0f, 0.0f, 0.0f );
	}

	// 정점 버퍼 생성
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

	// UV 좌표
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT2 ) * pvTexCoords.size( );
	d3dBufferData.pSysMem = &pvTexCoords[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer );

	// normal 좌표
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * pvNormals.size( );
	d3dBufferData.pSysMem = &pvNormals[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer );

	// tangent
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * pvTangents.size( );
	d3dBufferData.pSysMem = &pvTangents[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTangentBuffer );

	ID3D11Buffer *pd3dBuffers[4] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dTangentBuffer };
	UINT pnBufferStrides[4] = { sizeof( XMFLOAT3 ), sizeof( XMFLOAT3 ), sizeof( XMFLOAT2 ), sizeof(XMFLOAT3) };
	UINT pnBufferOffsets[4] = { 0, 0, 0, 0 };
	AssembleToVertexBuffer( 4, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	// 인덱스 버퍼 생성
	// n개의 정점으로 만들 수 있는 삼각형의 개수는 n-2 개
	// 삼각형 1개당 인덱스가 3개 필요하기 때문에 인덱스의 개수는 3(n-2)개가 필요
	m_nIndices = vertex.m_nIndexCount;

	m_vnIndices = vertex.m_pvIndices;

	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = &m_vnIndices[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer );

	CreateRasterizerState( pd3dDevice );

	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory( &d3dDepthStencilDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
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

	m_pMeshTexture = new CTexture( TextureCount, TextureCount, 0, 0 );

	for (int i = 0; i < TextureCount; i++)
		m_pMeshTexture->SetSampler( i, pd3dSamplerState );

	pd3dSamplerState->Release( );
	m_pMeshTexture->AddRef( );
}

CObjectMesh::~CObjectMesh( )
{
	if (m_pd3dDepthStencilState)
		m_pd3dDepthStencilState->Release( );
	if (m_pMeshTexture)
		m_pMeshTexture->Release( );
}


void CObjectMesh::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets );
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );
	pd3dDeviceContext->RSSetState( m_pd3dRasterizerState );

	m_pMeshTexture->UpdateSamplerShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( m_pd3dDepthStencilState, 1 );

	m_pMeshTexture->UpdateTextureShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->DrawIndexed( m_nIndices, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( NULL, 1 );
}

CAnimatedMesh::CAnimatedMesh( ID3D11Device *pd3dDevice, CFbxMesh vertex, int TextureCount ) : CMeshTextured( pd3dDevice )
{
	m_nVertices = vertex.m_nVertexCount;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_vPositions.resize( m_nVertices );
	vector<XMFLOAT2> pvTexCoords( m_nVertices );
	vector<XMFLOAT3> pvNormals( m_nVertices );
	vector<XMFLOAT3> pvTangents( m_nVertices );
	vector<XMFLOAT3> pvWeights( m_nVertices );
	vector<XMFLOAT4> pvBoneIndices( m_nVertices );
	m_skindata = vertex.m_skinnedData;

	for (int i = 0; i < m_nVertices; i++)
	{
		m_vPositions[i] = vertex.m_pVertices[i].m_position;
		pvTexCoords[i] = vertex.m_pVertices[i].m_textureUV;
		pvNormals[i] = XMFLOAT3( 0.0f, 0.3f, 1.f );
		pvTangents[i] = XMFLOAT3( 1.0f, 0.0f, 0.0f );
		pvWeights[i] = vertex.m_pVertices[i].m_weights;
		pvBoneIndices[i] = vertex.m_pVertices[i].m_boneIndices;
	}

	// 정점 버퍼 생성
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

	// UV 좌표
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT2 ) * pvTexCoords.size( );
	d3dBufferData.pSysMem = &pvTexCoords[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer );

	// normal 좌표
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * pvNormals.size( );
	d3dBufferData.pSysMem = &pvNormals[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer );

	// tangent 좌표
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * pvTangents.size( );
	d3dBufferData.pSysMem = &pvTangents[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dTangentBuffer );

	// weights
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT3 ) * pvWeights.size( );
	d3dBufferData.pSysMem = &pvWeights[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dWeightBuffer );

	// boneIndices
	d3dBufferDesc.ByteWidth = sizeof( XMFLOAT4 ) * pvBoneIndices.size( );
	d3dBufferData.pSysMem = &pvBoneIndices[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dBoneIdxBuffer );

	ID3D11Buffer *pd3dBuffers[6] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dTangentBuffer, m_pd3dWeightBuffer, m_pd3dBoneIdxBuffer };
	UINT pnBufferStrides[6] = { sizeof( XMFLOAT3 ), sizeof( XMFLOAT3 ), sizeof( XMFLOAT2 ), sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[6] = { 0, 0, 0, 0, 0, 0 };
	AssembleToVertexBuffer( 6, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	// 인덱스 버퍼 생성
	// n개의 정점으로 만들 수 있는 삼각형의 개수는 n-2 개
	// 삼각형 1개당 인덱스가 3개 필요하기 때문에 인덱스의 개수는 3(n-2)개가 필요
	m_nIndices = vertex.m_nIndexCount;

	m_vnIndices = vertex.m_pvIndices;

	::ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = &m_vnIndices[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer );

	CreateRasterizerState( pd3dDevice );

	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory( &d3dDepthStencilDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
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

	m_pMeshTexture = new CTexture( TextureCount, TextureCount, 0, 0 );

	for (int i = 0; i < TextureCount; i++)
		m_pMeshTexture->SetSampler( i, pd3dSamplerState );

	pd3dSamplerState->Release( );
	m_pMeshTexture->AddRef( );
}

CAnimatedMesh::~CAnimatedMesh( )
{

}

void CAnimatedMesh::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->IASetVertexBuffers( m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets );
	pd3dDeviceContext->IASetIndexBuffer( m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset );
	pd3dDeviceContext->IASetPrimitiveTopology( m_d3dPrimitiveTopology );
	pd3dDeviceContext->RSSetState( m_pd3dRasterizerState );

	m_pMeshTexture->UpdateSamplerShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( m_pd3dDepthStencilState, 1 );

	m_pMeshTexture->UpdateTextureShaderVariable( pd3dDeviceContext, 0, 0 );
	pd3dDeviceContext->DrawIndexed( m_nIndices, 0, 0 );
	pd3dDeviceContext->OMSetDepthStencilState( NULL, 1 );
}


//------------------------------------------------------------------------------------
CMeshDiffused::CMeshDiffused( ID3D11Device *pd3dDevice ) : CMesh( pd3dDevice )
{
	m_pd3dColorBuffer = NULL;
	m_nBaseVertex = 0;
}

CMeshDiffused::~CMeshDiffused( )
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release( );
}


CCubeMeshDiffused::CCubeMeshDiffused( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, D3DXCOLOR d3dxColor ) : CMeshDiffused( pd3dDevice )
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	//직육면체 메쉬는 2개의 정점 버퍼(위치 벡터 버퍼와 색상 버퍼)로 구성된다.
	//직육면체 메쉬의 정점 버퍼(위치 벡터 버퍼)를 생성한다.
	m_vPositions.resize( 8 );
	m_vPositions[0] = XMFLOAT3( -fx, +fy, -fz );
	m_vPositions[1] = XMFLOAT3( +fx, +fy, -fz );
	m_vPositions[2] = XMFLOAT3( +fx, +fy, +fz );
	m_vPositions[3] = XMFLOAT3( -fx, +fy, +fz );
	m_vPositions[4] = XMFLOAT3( -fx, -fy, -fz );
	m_vPositions[5] = XMFLOAT3( +fx, -fy, -fz );
	m_vPositions[6] = XMFLOAT3( +fx, -fy, +fz );
	m_vPositions[7] = XMFLOAT3( -fx, -fy, +fz );

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( D3DXVECTOR3 ) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = &m_vPositions[0];
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer );

	//직육면체 메쉬의 정점 버퍼(색상 버퍼)를 생성한다.
	D3DXCOLOR pd3dxColors[8];
	for (int i = 0; i < 8; i++) pd3dxColors[i] = d3dxColor + RANDOM_COLOR;

	d3dBufferDesc.ByteWidth = sizeof( D3DXCOLOR ) * m_nVertices;
	d3dBufferData.pSysMem = pd3dxColors;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer );

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof( D3DXVECTOR3 ), sizeof( D3DXCOLOR ) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer( 2, pd3dBuffers, pnBufferStrides, pnBufferOffsets );

	m_nIndices = 18;
	m_vnIndices.resize( m_nIndices );
	m_vnIndices[0] = 5; //5,6,4 - cw
	m_vnIndices[1] = 6; //6,4,7 - ccw
	m_vnIndices[2] = 4; //4,7,0 - cw
	m_vnIndices[3] = 7; //7,0,3 - ccw
	m_vnIndices[4] = 0; //0,3,1 - cw
	m_vnIndices[5] = 3; //3,1,2 - ccw
	m_vnIndices[6] = 1; //1,2,2 - cw 
	m_vnIndices[7] = 2; //2,2,3 - ccw
	m_vnIndices[8] = 2; //2,3,3 - cw  - Degenerated Index(2)
	m_vnIndices[9] = 3; //3,3,7 - ccw - Degenerated Index(3)
	m_vnIndices[10] = 3;//3,7,2 - cw  - Degenerated Index(3)
	m_vnIndices[11] = 7;//7,2,6 - ccw
	m_vnIndices[12] = 2;//2,6,1 - cw
	m_vnIndices[13] = 6;//6,1,5 - ccw
	m_vnIndices[14] = 1;//1,5,0 - cw
	m_vnIndices[15] = 5;//5,0,4 - ccw
	m_vnIndices[16] = 0;
	m_vnIndices[17] = 4;

	ZeroMemory( &d3dBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof( UINT ) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = &m_vnIndices[0];
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
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory( &d3dRasterizerDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;;
	pd3dDevice->CreateRasterizerState( &d3dRasterizerDesc, &m_pd3dRasterizerState );
}

void CCubeMeshDiffused::Render( ID3D11DeviceContext *pd3dDeviceContext )
{
	CMesh::Render( pd3dDeviceContext );
}