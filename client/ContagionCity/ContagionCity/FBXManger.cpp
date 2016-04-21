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

// FBX 파일을 Scene의 노드에 추가하는 함수
bool FBXManager::LoadFBX( const char* pstrFileName, int Layer, int Type )
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
		
		for (int i = 0; i < ChildCount; i++)
		{
			FbxNode* pfbxChildNode = pfbxRootNode->GetChild( i );
			FbxNodeAttribute *nodeAttribute = pfbxChildNode->GetNodeAttributeByIndex( 0 );
			if (nodeAttribute == NULL)
				continue;

			FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType( );
			FbxMesh* pMesh = (FbxMesh*)nodeAttribute;

			switch (attributeType)
			{
				case FbxNodeAttribute::eMesh:
				{
					// 메시데이터 로딩
					LoadFBXMeshData( pMesh, &tempVertices, &tempIndex, &tempUV, &tempBoneHierarachy, &tempBoneOffsets );
					break;
				}
				case FbxNodeAttribute::eSkeleton:
				{
					// 스키닝 데이터 로딩
	//				LoadBoneHierarachy( pfbxRootNode, &tempBoneHierarachy );
					tempAnimations = LoadBoneInfomation( pfbxChildNode, tempBoneHierarachy );
					break;
				}
			}
		}
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 자료들 저장하기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		SaveData( tempVertices, tempIndex, tempUV, tempBoneHierarachy, tempBoneOffsets, tempAnimations, Layer, Type );

		// 버텍스 정보들을 옮긴 뒤 노드들 제거
		for (int i = 0; i < m_pfbxScene->GetNodeCount( ); i++)
		{
			FbxNode* temp = m_pfbxScene->GetNode( i );
			m_pfbxScene->RemoveNode( temp );
		}
		return true;
	}
}

void FBXManager::LoadBoneHierarachy( FbxNode *pNode, std::vector<Bone> *pBoneHierarchy )
{
	for (int i = 0; i < pNode->GetChildCount( ); i++)
	{
		Bone tempBone;
	
		int parentIdx = 0;
		for (int j = 0; j < pBoneHierarchy->size( ); j++)
		{
			if (( *pBoneHierarchy )[j].boneName == pNode->GetName( ))
			{
				parentIdx = j;
				break;
			}
		}
		tempBone.boneName = pNode->GetChild(i)->GetName( );
		tempBone.parentBoneName = pNode->GetName( );
		tempBone.parentIdx = parentIdx;
		pBoneHierarchy->push_back( tempBone );
		LoadBoneHierarachy( pNode->GetChild( i ), pBoneHierarchy );
	}
}

void FBXManager::LoadInfluenceWeight( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices, std::vector<XMFLOAT4X4> *pBoneOffsets, std::vector<Bone> *pBoneHierarchy )
{
	// 본 계층구조, 오프셋변환, 가중치, 영향받는 정점 을 구해야함
	int numDeformers = pMesh->GetDeformerCount( FbxDeformer::eSkin);

	for (int i = 0; i < numDeformers; i++)
	{
		FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer( i, FbxDeformer::eSkin );
		if (!skin)
			continue;

		// 본의 정보를 얻어오는 행위
		int boneCount = skin->GetClusterCount( );		// 본의 개수		 54
		pBoneHierarchy->resize( boneCount );
		for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			// cluster == bone
			FbxCluster *cluster = skin->GetCluster( boneIndex );
			FbxNode *bone = cluster->GetLink( );		// 본과 연결되어있는 노드, 부모노드가 아님
			( *pBoneHierarchy)[boneIndex].boneName = bone->GetName( );
			( *pBoneHierarchy )[boneIndex].parentBoneName = bone->GetParent( )->GetName( );		// 부모노드의 이름
			

			FbxAMatrix LinkBoneMatrix;
			FbxAMatrix TransboneMatrix;
			FbxAMatrix ResultMatrix;

			cluster->GetTransformLinkMatrix( LinkBoneMatrix );
			cluster->GetTransformMatrix( TransboneMatrix );
			ResultMatrix = LinkBoneMatrix.Inverse( ) * TransboneMatrix;

			XMFLOAT4X4 tempOffsetMatl;
			tempOffsetMatl._11 = ResultMatrix.mData[0].mData[0]; tempOffsetMatl._12 = ResultMatrix.mData[0].mData[1];
			tempOffsetMatl._13 = ResultMatrix.mData[0].mData[2]; tempOffsetMatl._14 = ResultMatrix.mData[0].mData[3];
			tempOffsetMatl._21 = ResultMatrix.mData[1].mData[0]; tempOffsetMatl._22 = ResultMatrix.mData[1].mData[1];
			tempOffsetMatl._23 = ResultMatrix.mData[1].mData[2]; tempOffsetMatl._24 = ResultMatrix.mData[1].mData[3];
			tempOffsetMatl._31 = ResultMatrix.mData[2].mData[0]; tempOffsetMatl._32 = ResultMatrix.mData[2].mData[1];
			tempOffsetMatl._33 = ResultMatrix.mData[2].mData[2]; tempOffsetMatl._34 = ResultMatrix.mData[2].mData[3];
			tempOffsetMatl._41 = ResultMatrix.mData[3].mData[0]; tempOffsetMatl._42 = ResultMatrix.mData[3].mData[1];
			tempOffsetMatl._43 = ResultMatrix.mData[3].mData[2]; tempOffsetMatl._44 = ResultMatrix.mData[3].mData[3];

			pBoneOffsets->push_back( tempOffsetMatl );

			int *boneVertexIndices = cluster->GetControlPointIndices( );			// 해당 본에 영향을 받는 정점들
			double *boneVertexWeights = cluster->GetControlPointWeights( );		// 해당 본에 의한 정점의 가중치

			// 해당 본에 영향을 받는 모든 정점을 하나씩 가져옴	
			int numBoneVertexIndices = cluster->GetControlPointIndicesCount( );		
			for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
			{
				int tempBoneVertexIndex = boneVertexIndices[boneVertexIndex];			// 영향을 받는 정점의 인덱스
				float tempBoneWeight = (float)boneVertexWeights[boneVertexIndex];	// 영향을 받는 정점의 가중치 정도

				// 가중치 중 x가 0이면 첫번째 인덱스
				if (( *pVertices )[tempBoneVertexIndex].m_weights.x == 0)		
				{
					( *pVertices )[tempBoneVertexIndex].m_weights.x = tempBoneWeight;				
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.x = boneIndex;
				}
				// 가중치 중 x가 0이 아니고 y가 0이면 두번째 인덱스
				else if (( *pVertices )[tempBoneVertexIndex].m_weights.x != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.y == 0)
				{
					( *pVertices )[tempBoneVertexIndex].m_weights.y = tempBoneWeight;				
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.y = boneIndex;
				}
				// 가중치 중 x가 0이 아니고 y가 0이 아니고 z가 0이면 세번째 인덱스
				else if (( *pVertices )[tempBoneVertexIndex].m_weights.x != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.y != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.z == 0)
				{
					( *pVertices )[tempBoneVertexIndex].m_weights.z = tempBoneWeight;				
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.z = boneIndex;
				}
				// 모두 0이 아니면 4번째 인덱스, 가중치는 1에서 xyz 빼면 나머지 구할 수 있음
				else if (( *pVertices )[tempBoneVertexIndex].m_weights.x != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.y != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.z != 0)
				{
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.w = boneIndex;
				}
			}	
		}	// end of bonecount for

		for (int boneCount = 0; boneCount < pBoneHierarchy->size( ); boneCount++)
		{
			( *pBoneHierarchy )[boneCount].parentIdx = -1;
			for (int parentCount = 0; parentCount < pBoneHierarchy->size( ); parentCount++)
			{
				if (( *pBoneHierarchy )[boneCount].parentBoneName == ( *pBoneHierarchy )[parentCount].boneName)
				{
					( *pBoneHierarchy )[boneCount].parentIdx = parentCount;
					break;
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
	for (int animIndex = 0; animIndex < numAnimations; animIndex++)
	{
		FbxAnimStack *animStack = ( FbxAnimStack* )this->m_pfbxScene->GetSrcObject<FbxAnimStack>(animIndex);
		animName = animStack->GetName( );	// 이름

		LoadKeyframesByTime( animStack, pNode, &( animation.BoneAnimations ), BoneHierarchy );

		if (animation.BoneAnimations.size() != 0)
			tempAnimations.insert( { animIndex, animation } );
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

	FbxAnimLayer *pAnimLayer = (FbxAnimLayer*)pAnimStack->GetMember( );
	FbxAnimCurve *pTranslationCurve = pNode->LclTranslation.GetCurve( pAnimLayer );
	FbxAnimCurve *pRotationCurve = pNode->LclRotation.GetCurve( pAnimLayer );
	FbxAnimCurve *pScalingcurve = pNode->LclScaling.GetCurve( pAnimLayer );
	FbxAnimCurve *pNotNullCurve;

	long long mtxSize = 0;

	if (pTranslationCurve){
		mtxSize = pTranslationCurve->KeyGetCount( ); 
		pNotNullCurve = pTranslationCurve;
	}
	else if (pRotationCurve){
		mtxSize = pRotationCurve->KeyGetCount( );
		pNotNullCurve = pRotationCurve;
	}
	else if (pScalingcurve){
		mtxSize = pScalingcurve->KeyGetCount( );
		pNotNullCurve = pScalingcurve;
	}

	if (mtxSize != 0)
	{
		std::vector<Keyframe> _mtx( mtxSize );
		BoneAnimation tempBoneAnim;

		for (long long i = 0; i < mtxSize; i++)
		{
			FbxTime nTime = pNotNullCurve->KeyGetTime( i );

			if (pNode)
			{
				_mtx[i].TimePos = (float)nTime.GetSecondDouble( );		// 시간저장

				// 해당 시간의 변환 매트릭스 저장
				if (pScalingcurve)
				{
					FbxDouble3 scalingVector = pNode->EvaluateLocalScaling( nTime );
					_mtx[i].Scale.x = (float)scalingVector[0];
					_mtx[i].Scale.y = (float)scalingVector[2];
					_mtx[i].Scale.z = (float)scalingVector[1];
				}
				else
				{
					FbxDouble3 scalingVector = pNode->LclScaling.Get( );
					_mtx[i].Scale.x = (float)scalingVector[0];
					_mtx[i].Scale.y = (float)scalingVector[2];
					_mtx[i].Scale.z = (float)scalingVector[1];
				}

				if (pTranslationCurve)
				{
					FbxDouble3 translationVector = pNode->EvaluateLocalTranslation( nTime );
					_mtx[i].Translation.x = (float)translationVector[0];
					_mtx[i].Translation.y = (float)translationVector[2];
					_mtx[i].Translation.z = (float)translationVector[1];
				}
				else
				{
					FbxDouble3 translationVector = pNode->LclTranslation.Get( );
					_mtx[i].Translation.x = (float)translationVector[0];
					_mtx[i].Translation.y = (float)translationVector[2];
					_mtx[i].Translation.z = (float)translationVector[1];
				}

				if (pRotationCurve)
				{
					FbxDouble3 rotationVector = pNode->EvaluateLocalRotation( nTime );
					_mtx[i].RotationQuat.x = (float)rotationVector[0];
					_mtx[i].RotationQuat.y = (float)rotationVector[2];
					_mtx[i].RotationQuat.z = (float)rotationVector[1];
				}
				else
				{
					FbxDouble3 rotationVector = pNode->LclRotation.Get( );
					_mtx[i].RotationQuat.x = (float)rotationVector[0];
					_mtx[i].RotationQuat.y = (float)rotationVector[2];
					_mtx[i].RotationQuat.z = (float)rotationVector[1];
				}
			}
		}

		// 한 순간의 애니메이션 매트릭스
		tempBoneAnim.Keyframes = _mtx;
		tempBoneAnim.BoneName = pNode->GetName( );
		tempBoneAnim.PerentBoneName = pNode->GetParent( )->GetName( );
		tempBoneAnim.BoneIndex = BoneIdx;

		( *pvAnimations )[BoneIdx] = tempBoneAnim;
	}

	for (int i = 0; i < pNode->GetChildCount( ); i++)
	{
		LoadKeyframesByTime( pAnimStack, pNode->GetChild( i ), pvAnimations, BoneHierarchy );
	}
}

void FBXManager::LoadFBXMeshData( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertices, std::vector<UINT> *pIndices, std::vector<XMFLOAT2> *pUVs, std::vector<Bone> *pBoneHierarchy, std::vector<XMFLOAT4X4> *pBoneOffsets )
{
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 정점 좌표, 인덱스 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadVertexAndIndexInfomation( pMesh, pVertices, pIndices );

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ normal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadNormallnfomation( pMesh, pVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ tangent 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadTangentInfomation( pMesh, pVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ binormal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadBinormalInfomation( pMesh, pVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ UV 좌표 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadUVInformation( pMesh, pUVs );
	// 정점의 개수를 줄이려 했으나 UV 좌표 에러로 인해 막음
	// 최적화 단계에서 수정요망
	//			std::vector<XMFLOAT2> UVVectorByControlPoint( tempVertex.size( ) );
	//			for (int idx = 0; idx < tempIndex.size( ); idx++)
	//			{
	//				UVVectorByControlPoint[tempIndex[idx]] = tempUVVector[idx];
	//			}

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ  가중치와 뼈대 정보가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	LoadInfluenceWeight( pMesh, pVertices, pBoneOffsets, pBoneHierarchy );
}

void FBXManager::SaveData( std::vector<CFbxVertex> Vertex, std::vector<UINT> Index, std::vector<XMFLOAT2> UVVector, 
	std::vector<Bone> BoneHierarchy, std::vector<XMFLOAT4X4> BoneOffsets, std::map<int, AnimationClip> Animations, int iLayer, int iType )
{
	CFbxMesh tempMesh;

	tempMesh.m_pVertices.resize( Vertex.size( ) );
	tempMesh.m_pVertices = Vertex;

	for (int i = 0; i < Vertex.size( ); i++)
	{
		// UV좌표 저장
		tempMesh.m_pVertices[i].m_textureUV = UVVector[i];
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

	m_pMeshes.push_back( tempMesh );
}

void FBXManager::LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertex, std::vector<UINT> *pIndex )
{
	FbxVector4* pVertices = pMesh->GetControlPoints( );

	// 정점 좌표들을 저장할 공간
	int polygonCount = pMesh->GetPolygonCount( );		// 폴리곤의 개수
	//	int vertexCount = pMesh->GetControlPointsCount( );

	// 정점의 개수를 줄이려 했으나 UV 좌표 에러로 인해 막음
	// 최적화 단계에서 수정요망
	//	for (int polyCount = 0; polyCount < vertexCount; polyCount++)
	//	{
	//		tempVector.push_back( XMFLOAT3(pVertices[polyCount].mData[0], pVertices[polyCount].mData[2], pVertices[polyCount].mData[1]) );
	//	}

	int count = 0;

	for (int j = 0; j < polygonCount; j++)
	{
		int iNumVertices = pMesh->GetPolygonSize( j );	// 폴리곤을 구성하는 정점의 개수
		if (iNumVertices != 3)
		{
			pMesh->Destroy( );
			return;
		}

		for (int k = 0; k < iNumVertices; k++)
		{
			CFbxVertex tempVertex;
			int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

			tempVertex.m_position.x = (float)pVertices[iControlPointIndex].mData[0];
			tempVertex.m_position.y = (float)pVertices[iControlPointIndex].mData[2];
			tempVertex.m_position.z = (float)pVertices[iControlPointIndex].mData[1];

			pVertex->push_back( tempVertex );
			//			tempIndex.push_back( iControlPointIndex );
			pIndex->push_back( count++ );
		}
	}
}

void FBXManager::LoadTangentInfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices )
{
	int polygonVertexCount = pMesh->GetPolygonVertexCount( );

	for (int i = 0; i < polygonVertexCount; i++)
	{
		XMFLOAT4 tempTangent;
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
						tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[3] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int index = vertexTangent->GetIndexArray( ).GetAt( i );
						tempTangent.x = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[0] );
						tempTangent.y = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[2] );
						tempTangent.z = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[1] );
						tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[3] );
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
						tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( i ).mData[3] );
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int inVertexCounter = pMesh->GetPolygonVertexCount( );
						int index = vertexTangent->GetIndexArray( ).GetAt( inVertexCounter );
						tempTangent.x = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[0] );
						tempTangent.y = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[2] );
						tempTangent.z = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[1] );
						tempTangent.w = static_cast<float>( vertexTangent->GetDirectArray( ).GetAt( index ).mData[3] );
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
	for (int lUVSetIndex = 0; lUVSetIndex < NameListCount; lUVSetIndex++)
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

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray( ).GetAt( lPolyIndexCounter ) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray( ).GetAt( lUVIndex );

						XMFLOAT2 temp( lUVValue.mData[0], lUVValue.mData[1] );
						pVertices->push_back( temp );

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}