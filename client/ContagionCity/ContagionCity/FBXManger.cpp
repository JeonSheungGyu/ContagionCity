#include "stdafx.h"
#include "FBXManager.h"
#include <vector>
#include <fstream>
#include <string>

FBXManager::FBXManager( )
{
	m_pfbxManager = FbxManager::Create( );
	FbxIOSettings* pfbxIOSettings = FbxIOSettings::Create( m_pfbxManager, IOSROOT );
	m_pfbxManager->SetIOSettings( pfbxIOSettings );

	m_pfbxImporter = FbxImporter::Create( m_pfbxManager, "" );
	m_pfbxScene = FbxScene::Create( m_pfbxManager, "" );
}

FBXManager::~FBXManager( )
{
	m_pfbxImporter->Destroy( );
	m_pfbxManager->Destroy( );
}

// FBX 파일을 Scene의 노드에 추가하는 함수, 텍스처는 가변인자로 받음
bool FBXManager::LoadFBX( const char* pstrFileName, int Layer, int Type, int textureCount, ...)
{
	// 파일 이름을 로딩
	bool bResult = m_pfbxImporter->Initialize( pstrFileName, -1, m_pfbxManager->GetIOSettings( ) );
	if (!bResult)
		return false;

	// Scene에 추가, Scene의 루트노드의 하위로 들어가게됨
	bResult = m_pfbxImporter->Import( m_pfbxScene );

	if (!bResult)
		return false;

	FbxNode *pfbxRootNode = m_pfbxScene->GetRootNode( );

	if (pfbxRootNode)
	{
		int ChildCount = m_pfbxScene->GetRootNode( )->GetChildCount( );

		std::vector<UINT> tempIndex;
		std::vector<XMFLOAT2> tempUV;
		std::vector<Bone> tempBoneHierarachy;
		std::vector<XMFLOAT4X4> tempBoneOffsets;
		std::vector<CFbxVertex> tempVertices;
		std::map<int, AnimationClip> tempAnimations;
		std::vector<TCHAR*> tempTextures;

		// 텍스처 이름 저장
		va_list ap;
		va_start( ap, textureCount );
		for (int i = 0; i < textureCount; i++)
		{
			tempTextures.push_back( va_arg( ap, TCHAR* ) );
		}
		va_end( ap);

		// i를 0부터 시작하면 반드시 mesh가 먼저 시작되지만 i를 ChildCount 부터 감소시키면서 오면 반드시 skeleton이 먼저 시작된다
		for (int i = ChildCount - 1; i >= 0; i--)
	//	for (int i = 0; i < ChildCount; i++)
		{
			// 스키닝 데이터 로딩
			FbxNode* pfbxChildNode = pfbxRootNode->GetChild( i );
			FbxNodeAttribute *nodeAttribute = pfbxChildNode->GetNodeAttributeByIndex( 0 );
			if (nodeAttribute == NULL)
				continue;

			FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType( );
			FbxMesh* pMesh = (FbxMesh*)nodeAttribute;

			if (tempBoneHierarachy.size() == 0)	
				LoadBoneHierarachy( pfbxChildNode, &tempBoneHierarachy );

			switch (attributeType)
			{
				case FbxNodeAttribute::eMesh:
				{
					// 메시데이터 로딩
					// 매개변수로 std;;Vector<Bone> 넘기고 그 본과 현재 찾는 본 검색해서 인덱스 수정할것
					LoadFBXMeshData( pMesh, &tempVertices, &tempIndex, &tempUV, &tempBoneOffsets, tempBoneHierarachy );
					break;
				}
				case FbxNodeAttribute::eSkeleton:
				{
					tempAnimations = LoadBoneInfomation( pfbxChildNode, tempBoneHierarachy );
					break;
				}
			}
		}
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 자료들 저장하기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		SaveData( tempVertices, tempIndex, tempUV, tempBoneHierarachy, tempBoneOffsets, tempAnimations, Layer, Type, tempTextures );

		// 버텍스 정보들을 옮긴 뒤 노드들 제거
		for (int i = 0; i < m_pfbxScene->GetNodeCount( ); i++)
		{
			FbxNode* temp = m_pfbxScene->GetNode( i );
			m_pfbxScene->RemoveNode( temp );
		}
		return true;
	}
}

bool FBXManager::FindStringInString( std::string dstString, std::string Findword )
{
	if (dstString.find( Findword ) == std::string::npos)
		return false;
	else
		return true;
}

void FBXManager::LoadBoneHierarachy( FbxNode *pNode, std::vector<Bone> *pBoneHierarchy )
{ 
	Bone tempBone;

	int parentIdx = 0;

	if (pBoneHierarchy->size( ) == 0)
	{
		tempBone.boneName = pNode->GetName( );
		tempBone.parentBoneName = pNode->GetParent()->GetName( );
		tempBone.parentIdx = parentIdx;
		pBoneHierarchy->push_back( tempBone );
	}

	for (int i = 0; i < pNode->GetChildCount( ); i++)
	{
		
		for (int j = 0; j < pBoneHierarchy->size( ); j++)
		{
			if (( *pBoneHierarchy )[j].boneName == pNode->GetName( ))
			{
				parentIdx = j;
				break;
			}
		}
		
		if (!FindStringInString( pNode->GetChild(i)->GetName(), "Nub" ))
		{
			tempBone.boneName = pNode->GetChild( i )->GetName( );
			tempBone.parentBoneName = pNode->GetName( );
			tempBone.parentIdx = parentIdx;
			pBoneHierarchy->push_back( tempBone );
		}
		LoadBoneHierarachy( pNode->GetChild( i ), pBoneHierarchy );
	}
}

void FBXManager::LoadInfluenceWeight( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices, std::vector<XMFLOAT4X4> *pBoneOffsets, std::vector<Bone> BoneHierarchy )
{
	// 본 계층구조, 오프셋변환, 가중치, 영향받는 정점 을 구해야함
	int numDeformers = pMesh->GetDeformerCount( FbxDeformer::eSkin);

	for (int deformerCount = 0; deformerCount < numDeformers; deformerCount++)
	{
		FbxVector4* pIndexedVertex = pMesh->GetControlPoints( );

		// 정점 좌표들을 저장할 공간
		int polygonCount = pMesh->GetPolygonCount( );		// 폴리곤의 개수
		int vertexCount = pMesh->GetControlPointsCount( );
		std::vector<CFbxVertex> IndexVertex;

		for (int polyCount = 0; polyCount < vertexCount; polyCount++)
		{
			CFbxVertex tempVertex;
			tempVertex.m_position = XMFLOAT3( pIndexedVertex[polyCount].mData[0], pIndexedVertex[polyCount].mData[1], pIndexedVertex[polyCount].mData[2] );
			IndexVertex.push_back( tempVertex );
		}

		FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer( deformerCount, FbxDeformer::eSkin );
		if (!skin)
			continue;

		pBoneOffsets->resize( BoneHierarchy.size( ) );
		// 본의 정보를 얻어오는 행위
		int boneCount = skin->GetClusterCount( );		// 본의 개수		 54
		for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			// cluster == bone
			FbxCluster *cluster = skin->GetCluster( boneIndex );
			FbxNode *bone = cluster->GetLink( );		// 본과 연결되어있는 노드, 부모노드가 아님
			int boneHierIdx = 0;

			for (int boneHier = 0; boneHier < BoneHierarchy.size( ); boneHier++)
			{
				if (bone->GetName( ) == BoneHierarchy[boneHier].boneName)
				{
					boneHierIdx = boneHier;
					break;
				}
			}

			FbxAMatrix LinkBoneMatrix;
			FbxAMatrix TransboneMatrix;
			FbxAMatrix ResultMatrix;

			cluster->GetTransformLinkMatrix( LinkBoneMatrix );
			cluster->GetTransformMatrix( TransboneMatrix );
			ResultMatrix = LinkBoneMatrix.Inverse( )  *TransboneMatrix;

			XMFLOAT4X4 tempOffsetMatl;
			tempOffsetMatl._11 = ResultMatrix.mData[0].mData[0]; tempOffsetMatl._12 = ResultMatrix.mData[0].mData[1];
			tempOffsetMatl._13 = ResultMatrix.mData[0].mData[2]; tempOffsetMatl._14 = ResultMatrix.mData[0].mData[3];
			tempOffsetMatl._21 = ResultMatrix.mData[1].mData[0]; tempOffsetMatl._22 = ResultMatrix.mData[1].mData[1];
			tempOffsetMatl._23 = ResultMatrix.mData[1].mData[2]; tempOffsetMatl._24 = ResultMatrix.mData[1].mData[3];
			tempOffsetMatl._31 = ResultMatrix.mData[2].mData[0]; tempOffsetMatl._32 = ResultMatrix.mData[2].mData[1];
			tempOffsetMatl._33 = ResultMatrix.mData[2].mData[2]; tempOffsetMatl._34 = ResultMatrix.mData[2].mData[3];
			tempOffsetMatl._41 = ResultMatrix.mData[3].mData[0]; tempOffsetMatl._42 = ResultMatrix.mData[3].mData[1];
			tempOffsetMatl._43 = ResultMatrix.mData[3].mData[2]; tempOffsetMatl._44 = ResultMatrix.mData[3].mData[3];
			
			(*pBoneOffsets)[boneHierIdx] = tempOffsetMatl;	
		
			double *boneVertexWeights = cluster->GetControlPointWeights( );		// 해당 본에 의한 정점의 가중치
			int *boneVertexIndices = cluster->GetControlPointIndices( );					// 해당 본에 영향을 받는 정점들
			// 해당 본에 영향을 받는 모든 정점을 하나씩 가져옴	
			int numBoneVertexIndices = cluster->GetControlPointIndicesCount( );		

			for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
			{
				float tempBoneWeight = (float)boneVertexWeights[boneVertexIndex];	// 영향을 받는 정점의 가중치 정도

				// 가중치가 0이면 다음걸로 넘김
				if (!tempBoneWeight)
					continue;

				int tempBoneVertexIndex = boneVertexIndices[boneVertexIndex];			// 영향을 받는 정점의 인덱스

				// 가중치 중 x가 0이면 첫번째 인덱스
				if (IndexVertex[tempBoneVertexIndex].m_weights.x == 0)
				{
					IndexVertex[tempBoneVertexIndex].m_weights.x = tempBoneWeight;
					IndexVertex[tempBoneVertexIndex].m_boneIndices.x = boneHierIdx;
				}
				// 가중치 중 x가 0이 아니고 y가 0이면 두번째 인덱스
				else if (IndexVertex[tempBoneVertexIndex].m_weights.x != 0 && IndexVertex[tempBoneVertexIndex].m_weights.y == 0)
				{
					IndexVertex[tempBoneVertexIndex].m_weights.y = tempBoneWeight;
					IndexVertex[tempBoneVertexIndex].m_boneIndices.y = boneHierIdx;
				}
				// 가중치 중 x가 0이 아니고 y가 0이 아니고 z가 0이면 세번째 인덱스
				else if (IndexVertex[tempBoneVertexIndex].m_weights.x != 0 && IndexVertex[tempBoneVertexIndex].m_weights.y != 0 && IndexVertex[tempBoneVertexIndex].m_weights.z == 0)
				{
					IndexVertex[tempBoneVertexIndex].m_weights.z = tempBoneWeight;
					IndexVertex[tempBoneVertexIndex].m_boneIndices.z = boneHierIdx;
				}
				// 모두 0이 아니면 4번째 인덱스, 가중치는 1에서 xyz 빼면 나머지 구할 수 있음
				else if (IndexVertex[tempBoneVertexIndex].m_weights.x != 0 && IndexVertex[tempBoneVertexIndex].m_weights.y != 0 && IndexVertex[tempBoneVertexIndex].m_weights.z != 0)
				{
					IndexVertex[tempBoneVertexIndex].m_boneIndices.w = boneHierIdx;
				}
			}
		}	// end of bonecount for

		for (int i = 0; i < pVertices->size( ); i++)
		{
			if (pVertices->data( )[i].m_boneIndices.x == 0 && pVertices->data()[i].m_weights.x == 0)
			{
				CFbxVertex tempVertex = pVertices->data( )[i];
				for (int j = 0; j < IndexVertex.size( ); j++)
				{
					CFbxVertex tempVertex2 = IndexVertex[j];

					if (tempVertex.m_position.x == tempVertex2.m_position.x && tempVertex.m_position.y == tempVertex2.m_position.y && tempVertex.m_position.z == tempVertex2.m_position.z)
					{
						pVertices->data( )[i].m_weights.x = IndexVertex[j].m_weights.x;
						pVertices->data( )[i].m_weights.y = IndexVertex[j].m_weights.y;
						pVertices->data( )[i].m_weights.z = IndexVertex[j].m_weights.z;

						pVertices->data( )[i].m_boneIndices.x = IndexVertex[j].m_boneIndices.x;
						pVertices->data( )[i].m_boneIndices.y = IndexVertex[j].m_boneIndices.y;
						pVertices->data( )[i].m_boneIndices.z = IndexVertex[j].m_boneIndices.z;
						pVertices->data( )[i].m_boneIndices.w = IndexVertex[j].m_boneIndices.w;
						break;
					}
				}
			}
		}

	}	// end of deformer for
}

std::map<int, AnimationClip> FBXManager::LoadBoneInfomation( FbxNode* pNode, std::vector<Bone> BoneHierarchy )
{
	bool isAnimated = false;
	std::map<int, AnimationClip> tempAnimations;
	AnimationClip animation;
	std::string animName;

	animation.BoneAnimations.resize( BoneHierarchy.size( ) );
	// 애니메이션클립을 찾는 행위
	int numAnimations = m_pfbxScene->GetSrcObjectCount<FbxAnimStack>();
	for (int animIndex = 1; animIndex < numAnimations; animIndex++)
	{
		FbxAnimStack *animStack = ( FbxAnimStack* )this->m_pfbxScene->GetSrcObject<FbxAnimStack>(animIndex);
		animName = animStack->GetName( );	// 이름

		LoadKeyframesByTime( animStack, pNode, &( animation.BoneAnimations ), BoneHierarchy );

		if (animation.BoneAnimations.size() != 0)
			tempAnimations.insert( { animIndex-1, animation } );
	}	// end of animation for
	return tempAnimations;
}

void FBXManager::LoadKeyframesByTime( FbxAnimStack *pAnimStack, FbxNode *pNode, std::vector<BoneAnimation>* pvAnimations, std::vector<Bone> BoneHierarchy )
{
	int BoneIdx = -1;
	std::string boneName = pNode->GetName( );

	for (int i = 0; i < BoneHierarchy.size( ); i++)
	{
		if (BoneHierarchy[i].boneName == boneName)
		{
			BoneIdx = i;
			break;
		}
	}

	// BoneIdx 가 -1일 수는 없음, 0부터 시작하기 때문
	// 위의 for을 검사하고도 BoneIdx가 -1이라는 소리는 본계층구조에 없는 노드라는 의미
	// 따라서 밑의 검사를 할 필요가 없음
	if (BoneIdx == -1)
		return;

	int stacks = pAnimStack->GetMemberCount<FbxAnimLayer>( );
	for (int i = 0; i < stacks; i++)
	{
		FbxAnimLayer *pfbxAnimLayer = pAnimStack->GetMember<FbxAnimLayer>( i );
		FbxAnimCurve *pfbxAnimCurve = pNode->LclRotation.GetCurve( pfbxAnimLayer, "X" );

		if (!pfbxAnimCurve)
			continue;
		long long mtxSize = pfbxAnimCurve->KeyGetCount( );

		if (mtxSize != 0)
		{
			std::vector<Keyframe> _mtx( mtxSize );
			BoneAnimation tempBoneAnim;

			float StartTime = (float)pfbxAnimCurve->KeyGetTime( 0 ).GetSecondDouble( );
			for (long long i = 0; i < mtxSize; i++)
			{
				FbxTime nTime = pfbxAnimCurve->KeyGetTime( i );

				if (pNode)
				{
					// 해당 본의 월드좌표계에서의 행렬
					_mtx[i].TimePos = (float)nTime.GetSecondDouble( ) - StartTime;		// 시간저장

					FbxAMatrix aniMatrix = pNode->EvaluateLocalTransform( nTime );
					FbxQuaternion Q = aniMatrix.GetQ( );
					FbxVector4 S = aniMatrix.GetS( );
					FbxVector4 T = aniMatrix.GetT( );

					_mtx[i].RotationQuat.x = Q[0]; _mtx[i].RotationQuat.y = Q[1]; _mtx[i].RotationQuat.z = Q[2]; _mtx[i].RotationQuat.w = Q[3];
					_mtx[i].Scale.x = S[0]; _mtx[i].Scale.y = S[1]; _mtx[i].Scale.z = S[2];
					_mtx[i].Translation.x = T[0]; _mtx[i].Translation.y = T[1]; _mtx[i].Translation.z = T[2];
				}
			}

			// 한 순간의 애니메이션 매트릭스
			tempBoneAnim.Keyframes = _mtx;
			tempBoneAnim.BoneName = pNode->GetName( );
			tempBoneAnim.PerentBoneName = pNode->GetParent( )->GetName( );
			tempBoneAnim.BoneIndex = BoneIdx;

			( *pvAnimations )[BoneIdx] = tempBoneAnim;
		}
	}

	for (int i = 0; i < pNode->GetChildCount( ); i++)
	{
		LoadKeyframesByTime( pAnimStack, pNode->GetChild( i ), pvAnimations, BoneHierarchy );
	}
}

void FBXManager::LoadFBXMeshData( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertices, std::vector<UINT> *pIndices,
	std::vector<XMFLOAT2> *pUVs, std::vector<XMFLOAT4X4> *pBoneOffsets, std::vector<Bone> BoneHierarchy )
{
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 정점 좌표, 인덱스 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadVertexAndIndexInfomation( pMesh, pVertices, pIndices );

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ normal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadNormallnfomation( pMesh, pVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ tangent 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//	LoadTangentInfomation( pMesh, pVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ binormal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//	LoadBinormalInfomation( pMesh, pVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ UV 좌표 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadUVInformation( pMesh, pUVs );

//	LoadMeshData( pMesh, pVertices, pIndices );
	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ  가중치와 뼈대 정보가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadInfluenceWeight( pMesh, pVertices, pBoneOffsets, BoneHierarchy );
}

void FBXManager::SaveData( std::vector<CFbxVertex> Vertex, std::vector<UINT> Index, std::vector<XMFLOAT2> UVVector, 
	std::vector<Bone> BoneHierarchy, std::vector<XMFLOAT4X4> BoneOffsets, std::map<int, AnimationClip> Animations, int iLayer, int iType, std::vector<TCHAR*> textures )
{
	CFbxMesh tempMesh;

	tempMesh.m_pVertices.resize( Vertex.size( ) );
	tempMesh.m_pVertices = Vertex;

	for (int i = 0; i < tempMesh.m_pVertices.size( ); i++)
	{
		tempMesh.m_pVertices[i].m_textureUV.x = UVVector[i].x;
		tempMesh.m_pVertices[i].m_textureUV.y = UVVector[i].y;
	}
	// vertex 개수
	tempMesh.m_nVertexCount = Vertex.size( );
	// 인덱스들의 모임
	tempMesh.m_pvIndices = Index;
	// index 개수
	tempMesh.m_nIndexCount = Index.size( );
	// 레이어
	tempMesh.m_iLayer = iLayer;
	// 타입
	tempMesh.m_iType = iType;

	// 스키닝 데이터
	tempMesh.m_skinnedData.Set( BoneHierarchy, BoneOffsets, Animations );
	// 텍스처이름
	tempMesh.m_pTextures = textures;

	m_pMeshes.push_back( tempMesh );
}

void FBXManager::LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertex, std::vector<UINT> *pIndex )
{
	FbxVector4* pVertices = pMesh->GetControlPoints();

	// 정점 좌표들을 저장할 공간
	int polygonCount = pMesh->GetPolygonCount( );		// 폴리곤의 개수
	int vertexCount = pMesh->GetControlPointsCount( );

	//for (int polyCount = 0; polyCount < vertexCount; polyCount++)
	//{
	//	CFbxVertex tempVertex;
	//	tempVertex.m_position = XMFLOAT3( pVertices[polyCount].mData[0], pVertices[polyCount].mData[1], pVertices[polyCount].mData[2] );
	//	pVertex->push_back( tempVertex );
	//}

	//int iIndexCount = pMesh->GetPolygonVertexCount( );
	////int* indices = new int[iIndexCount];
	//int* indices = pMesh->GetPolygonVertices( );
	//for (int idx = 0; idx < iIndexCount; idx++)
	//{
	//	pIndex->push_back( indices[idx] );
	//}

	int count = 0;
	for (int j = 0; j < polygonCount; j++)
	{
		int iNumVertices = pMesh->GetPolygonSize( j );	// 폴리곤을 구성하는 정점의 개수
		if (iNumVertices != 3)
		{
			//	pMesh->Destroy( );
			//	return;
		}

		for (int k = 0; k < iNumVertices; k++)
		{
			int iControlPointIndex = pMesh->GetPolygonVertex( j, k );
			pIndex->push_back( count++ );

			CFbxVertex temp;
			temp.m_position.x = (float)pVertices[iControlPointIndex].mData[0];
			temp.m_position.y = (float)pVertices[iControlPointIndex].mData[1];
			temp.m_position.z = (float)pVertices[iControlPointIndex].mData[2];

			pVertex->push_back( temp );
		}
	}

//	LoadGeometry( pMesh, pVertex );
}

void FBXManager::LoadMeshData( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertex, std::vector<UINT> *pIndex )
{
	// 정점 좌표들을 저장할 공간
	std::vector<CFbxVertex> pPolygonVertex;

	FbxVector4* pVertices = pMesh->GetControlPoints( );

	int polygonCount = pMesh->GetPolygonCount( );		// 폴리곤의 개수
	int vertexCount = pMesh->GetControlPointsCount( );

	for (int polyCount = 0; polyCount < vertexCount; polyCount++)
	{
		CFbxVertex tempVertex;
		tempVertex.m_position = XMFLOAT3( pVertices[polyCount].mData[0], pVertices[polyCount].mData[1], pVertices[polyCount].mData[2] );
		pVertex->push_back( tempVertex );
	}

	int iIndexCount = pMesh->GetPolygonVertexCount( );
	int* indices = pMesh->GetPolygonVertices( );
	for (int idx = 0; idx < iIndexCount; idx++)
	{
		pIndex->push_back( indices[idx] );
	}

	LoadGeometry( pMesh, &pPolygonVertex );
	
	//폴리곤버텍스와 ControlPoint 병합하기
	for (int i = 0; i < pVertex->size( ); i++)
	{
		XMFLOAT3 ControlPoint = pVertex->data()[i].m_position;
		for (int j = 0; j < pPolygonVertex.size( ); j++)
		{
			XMFLOAT3 PolygonVertex = pPolygonVertex[j].m_position;

			if (ControlPoint.x == PolygonVertex.x && ControlPoint.y == PolygonVertex.y && ControlPoint.z == PolygonVertex.z)
			{
				pVertex->data()[i] = pPolygonVertex[j];
				break;
			}
		}
	}
}

void FBXManager::LoadGeometry( FbxMesh* pMesh,  std::vector<CFbxVertex> *pVertex )
{
	// polygons
	int countPolys = pMesh->GetPolygonCount( );

	int vertexID = 0;
	for (int i = 0; i < countPolys; i++)
	{
		int countPolyVerts = pMesh->GetPolygonSize( i );

		for (int j = 0; j < countPolyVerts; j++)
		{
			CFbxVertex vertex;

			//---------------------------------------------------------
			// position
			//---------------------------------------------------------
			FbxVector4* pVertices = pMesh->GetControlPoints( );

			int cp_index = pMesh->GetPolygonVertex( i, j );
			vertex.m_position.x = (float)pVertices[cp_index].mData[0];
			vertex.m_position.y = (float)pVertices[cp_index].mData[1];
			vertex.m_position.z = (float)pVertices[cp_index].mData[2];

			//---------------------------------------------------------
			// normal
			//---------------------------------------------------------
			FbxVector4 fbxNormal;
			FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal( 0 );
			if (leNormal == nullptr)
			{
				//				ERRPRINT("No normals in mesh");
			}
			else
			{
				FbxLayerElement::EMappingMode normals_MapMode = leNormal->GetMappingMode( );
				if (normals_MapMode == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leNormal->GetReferenceMode( ))
					{
						case FbxGeometryElement::eDirect:
							fbxNormal = leNormal->GetDirectArray( ).GetAt( vertexID );
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray( ).GetAt( vertexID );
							fbxNormal = leNormal->GetDirectArray( ).GetAt( id );
						}
						break;
						default:
							break;
					}
				}
				else if (normals_MapMode == FbxGeometryElement::eByControlPoint)
				{
					switch (leNormal->GetReferenceMode( ))
					{
						case FbxGeometryElement::eDirect:
							fbxNormal = leNormal->GetDirectArray( ).GetAt( cp_index );
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray( ).GetAt( cp_index );
							fbxNormal = leNormal->GetDirectArray( ).GetAt( id );
						}
						break;
						default:
							break;
					}
				}
				vertex.m_normal.x = fbxNormal.mData[0];
				vertex.m_normal.y = fbxNormal.mData[1];
				vertex.m_normal.z = fbxNormal.mData[2];
			}

			//---------------------------------------------------------
			// tangent
			//---------------------------------------------------------
			FbxVector4 fbxTangent;
			FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent( 0 );
			if (leTangent == nullptr)
			{
				//				ERRPRINT("No tangents in mesh");
			}
			else
			{
				FbxLayerElement::EMappingMode tangent_MapMode = leTangent->GetMappingMode( );
				if (tangent_MapMode == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leTangent->GetReferenceMode( ))
					{
						case FbxGeometryElement::eDirect:
							fbxTangent = leTangent->GetDirectArray( ).GetAt( vertexID );
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leTangent->GetIndexArray( ).GetAt( vertexID );
							fbxTangent = leTangent->GetDirectArray( ).GetAt( id );
						}
						break;
						default:
							break;
					}
				}
				else if (tangent_MapMode == FbxGeometryElement::eByControlPoint)
				{
					switch (leTangent->GetReferenceMode( ))
					{
						case FbxGeometryElement::eDirect:
							fbxTangent = leTangent->GetDirectArray( ).GetAt( cp_index );
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leTangent->GetIndexArray( ).GetAt( cp_index );
							fbxTangent = leTangent->GetDirectArray( ).GetAt( id );
						}
						break;
						default:
							break;
					}
				}
				vertex.m_tangent.x = fbxTangent.mData[0];
				vertex.m_tangent.y = fbxTangent.mData[1];
				vertex.m_tangent.z = fbxTangent.mData[2];
			}

			//---------------------------------------------------------
			// binormal
			//---------------------------------------------------------
			FbxVector4 fbxBinormal;
			FbxGeometryElementBinormal* leBinormal = pMesh->GetElementBinormal( 0 );
			if (leBinormal == nullptr)
			{
				//				ERRPRINT( "No binormals in mesh" );
			}
			else
			{
				FbxLayerElement::EMappingMode binormal_MapMode = leTangent->GetMappingMode( );
				if (binormal_MapMode == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leBinormal->GetReferenceMode( ))
					{
						case FbxGeometryElement::eDirect:
							fbxBinormal = leBinormal->GetDirectArray( ).GetAt( vertexID );
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leBinormal->GetIndexArray( ).GetAt( vertexID );
							fbxBinormal = leBinormal->GetDirectArray( ).GetAt( id );
						}
						break;
						default:
							break;
					}
				}
				else if (binormal_MapMode == FbxGeometryElement::eByControlPoint)
				{
					switch (leBinormal->GetReferenceMode( ))
					{
						case FbxGeometryElement::eDirect:
							fbxBinormal = leBinormal->GetDirectArray( ).GetAt( cp_index );
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leBinormal->GetIndexArray( ).GetAt( cp_index );
							fbxBinormal = leBinormal->GetDirectArray( ).GetAt( id );
						}
						break;
						default:
							break;
					}
				}
				vertex.m_binormal.x = fbxBinormal.mData[0];
				vertex.m_binormal.y = fbxBinormal.mData[1];
				vertex.m_binormal.z = fbxBinormal.mData[2];
			}

			//---------------------------------------------------------
			// uv
			//---------------------------------------------------------
			FbxVector2 uv;
			FbxGeometryElementUV* leUV = pMesh->GetElementUV( 0 );
			if (leUV == nullptr)
			{
			}
			else
			{
				switch (leUV->GetMappingMode( ))
				{
					case FbxGeometryElement::eByControlPoint:
					{
						switch (leUV->GetReferenceMode( ))
						{
							case FbxGeometryElement::eDirect:
								uv = leUV->GetDirectArray( ).GetAt( cp_index );
								break;
							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leUV->GetIndexArray( ).GetAt( cp_index );
								uv = leUV->GetDirectArray( ).GetAt( id );
							}
							break;
							default:
								break;
						}
					}
					break;

					case FbxGeometryElement::eByPolygonVertex:
					{
						int lTextureUVIndex = pMesh->GetTextureUVIndex( i, j );
						switch (leUV->GetReferenceMode( ))
						{
							case FbxGeometryElement::eDirect:
							case FbxGeometryElement::eIndexToDirect:
							{
								uv = leUV->GetDirectArray( ).GetAt( lTextureUVIndex );
							}
							break;
							default:
								break;
						}
					}
					default:
						break;
				}
				vertex.m_textureUV.x = (float)uv.mData[0];
				vertex.m_textureUV.y = 1.0f - (float) uv.mData[1];
			}

			++vertexID;
			pVertex->push_back( vertex );
		}
	}
}

void FBXManager::LoadTangentInfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices )
{
	int polygonVertexCount = pMesh->GetPolygonVertexCount( );

	for (int i = 0; i < polygonVertexCount; i++)
	{
		XMFLOAT3 tempTangent;
		FbxGeometryElementTangent* vertexTangent = pMesh->GetElementTangent( 0 );
		if (!vertexTangent)
			continue;

		switch (vertexTangent->GetMappingMode( ))
		{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexTangent->GetReferenceMode( ))
				{
					case FbxGeometryElement::eDirect:
					{
						tempTangent.x = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[0] );
						tempTangent.y = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[2] );
						tempTangent.z = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[1] );
			//			tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[3] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int index = vertexTangent->GetIndexArray( ).GetAt( i );
						tempTangent.x = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[0] );
						tempTangent.y = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[2] );
						tempTangent.z = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[1] );
			//			tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[3] );
					}
					break;
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexTangent->GetReferenceMode( ))
				{
					case FbxGeometryElement::eDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						tempTangent.x = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[0] );
						tempTangent.y = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[2] );
						tempTangent.z = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[1] );
			//			tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[3] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						int index = vertexTangent->GetIndexArray( ).GetAt( inVertexCounter );
						tempTangent.x = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[0] );
						tempTangent.y = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[2] );
						tempTangent.z = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[1] );
			//			tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[3] );
					}
					break;
				}
				break;
		}
		(*pVertices)[i].m_tangent = tempTangent;
	}
}

void FBXManager::LoadBinormalInfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices )
{
	int polygonVertexCount = pMesh->GetPolygonVertexCount( );

	for (int i = 0; i < polygonVertexCount; i++)
	{
		XMFLOAT3 tempBinomal;
		FbxGeometryElementBinormal* vertexBinormal = pMesh->GetElementBinormal( 0 );
		if (!vertexBinormal)
			continue;

		switch (vertexBinormal->GetMappingMode( ))
		{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexBinormal->GetReferenceMode( ))
				{
					case FbxGeometryElement::eDirect:
					{
						tempBinomal.x = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( i ).mData[0] );
						tempBinomal.y = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( i ).mData[2] );
						tempBinomal.z = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( i ).mData[1] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int index = vertexBinormal->GetIndexArray( ).GetAt( i );
						tempBinomal.x = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( index ).mData[0] );
						tempBinomal.y = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( index ).mData[2] );
						tempBinomal.z = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( index ).mData[1] );
					}
					break;
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexBinormal->GetReferenceMode( ))
				{
					case FbxGeometryElement::eDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						tempBinomal.x = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( i ).mData[0] );
						tempBinomal.y = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( i ).mData[2] );
						tempBinomal.z = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( i ).mData[1] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						int index = vertexBinormal->GetIndexArray( ).GetAt( inVertexCounter );
						tempBinomal.x = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( index ).mData[0] );
						tempBinomal.y = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( index ).mData[2] );
						tempBinomal.z = static_cast<float>( vertexBinormal->GetDirectArray( ).GetAt( index ).mData[1] );
					}
					break;
				}
				break;
		}
		( *pVertices )[i].m_binormal = tempBinomal;
	}
}

void FBXManager::LoadNormallnfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices )
{
	int polygonVertexCount = pMesh->GetPolygonVertexCount( );

	for (int i = 0; i < polygonVertexCount; i++)
	{
		XMFLOAT3 tempNormal;
		FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal( 0 );
		if (!vertexNormal)
			continue;

		switch (vertexNormal->GetMappingMode( ))
		{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexNormal->GetReferenceMode( ))
				{
					case FbxGeometryElement::eDirect:
					{
						tempNormal.x = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( i ).mData[0] );
						tempNormal.y = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( i ).mData[2] );
						tempNormal.z = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( i ).mData[1] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int index = vertexNormal->GetIndexArray( ).GetAt( i );
						tempNormal.x = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( index ).mData[0] );
						tempNormal.y = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( index ).mData[2] );
						tempNormal.z = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( index ).mData[1] );
					}
					break;
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexNormal->GetReferenceMode( ))
				{
					case FbxGeometryElement::eDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						tempNormal.x = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( i ).mData[0] );
						tempNormal.y = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( i ).mData[2] );
						tempNormal.z = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( i ).mData[1] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						int index = vertexNormal->GetIndexArray( ).GetAt( inVertexCounter );
						tempNormal.x = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( index ).mData[0] );
						tempNormal.y = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( index ).mData[2] );
						tempNormal.z = static_cast<float>( vertexNormal->GetDirectArray( ).GetAt( index ).mData[1] );
					}
					break;
				}
				break;
		}
		( *pVertices )[i].m_normal = tempNormal;
	}
}

void FBXManager::LoadUVInformation( FbxMesh* pMesh, std::vector<XMFLOAT2> *pVertices )
{
	//get all UV set names
	FbxStringList lUVSetNameList;
	pMesh->GetUVSetNames( lUVSetNameList );
	int NameListCount = lUVSetNameList.GetCount( );

	//iterating over all uv sets
	for (int lUVSetIndex = 0; lUVSetIndex < 1; lUVSetIndex++)
	{
		//get lUVSetIndex-th uv set
		const char* lUVSetName = lUVSetNameList.GetStringAt( lUVSetIndex );
		const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV( lUVSetName );

		if (!lUVElement)
			continue;

		// only support mapping mode eByPolygonVertex and eByControlPoint
		if (lUVElement->GetMappingMode( ) != FbxGeometryElement::eByPolygonVertex &&
			lUVElement->GetMappingMode( ) != FbxGeometryElement::eByControlPoint)
			return;

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode( ) != FbxGeometryElement::eDirect;
		const int lIndexCount = ( lUseIndex ) ? lUVElement->GetIndexArray( ).GetCount( ) : 0;

		//iterating through the data by polygon
		const int lPolyCount = pMesh->GetPolygonCount( );

		if (lUVElement->GetMappingMode( ) == FbxGeometryElement::eByControlPoint)
		{
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize( lPolyIndex );
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					FbxVector2 lUVValue;

					//get the index of the current vertex in control points array
					int lPolyVertIndex = pMesh->GetPolygonVertex( lPolyIndex, lVertIndex );

					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray( ).GetAt( lPolyVertIndex ) : lPolyVertIndex;

					lUVValue = lUVElement->GetDirectArray( ).GetAt( lUVIndex );

					XMFLOAT2 temp( lUVValue.mData[0], lUVValue.mData[1] );
					pVertices->push_back( temp );
				}
			}
		}
		else if (lUVElement->GetMappingMode( ) == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize( lPolyIndex );
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					if (lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						int lTextureUVIndex = pMesh->GetTextureUVIndex( lPolyIndex, lVertIndex );

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray( ).GetAt( lPolyIndexCounter ) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray( ).GetAt( lTextureUVIndex );

						XMFLOAT2 temp( (float)lUVValue.mData[0], 1-(float)lUVValue.mData[1] );
						pVertices->push_back( temp );

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}