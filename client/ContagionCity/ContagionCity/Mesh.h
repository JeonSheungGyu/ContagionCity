#pragma once
#include <vector>

#define RANDOM_COLOR XMCOLOR((rand() * 0xFFFFFF) / RAND_MAX)

struct MESHINTERSECTINFO
{
	DWORD m_dwFaceIndex;
	float m_fU;
	float m_fV;
	float m_fDinstance;
};

class CVertex
{
	// 정점의 위치 정보를 저장하기 위한 멤버 변수를 선언
	XMFLOAT3 m_vPosition;
public:
	CVertex( ) { m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f ); }
	CVertex( XMFLOAT3 vPosition ){ m_vPosition = vPosition; }
	XMFLOAT3 GetVertex( ) { return m_vPosition; }
	~CVertex( );
};

struct CFbxVertex
{
public:
	std::vector<XMFLOAT3> m_pvPositions;
	int m_nVertexCount;
	int m_iLayer;
	int m_iType;
};

class CDiffusedVertex
{
	XMFLOAT3 m_vPosition;
	XMCOLOR m_cDiffuse;

public:
	CDiffusedVertex( float x, float y, float z, XMCOLOR cDiffuse ) { m_vPosition = XMFLOAT3( x, y, z ); m_cDiffuse = cDiffuse; }
	CDiffusedVertex( XMFLOAT3 vPosition, XMCOLOR cDiffuse ) { m_vPosition = vPosition; m_cDiffuse = cDiffuse; }
	CDiffusedVertex( ){ m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f ); m_cDiffuse = XMCOLOR( 0.0f, 0.0f, 0.0f, 0.0f ); }
	~CDiffusedVertex( ) {}
};

class AABB
{
public:
	// 바운딩 박스의 최소점과 최대점
	XMFLOAT3 m_vMin;
	XMFLOAT3 m_vMax;

public:
	AABB( ){ m_vMin = XMFLOAT3( +FLT_MAX, +FLT_MAX, +FLT_MAX ); m_vMax = XMFLOAT3( -FLT_MAX, -FLT_MAX, -FLT_MAX ); }
	AABB( XMFLOAT3 vMin, XMFLOAT3 vMax ) { m_vMin = vMin; m_vMax = vMax; }

	// 두 개의 바운딩 박스를 합침
	void Union( XMFLOAT3& vMin, XMFLOAT3& vMax );
	void Union( AABB *pAABB );
	// 바운딩 박스의 8개의 꼭지점을 행렬로 변환하고 최소점과 최대점을 다시 계산
	void Update( XMFLOAT4X4 *pmtxTransform );
};

class CMesh
{
public:
	// 각 정점의 위치 벡터를 픽킹을 위하여 저장
	XMFLOAT3 *m_pvPositions;
	// 메시의 인덱스를 저장
	UINT *m_pnIndices;

	int CheckRayIntersection( XMFLOAT3 *pvRayPosition, XMFLOAT3 *pvRatDirection, MESHINTERSECTINFO *pIntersectInfo );

	CMesh( );
	CMesh( ID3D11Device *pd3dDevice );
	virtual ~CMesh( );

private:
	int m_nReferences;

public:
	void AddRef( ) { m_nReferences++; }
	void Release( ) { if (--m_nReferences <= 0) delete this; }

protected:
	AABB m_bcBoundingCube;

protected:
	D3D11_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology;

	// 정점의 위치 벡터와 색상을 저장하기 위한 버퍼의 대한 인터페이스 포인터
	ID3D11Buffer *m_pd3dPositionBuffer;
	
	// 버퍼들을 입력조립기에 연결하기 위한 시작 슬롯 번호
	UINT m_nSlot;
	// 인스턴싱을 위한 정점 버퍼는 메시의 정점 데이터와 인스턴싱 데이터를 갖는다.
	// 그러므로 인스턴싱을 위한 정점 버퍼는 하나가 아니라 버퍼들의 배열이다.
	// 정점의 요소들을 나타내는 버퍼들을 입력조립기에 전달하기 위한 버퍼
	ID3D11Buffer **m_ppd3dVertexBuffers;
	// 정점을 조립하기 위해 필요한 버퍼의 개수
	int m_nBuffers;
	
	// 정점의 개수
	int m_nVertices;
	UINT m_nStartVertex;
	// 정점의 요소들을 나타내는 버퍼들의 원소의 바이트 수를 나타내는 배열
	UINT *m_pnVertexStrides;
	// 정점의 요소들을 나타내는 버퍼들의 시작 위치를 나타내는 배열
	UINT *m_pnVertexOffsets;

	// 인덱스 버퍼에 대한 인터페이스 포인터
	ID3D11Buffer *m_pd3dIndexBuffer;
	// 인덱스 버퍼가 포함하는 인덱스의 개수
	UINT m_nIndices;
	// 인덱스 버퍼에서 메시를 표현하기 위해 사용되는 시작 인덱스
	UINT m_nStartIndex;
	// 각 인덱스에 더해질 인덱스
	int m_nBaseVertex;
	UINT m_nIndexOffset;
	// 각 인덱스의 형식
	DXGI_FORMAT m_dxgiIndexFormat;

	ID3D11RasterizerState *m_pd3dRasterizerState;

public:
	AABB GetBoundingCube( ){ return m_bcBoundingCube; }

	// 메시의 정점 버퍼들을 배열로 조립
	void AssembleToVertexBuffer( int nBuffers = 0, ID3D11Buffer **m_pd3dBuffers = NULL, UINT *pnBufferStrides = NULL, UINT *pnBufferOffset = NULL );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice ){}
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
	// 인스턴싱을 이용하여 렌더링
	virtual void RenderInstanced( ID3D11DeviceContext *pd3dDeviceContext, int nInstances = 0, int nStartInstance = 0 );
};

class CMeshDiffused : public CMesh
{
public:
	CMeshDiffused( ID3D11Device *pd3dDevice );
	virtual ~CMeshDiffused( );

protected:
	ID3D11Buffer *m_pd3dColorBuffer;
};

class CCubeMeshDiffused : public CMeshDiffused
{
public:
	// 직육면체의 가로, 세로, 높이의 크기를 지정하여 직육면체 메시를 생성
	CCubeMeshDiffused( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, XMCOLOR Color );
	virtual ~CCubeMeshDiffused( );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
};

class CMeshIlluminated : public CMesh
{
public:
	CMeshIlluminated( ID3D11Device *pd3dDevice );
	virtual ~CMeshIlluminated( );

protected:
	// 조명의 영향을 계산하기 위한 법선벡터
	ID3D11Buffer *m_pd3dNormalBuffer;

public:
	// 정점이 포함된 삼각형의 법선벡터를 계산하는 함수
	XMFLOAT3 CalculateTriangleNormal( UINT nIndex0, UINT nIndex1, UINT nIndex2 );
	void SetTriangleListVertexNormal( XMFLOAT3 *pvNormals );
	// 정점의 법선벡터의 평균을 계산하는 함수
	void SetAverageVertexNormal( XMFLOAT3 *pvNormals, int nPrimitives, int nOffset, bool bStrip );
	void CalculateVertexNormal( XMFLOAT3 *pvNormals );
};

// 조명을 사용하여 정점의 색상을 결정하기 위하여 정점이 법선벡터를 갖는 직육면체 메시 클래스
class CCubeMeshIlluminated : public CMeshIlluminated
{
public:
	CCubeMeshIlluminated( ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f );
	virtual ~CCubeMeshIlluminated( );
};

class CMeshTextured : public CMesh
{
public:
	CMeshTextured( ID3D11Device *pd3dDevice );
	virtual ~CMeshTextured( );

protected:
	// 텍스처 매핑을 하기 위하여 텍스처 좌표가 필요
	ID3D11Buffer *m_pd3dTexCoordBuffer;
};

class CCubeMeshTextured : public CMeshTextured
{
public:
	CCubeMeshTextured( ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f );
	virtual ~CCubeMeshTextured( );
};


class CTexture;

class CSkyBoxMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;

	CTexture *m_pSkyboxTexture;

public:
	CSkyBoxMesh( ID3D11Device *pd3dDevice, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f );
	virtual ~CSkyBoxMesh( );

	void OnChangeSkyBoxTextures( ID3D11Device *pd3dDevice, int nIndex = 0 );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
};

class CGroundMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;
	CTexture *m_pGroundTexture;

public:
	CGroundMesh( ID3D11Device *pd3dDevice, CFbxVertex vertex );
	virtual ~CGroundMesh( );

	void OnChangeTexture( ID3D11Device *pd3dDevice );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );

};