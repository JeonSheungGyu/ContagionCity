#pragma once
#include <vector>
#include <map>

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
	// ������ ��ġ ������ �����ϱ� ���� ��� ������ ����
	XMFLOAT3 m_vPosition;
public:
	CVertex( ) { m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f ); }
	CVertex( XMFLOAT3 vPosition ){ m_vPosition = vPosition; }
	XMFLOAT3 GetVertex( ) { return m_vPosition; }
	~CVertex( );
};

// �ִϸ��̼� ������ ������ ������, Ư�� �ð������� ��ȯ ������ ����
// �ð��� ���� ������ ������
struct Keyframe
{
	Keyframe( );
	~Keyframe( );

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

// �ϳ��� ����, �ִϸ��̼� �������� ������ ����
struct BoneAnimation
{
	// �ð��� ���� ��ȯ ������ ����, ���밡 �����̴� ���
	std::vector<Keyframe> Keyframes;

	float GetStartTime( ) const;
	float GetEndTime( ) const;

	void Interpolate( float t, XMFLOAT4X4& M );
};

// �ִϸ��̼� Ŭ�� �����, �ȱ�, �ٱ�, ���� ���� �������� �ִϸ��̼��� �ǹ���
struct AnimationClip
{
	// ���� Ŭ���� ��� ������ ���� �ð� �� ���� ���� ���� ��ȯ
	float GetClipStartTime( ) const;
	// ���� Ŭ���� ��� ������ ���� �ð� �� ���� ���� ���� ��ȯ
	float GetClipEndTime( ) const;

	// �� Ŭ���� �� BoneAnimation�� �����鼭 �ִϸ��̼ǵ��� ����
	void Interpolate( float t, std::vector<XMFLOAT4X4>& boneTransforms );

	//  ���뺰 �ִϸ��̼ǵ�
	std::vector<BoneAnimation> BoneAnimations;
};

// ��Ű�� �ִϸ��̼� �ڷḦ ���� Ŭ����
class SkinnedData
{
	UINT BoneCount( ) const;
	float GetClipStartTime( const std::string& clipName ) const;
	float GetClipEndTime( const std::string& clipName ) const;

	void Set(
		std::vector<int>& boneHierarchy,
		std::vector<XMFLOAT4X4>& boneOffsets,
		std::map<std::string, AnimationClip>& animations );

	// ������ȯ�� ���ϴ� �Լ�, ������ ��ȯ���� ��ģ ��ȯ ��Ʈ�����̴�.
	void GetFinalTransforms( const std::string& clipName, float timePos,
		std::vector<XMFLOAT4X4>& finalTransforms );

private:
	// i�� ������ �θ��� ������ ��´�, i�� ����� �ִϸ��̼� Ŭ���� i��° BoneAnimaion�ν��Ͻ��� �����ȴ�.
	std::vector<int> mBoneHierarchy;

	// i�� ������ ������ ��ȯ
	std::vector<XMFLOAT4X4> mBoneOffsets;

	// �ִϸ��̼� ������ Ŭ������ ����
	std::map<std::string, AnimationClip> mAnimations;
};

struct CFbxVertex
{
	XMFLOAT3 m_position;
	XMFLOAT2 m_textureUV;
	XMFLOAT4 m_tangentU;
	XMFLOAT3 m_weights;		// ����ġ
	BYTE m_boneIndices[4];	// �� ������ ������ �ִ� ����
};

struct CFbxMesh
{
public:
	std::vector<CFbxVertex> m_pVertexes;
	std::vector<UINT> m_pvIndices;

	int m_nIndexCount;
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
	// �ٿ�� �ڽ��� �ּ����� �ִ���
	XMFLOAT3 m_vMin;
	XMFLOAT3 m_vMax;

public:
	AABB( ){ m_vMin = XMFLOAT3( +FLT_MAX, +FLT_MAX, +FLT_MAX ); m_vMax = XMFLOAT3( -FLT_MAX, -FLT_MAX, -FLT_MAX ); }
	AABB( XMFLOAT3 vMin, XMFLOAT3 vMax ) { m_vMin = vMin; m_vMax = vMax; }

	// �� ���� �ٿ�� �ڽ��� ��ħ
	void Union( XMFLOAT3& vMin, XMFLOAT3& vMax );
	void Union( AABB *pAABB );
	// �ٿ�� �ڽ��� 8���� �������� ��ķ� ��ȯ�ϰ� �ּ����� �ִ����� �ٽ� ���
	void Update( XMFLOAT4X4 *pmtxTransform );
};

class CMesh
{
public:
	// �� ������ ��ġ ���͸� ��ŷ�� ���Ͽ� ����
	std::vector<XMFLOAT3> m_vPositions;
	// �޽��� �ε����� ����
	std::vector<UINT> m_vnIndices;

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

	// ������ ��ġ ���Ϳ� ������ �����ϱ� ���� ������ ���� �������̽� ������
	ID3D11Buffer *m_pd3dPositionBuffer;
	
	// ���۵��� �Է������⿡ �����ϱ� ���� ���� ���� ��ȣ
	UINT m_nSlot;
	// �ν��Ͻ��� ���� ���� ���۴� �޽��� ���� �����Ϳ� �ν��Ͻ� �����͸� ���´�.
	// �׷��Ƿ� �ν��Ͻ��� ���� ���� ���۴� �ϳ��� �ƴ϶� ���۵��� �迭�̴�.
	// ������ ��ҵ��� ��Ÿ���� ���۵��� �Է������⿡ �����ϱ� ���� ����
	ID3D11Buffer **m_ppd3dVertexBuffers;
	// ������ �����ϱ� ���� �ʿ��� ������ ����
	int m_nBuffers;
	
	// ������ ����
	int m_nVertices;
	UINT m_nStartVertex;
	// ������ ��ҵ��� ��Ÿ���� ���۵��� ������ ����Ʈ ���� ��Ÿ���� �迭
	UINT *m_pnVertexStrides;
	// ������ ��ҵ��� ��Ÿ���� ���۵��� ���� ��ġ�� ��Ÿ���� �迭
	UINT *m_pnVertexOffsets;

	// �ε��� ���ۿ� ���� �������̽� ������
	ID3D11Buffer *m_pd3dIndexBuffer;
	// �ε��� ���۰� �����ϴ� �ε����� ����
	UINT m_nIndices;
	// �ε��� ���ۿ��� �޽ø� ǥ���ϱ� ���� ���Ǵ� ���� �ε���
	UINT m_nStartIndex;
	// �� �ε����� ������ �ε���
	int m_nBaseVertex;
	UINT m_nIndexOffset;
	// �� �ε����� ����
	DXGI_FORMAT m_dxgiIndexFormat;

	ID3D11RasterizerState *m_pd3dRasterizerState;

public:
	AABB GetBoundingCube( ){ return m_bcBoundingCube; }

	// �޽��� ���� ���۵��� �迭�� ����
	void AssembleToVertexBuffer( int nBuffers = 0, ID3D11Buffer **m_pd3dBuffers = NULL, UINT *pnBufferStrides = NULL, UINT *pnBufferOffset = NULL );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice ){}
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
	// �ν��Ͻ��� �̿��Ͽ� ������
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
	// ������ü�� ����, ����, ������ ũ�⸦ �����Ͽ� ������ü �޽ø� ����
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
	// ������ ������ ����ϱ� ���� ��������
	ID3D11Buffer *m_pd3dNormalBuffer;

public:
	// ������ ���Ե� �ﰢ���� �������͸� ����ϴ� �Լ�
	XMFLOAT3 CalculateTriangleNormal( UINT nIndex0, UINT nIndex1, UINT nIndex2 );
	void SetTriangleListVertexNormal( XMFLOAT3 *pvNormals );
	// ������ ���������� ����� ����ϴ� �Լ�
	void SetAverageVertexNormal( XMFLOAT3 *pvNormals, int nPrimitives, int nOffset, bool bStrip );
	void CalculateVertexNormal( XMFLOAT3 *pvNormals );
};

// ������ ����Ͽ� ������ ������ �����ϱ� ���Ͽ� ������ �������͸� ���� ������ü �޽� Ŭ����
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
	// �ؽ�ó ������ �ϱ� ���Ͽ� �ؽ�ó ��ǥ�� �ʿ�
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
	void ChangeRasterizerState( ID3D11Device* pd3dDevice, bool ClockWise, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode );
};

class CObjectMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;
	CTexture *m_pMeshTexture;

	// �������� �ִ� �ּ�
	XMFLOAT3 min;
	XMFLOAT3 max;

public:
	CObjectMesh( ID3D11Device *pd3dDevice, CFbxMesh vertex, int TextureCount );
	virtual ~CObjectMesh( );

	void OnChangeTexture( ID3D11Device *pd3dDevice, _TCHAR *texturePath, int index );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice );
	// Cull Mode�� �ո��� �׸� ������ �޸��� �׸�������
	// ClockWise�� ���ε����� ����, TRUE �̸� �ݽð�, FALSE�̸� �ð�
	// Fill Mode�� �ָ���� �Ұ����� ���̾����������� �Ұ�����
	void ChangeRasterizerState( ID3D11Device* pd3dDevice, bool ClockWise, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode );
	void FindMinMax( );
};