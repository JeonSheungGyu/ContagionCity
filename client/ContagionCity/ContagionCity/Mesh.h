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
	// 정점의 위치 정보를 저장하기 위한 멤버 변수를 선언
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

// 재질정보
struct Material
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

// 애니메이션 정보를 가지는 프레임, 특정 시간에서의 변환 정보를 가짐
// 시간에 따른 값들을 저장함
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

// 하나의 뼈대, 애니메이션 정보들을 가지고 있음
struct BoneAnimation		// CAnimation
{
	// 시간에 따른 변환 값들을 저장, 뼈대가 움직이는 모양
	std::vector<Keyframe> Keyframes;
	// 본 이름
	std::string BoneName;
	// 부모 이름
	std::string PerentBoneName;
	// 본 인덱스
	int BoneIndex;

	float GetStartTime( ) const;
	float GetEndTime( ) const;

	void Interpolate( float t, XMFLOAT4X4& M );
};

// 애니메이션 클립 만들기, 걷기, 뛰기, 공격 등의 개별적인 애니메이션을 의미함
struct AnimationClip		// CAnimationSet
{
	// 현재 클립의 모든 뼈대의 시작 시간 중 가장 빠른 것을 반환
	float GetClipStartTime( ) const;
	// 현재 클립의 모든 뼈대의 종료 시간 중 가장 늦은 것을 반환
	float GetClipEndTime( ) const;

	// 이 클립의 각 BoneAnimation을 훑으면서 애니메이션들을 보간
	void Interpolate( float t, std::vector<XMFLOAT4X4>& boneTransforms );

	// 뼈대별 애니메이션들
	std::vector<BoneAnimation> BoneAnimations;
	
	// 뼈대 찾기
	BoneAnimation* FindBone( int index );
	BoneAnimation* FindBone( std::string name );
};

// 스키닝 애니메이션 자료를 담을 클래스
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

	// 최종변환을 구하는 함수, 오프셋 변환까지 마친 변환 매트릭스이다.
	void GetFinalTransforms( const int& clipName, float timePos,
		std::vector<XMFLOAT4X4>& finalTransforms );
	void SkinnedData::GetMatrixByTime( const int& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms );

public:
	// i번 뼈대의 부모의 이름을 담는다, i번 뼈대는 애니메이션 클립의 i번째 BoneAnimaion인스턴스에 대응된다.
	std::vector<Bone> mBoneHierarchy;

	// i번 뼈대의 오프셋 변환, 부모로 가는 오프셋 변환
	std::vector<XMFLOAT4X4> mBoneOffsets;

	// 애니메이션 정보를 클립별로 저장
	std::map<int, AnimationClip> mAnimations;
};

struct CFbxVertex
{
	XMFLOAT3 m_position;
	XMFLOAT2 m_textureUV;
	XMFLOAT3 m_normal;
	XMFLOAT3 m_tangent;
	XMFLOAT3 m_binormal;

	XMFLOAT3 m_weights = XMFLOAT3( 0.0f, 0.0f, 0.0f );						// 가중치
	XMFLOAT4 m_boneIndices = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );		// 이 정점에 영향을 주는 뼈대
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
	std::vector<XMFLOAT3> m_vPositions;
	// 메시의 인덱스를 저장
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
	void SetBoundingCube( XMFLOAT3 max, XMFLOAT3 min );
	void SetBoundingCube( AABB boundingBox );

	// 메시의 정점 버퍼들을 배열로 조립
	void AssembleToVertexBuffer( int nBuffers = 0, ID3D11Buffer **m_pd3dBuffers = NULL, UINT *pnBufferStrides = NULL, UINT *pnBufferOffset = NULL );

	virtual void CreateRasterizerState( ID3D11Device *pd3dDevice ){}
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext );
	// 인스턴싱을 이용하여 렌더링
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
	// Cull Mode는 앞면을 그릴 것인지 뒷면을 그릴것인지
	// ClockWise는 와인딩오더 설정, TRUE 이면 반시계, FALSE이면 시계
	// Fill Mode는 솔리드로 할것인지 와이어프레임으로 할것인지
	virtual void ChangeRasterizerState( ID3D11Device* pd3dDevice, bool ClockWise, D3D11_CULL_MODE CullMode, D3D11_FILL_MODE FillMode );
	void SetTextureCount( int count ) { m_nTextureCount = count; }
	int GetTextureCount( ){ return m_nTextureCount; }

protected:
	// 텍스처 매핑을 하기 위하여 텍스처 좌표가 필요
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

	// 정점 좌표
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

	//직육면체의 가로, 세로, 높이의 크기를 지정하여 직육면체 메쉬를 생성한다.
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
//			ResultMatrix = LinkBoneMatrix.Inverse( )*TransBoneMatrix;      // 지역행렬?
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
//				// bone 정보에 따라서 가중치 부여
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
//	FbxTime maxTime = pAnimStack->GetLocalTimeSpan( ).GetDuration( );      // 최대 길이
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
//			aniMatrix = pNode->EvaluateGlobalTransform( ntime );       // 시간대별로 노드 매트릭스 정보
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
