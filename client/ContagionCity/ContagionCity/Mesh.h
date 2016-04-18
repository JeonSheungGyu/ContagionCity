#pragma once
#include <vector>
#include <map>

#define RANDOM_COLOR D3DXCOLOR((rand() * 0xFFFFFF) / RAND_MAX)

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

// ��������
struct Material
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

// m3d ���� �ε��� ����� ���� ����
struct M3dMaterial
{
	Material Mat;

	bool AlphaClip;
	std::string EffectTypeName;
	std::wstring DiffuseMapName;
	std::wstring NormalMapName;
};

// m3d ���� �ε����� �κ����� �κ�, �� ������� ���� ����
struct Subset
{
	Subset() : Id( -1 ),	VertexStart( 0 ), VertexCount( 0 ), FaceStart( 0 ), FaceCount( 0 ){}

	UINT Id;
	UINT VertexStart;
	UINT VertexCount;
	UINT FaceStart;
	UINT FaceCount;
};

// m3d ���� �ε��� ����� ����
struct M3dVertex
{
	XMFLOAT3 pos;
	XMFLOAT4 TangentU;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;

	XMFLOAT3 weights;		// ����ġ
	BYTE boneIndices[4];		// �� ������ ������ �ִ� ����
};

struct CM3dMesh
{
public:
	std::vector<M3dMaterial> m_mats;
	std::vector<Subset> m_subsets;
	std::vector<M3dVertex> m_pVertexes;
	std::vector<USHORT> m_pvIndices;
//	SkinnedData m_skinnedData;

	int m_nIndexCount;
	int m_nVertexCount;
	int m_iLayer;
	int m_iType;
};

// �ִϸ��̼� ������ ������ ������, Ư�� �ð������� ��ȯ ������ ����
// �ð��� ���� ������ ������
struct Keyframe			// CKeyframe
{
	Keyframe( );
	~Keyframe( );

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

// �ϳ��� ����, �ִϸ��̼� �������� ������ ����
struct BoneAnimation		// CAnimation
{
	// �ð��� ���� ��ȯ ������ ����, ���밡 �����̴� ���
	std::vector<Keyframe> Keyframes;
	// �� �̸�
	std::string BoneName;
	// �θ� �̸�
	std::string PerentBoneName;
	// �� �ε���
	int BoneIndex;

	float GetStartTime( ) const;
	float GetEndTime( ) const;

	void Interpolate( float t, XMFLOAT4X4& M );
};

// �ִϸ��̼� Ŭ�� �����, �ȱ�, �ٱ�, ���� ���� �������� �ִϸ��̼��� �ǹ���
struct AnimationClip		// CAnimationSet
{
	// ���� Ŭ���� ��� ������ ���� �ð� �� ���� ���� ���� ��ȯ
	float GetClipStartTime( ) const;
	// ���� Ŭ���� ��� ������ ���� �ð� �� ���� ���� ���� ��ȯ
	float GetClipEndTime( ) const;

	// �� Ŭ���� �� BoneAnimation�� �����鼭 �ִϸ��̼ǵ��� ����
	void Interpolate( float t, std::vector<XMFLOAT4X4>& boneTransforms );

	// ���뺰 �ִϸ��̼ǵ�
	std::vector<BoneAnimation> BoneAnimations;
	
	// ���� ã��
	BoneAnimation* FindBone( int index );
	BoneAnimation* FindBone( std::string name );
};

// ��Ű�� �ִϸ��̼� �ڷḦ ���� Ŭ����
class SkinnedData
{
public:
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

	// i�� ������ ������ ��ȯ, �θ�� ���� ������ ��ȯ
	std::vector<XMFLOAT4X4> mBoneOffsets;

	// �ִϸ��̼� ������ Ŭ������ ����
	std::map<std::string, AnimationClip> mAnimations;
};

struct CFbxVertex
{
	XMFLOAT3 m_position;
	XMFLOAT2 m_textureUV;
	XMFLOAT3 m_normal;
	XMFLOAT4 m_tangent;
	XMFLOAT3 m_binormal;

	XMFLOAT3 m_weights = XMFLOAT3(0.0f , 0.0f, 0.0f);						// ����ġ
	XMFLOAT4 m_boneIndices = XMFLOAT4(-1.0f,-1.0f, -1.0f, -1.0f);		// �� ������ ������ �ִ� ����
};

struct CFbxMesh
{
public:
	std::vector<CFbxVertex> m_pVertices;
	std::vector<UINT> m_pvIndices;
	SkinnedData m_skinnedData;

	int m_nIndexCount;
	int m_nVertexCount;
	int m_iLayer;
	int m_iType;
};


class CDiffusedVertex
{
	XMFLOAT3 m_vPosition;
	D3DXCOLOR m_cDiffuse;

public:
	CDiffusedVertex( float x, float y, float z, D3DXCOLOR cDiffuse ) { m_vPosition = XMFLOAT3( x, y, z ); m_cDiffuse = cDiffuse; }
	CDiffusedVertex( XMFLOAT3 vPosition, D3DXCOLOR cDiffuse ) { m_vPosition = vPosition; m_cDiffuse = cDiffuse; }
	CDiffusedVertex( ){ m_vPosition = XMFLOAT3( 0.0f, 0.0f, 0.0f ); m_cDiffuse = D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.0f ); }
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
	void SetBoundingCube( XMFLOAT3 max, XMFLOAT3 min );
	void SetBoundingCube( AABB boundingBox );

	// �޽��� ���� ���۵��� �迭�� ����
	void AssembleToVertexBuffer( int nBuffers = 0, ID3D11Buffer **m_pd3dBuffers = NULL, UINT *pnBufferStrides = NULL, UINT *pnBufferOffset = NULL );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice ){}
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
	// �ν��Ͻ��� �̿��Ͽ� ������
	virtual void RenderInstanced( ID3D11DeviceContext *pd3dDeviceContext, int nInstances = 0, int nStartInstance = 0 );

	void FindMinMax( );
	void GetMinMax( XMFLOAT3* min, XMFLOAT3* max );
};

class CTexture;

class CMeshTextured : public CMesh
{
public:
	CMeshTextured( ID3D11Device *pd3dDevice );
	virtual ~CMeshTextured( );

	void OnChangeTexture( ID3D11Device *pd3dDevice, _TCHAR *texturePath, int index );
	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice );
	// Cull Mode�� �ո��� �׸� ������ �޸��� �׸�������
	// ClockWise�� ���ε����� ����, TRUE �̸� �ݽð�, FALSE�̸� �ð�
	// Fill Mode�� �ָ���� �Ұ����� ���̾����������� �Ұ�����
	virtual void ChangeRasterizerState( ID3D11Device* pd3dDevice, bool ClockWise, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode );

protected:
	// �ؽ�ó ������ �ϱ� ���Ͽ� �ؽ�ó ��ǥ�� �ʿ�
	ID3D11Buffer *m_pd3dTexCoordBuffer;
	CTexture *m_pMeshTexture;

	
};

class CSkyBoxMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;

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
	ID3D11Buffer *m_pd3dNormalBuffer;
	ID3D11Buffer *m_pd3dTangentBuffer;

public:
	CObjectMesh( ID3D11Device *pd3dDevice, CFbxMesh vertex, int TextureCount );
	virtual ~CObjectMesh( );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
};

class CAnimatedMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;
	ID3D11Buffer *m_pd3dNormalBuffer;
	ID3D11Buffer *m_pd3dTangentBuffer;
	ID3D11Buffer *m_pd3dWeightBuffer;
	ID3D11Buffer *m_pd3dBoneIdxBuffer;

	// ���� ��ǥ
	std::vector<CFbxVertex> m_pVertices;
	SkinnedData m_skindata;

public:
	CAnimatedMesh( ID3D11Device *pd3dDevice, CFbxMesh vertex, int TextureCount );
	~CAnimatedMesh( );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
};



//void BuildMeshSkinning( FbxMesh* mesh, vector<vector<pair<int, float>>>& boneIndices,
//	map<int, vector<int>> mVertexByIndex, VERTEX* _vertex, Model* _model ){
//
//	int numSkins = mesh->GetDeformerCount( FbxDeformer::eSkin );
//
//	if (numSkins > 0){      // 1
//		int numControlPoints = mesh->GetControlPointsCount( );      // 683
//	}
//
//	for (int iSkin = 0; iSkin < numSkins; ++iSkin){
//		FbxSkin* skin = (FbxSkin*)mesh->GetDeformer( iSkin, FbxDeformer::eSkin );
//		int clusterCount = skin->GetClusterCount( );
//
//		if (clusterCount == 0)
//			continue;
//		for (int iCluster = 0; iCluster < clusterCount; ++iCluster){      //21
//			FbxCluster* cluster = skin->GetCluster( iCluster );
//			FbxNode* bone = cluster->GetLink( );
//
//			string BoneName = string( bone->GetName( ) );
//
//			FbxAMatrix LinkBoneMatrix;
//			FbxAMatrix TransBoneMatrix;
//			FbxAMatrix ResultMatrix;
//
//			cluster->GetTransformLinkMatrix( LinkBoneMatrix );
//			cluster->GetTransformMatrix( TransBoneMatrix );
//			ResultMatrix = LinkBoneMatrix.Inverse( )*TransBoneMatrix;      // �������?
//
//			Bone* bones;
//
//			bones = _model->fineBone( BoneName );
//			//cout << BoneName << endl;
//			if (!bones){
//				cout << "not find bone name!!!!" << endl;
//			}
//			else{
//				//   cout << bones->index << ", " << bones->name << endl;
//				bones->boneMatrix._11 = ResultMatrix.mData[0].mData[0]; bones->boneMatrix._12 = ResultMatrix.mData[0].mData[1];
//				bones->boneMatrix._13 = ResultMatrix.mData[0].mData[2]; bones->boneMatrix._14 = ResultMatrix.mData[0].mData[3];
//				bones->boneMatrix._21 = ResultMatrix.mData[1].mData[0]; bones->boneMatrix._22 = ResultMatrix.mData[1].mData[1];
//				bones->boneMatrix._23 = ResultMatrix.mData[1].mData[2]; bones->boneMatrix._24 = ResultMatrix.mData[1].mData[3];
//				bones->boneMatrix._31 = ResultMatrix.mData[2].mData[0]; bones->boneMatrix._32 = ResultMatrix.mData[2].mData[1];
//				bones->boneMatrix._33 = ResultMatrix.mData[2].mData[2]; bones->boneMatrix._34 = ResultMatrix.mData[2].mData[3];
//				bones->boneMatrix._41 = ResultMatrix.mData[3].mData[0]; bones->boneMatrix._42 = ResultMatrix.mData[3].mData[1];
//				bones->boneMatrix._43 = ResultMatrix.mData[3].mData[2]; bones->boneMatrix._44 = ResultMatrix.mData[3].mData[3];
//				_model->SetBoneMatrix( bones->index, bones->boneMatrix );
//
//				if (!bone){
//					cout << "cluster " << iCluster << "has no link" << endl;
//					continue;
//				}
//
//				int numInfluencedVertices = cluster->GetControlPointIndicesCount( );
//				int* pIndexArray = cluster->GetControlPointIndices( );
//				double* pWeightArray = cluster->GetControlPointWeights( );
//
//
//				//cout << "Cluster : " << iCluster << " : " << bone->GetName()<<", influences : "<<numInfluencedVertices<< endl;
//				//   getchar();
//				// bone ������ ���� ����ġ �ο�
//				for (int iControlPoint = 0; iControlPoint < numInfluencedVertices; ++iControlPoint){
//
//					float weight = (float)pWeightArray[iControlPoint];
//
//					//cout << "weight : " << weight << endl;
//					if (weight){
//
//						int iControlPointIndex = pIndexArray[iControlPoint];      //BoneIndex
//
//						boneIndices[iControlPointIndex].push_back( pair<int, float>( bones->index, weight ) );
//					}
//				}
//			}
//		}
//	}
//	for (int j = 0; j < boneIndices.size( ); j++){
//		sort( boneIndices[j].begin( ), boneIndices[j].end( ) );
//	}
//	return;
//}