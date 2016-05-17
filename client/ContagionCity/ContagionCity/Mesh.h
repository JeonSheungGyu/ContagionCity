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

class Bone
{
public:
	std::string boneName;
	int parentIdx;
	std::string parentBoneName;
};

// ��������
struct Material
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
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

	XMFLOAT4X4 mtxMatrix;
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
	float GetClipStartTime( const int& clipName ) const;
	float GetClipEndTime( const int& clipName ) const;

	void Set(
		std::vector<Bone>& boneHierarchy,
		std::vector<XMFLOAT4X4>& boneOffsets,
		std::map<int, AnimationClip>& animations );

	// ������ȯ�� ���ϴ� �Լ�, ������ ��ȯ���� ��ģ ��ȯ ��Ʈ�����̴�.
	void GetFinalTransforms( const int& clipName, float timePos,
		std::vector<XMFLOAT4X4>& finalTransforms );
	void SkinnedData::GetMatrixByTime( const int& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms );

public:
	// i�� ������ �θ��� �̸��� ��´�, i�� ����� �ִϸ��̼� Ŭ���� i��° BoneAnimaion�ν��Ͻ��� �����ȴ�.
	std::vector<Bone> mBoneHierarchy;

	// i�� ������ ������ ��ȯ, �θ�� ���� ������ ��ȯ
	std::vector<XMFLOAT4X4> mBoneOffsets;

	// �ִϸ��̼� ������ Ŭ������ ����
	std::map<int, AnimationClip> mAnimations;
};

struct CFbxVertex
{
	XMFLOAT3 m_position;
	XMFLOAT2 m_textureUV;
	XMFLOAT3 m_normal;
	XMFLOAT3 m_tangent;
	XMFLOAT3 m_binormal;

	XMFLOAT3 m_weights = XMFLOAT3( 0.0f, 0.0f, 0.0f );						// ����ġ
	XMFLOAT4 m_boneIndices = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );		// �� ������ ������ �ִ� ����
};

struct CFbxMesh
{
public:
	std::vector<TCHAR*> m_pTextures;
	std::vector<CFbxVertex> m_pVertices;
	std::vector<CFbxVertex> m_pVerticesPolygon;
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

	bool CheckRayIntersection( XMVECTOR *pvRayPosition, XMVECTOR *pvRatDirection, float *pHitDist, XMFLOAT3 *pIntersectionPos );

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
	void SetTextureCount( int count ) { m_nTextureCount = count; }
	int GetTextureCount( ){ return m_nTextureCount; }

protected:
	// �ؽ�ó ������ �ϱ� ���Ͽ� �ؽ�ó ��ǥ�� �ʿ�
	ID3D11Buffer *m_pd3dTexCoordBuffer;
	CTexture *m_pMeshTexture;
	int m_nTextureCount;
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

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
	SkinnedData *GetSkinnedData( ){ return &m_skindata; }
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

	//������ü�� ����, ����, ������ ũ�⸦ �����Ͽ� ������ü �޽��� �����Ѵ�.
	CCubeMeshDiffused( ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, D3DXCOLOR d3dxColor );
	virtual ~CCubeMeshDiffused( );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice );
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

//
//void DisplayAnimationRecursive( FbxAnimStack* pAnimStack, FbxNode* pNode, D3DXMATRIX* _mtx, Model* _model ){
//	int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>( );
//	FbxTime maxTime = pAnimStack->GetLocalTimeSpan( ).GetDuration( );      // �ִ� ����
//	FbxMatrix aniMatrix;
//	long long mtxSize = maxTime.GetMilliSeconds( ) / 10;
//
//	_mtx = new D3DXMATRIX[mtxSize];
//	//10milisecond
//	for (long long i = 0; i < mtxSize; i++){
//		FbxTime ntime;
//		ntime.SetMilliSeconds( i * 10 );
//
//		if (!pNode){
//			cout << "pNode ==0" << endl;
//		}
//		else{
//			aniMatrix = pNode->EvaluateGlobalTransform( ntime );       // �ð��뺰�� ��� ��Ʈ���� ����
//
//			_mtx[i]._11 = aniMatrix.mData[0].mData[0]; _mtx[i]._12 = aniMatrix.mData[0].mData[1]; _mtx[i]._13 = aniMatrix.mData[0].mData[2]; _mtx[i]._14 = aniMatrix.mData[0].mData[3];
//			_mtx[i]._21 = aniMatrix.mData[1].mData[0]; _mtx[i]._22 = aniMatrix.mData[1].mData[1]; _mtx[i]._23 = aniMatrix.mData[1].mData[2]; _mtx[i]._24 = aniMatrix.mData[1].mData[3];
//			_mtx[i]._31 = aniMatrix.mData[2].mData[0]; _mtx[i]._32 = aniMatrix.mData[2].mData[1]; _mtx[i]._33 = aniMatrix.mData[2].mData[2]; _mtx[i]._34 = aniMatrix.mData[2].mData[3];
//			_mtx[i]._41 = aniMatrix.mData[3].mData[0]; _mtx[i]._42 = aniMatrix.mData[3].mData[1]; _mtx[i]._43 = aniMatrix.mData[3].mData[2]; _mtx[i]._44 = aniMatrix.mData[3].mData[3];
//
//		}
//	}
//	_model->MaxAnimSize = mtxSize;
//	_model->SetAnimationMatrix( mtxSize, _mtx );
//
//	for (int i = 0; i < pNode->GetChildCount( ); i++){
//		DisplayAnimationRecursive( pAnimStack, pNode->GetChild( i ), _mtx, _model );
//	}
//}
