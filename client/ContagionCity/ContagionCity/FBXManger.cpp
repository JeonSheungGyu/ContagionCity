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
	// 현재 Scene에 저장되어있는 메시들의 개수를 저장

	if (pfbxRootNode)
	{
		int count = m_pfbxScene->GetRootNode( )->GetChildCount( );

		for (int i = 0; i < m_pfbxScene->GetRootNode( )->GetChildCount( ); i++)
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
					LoadFBXMeshData( pMesh, Layer, Type );
		//			tempSkinnedData.Set( tempBoneHierarachy, tempBoneOffsets, tempAnimations );
					std::map<std::string, AnimationClip> tempAnimations;
					tempAnimations = LoadBoneInfomation( pfbxChildNode );
					break;
				}
				case FbxNodeAttribute::eSkeleton:
				{
					std::map<std::string, AnimationClip> tempAnimations;
					tempAnimations = LoadBoneInfomation( pfbxChildNode );
					break;
				}
			}
		}
		// 버텍스 정보들을 옮긴 뒤 노드들 제거
		for (int i = 0; i < m_pfbxScene->GetNodeCount( ); i++)
		{
			FbxNode* temp = m_pfbxScene->GetNode( i );
			m_pfbxScene->RemoveNode( temp );
		}
		return true;
	}
}

void FBXManager::LoadBoneHierarachy( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices )
{
	// 본 계층구조, 오프셋변환, 가중치, 영향받는 정점 을 구해야함
	std::vector<XMFLOAT4X4> tempBoneOffsets;		// i번 뼈대가 부모로 가는 오프셋변환
	std::vector<int> tempBoneHierarachy;			// 뼈대의 전체 계층구조

	int numDeformers = pMesh->GetDeformerCount( FbxDeformer::eSkin);

	for (int i = 0; i < numDeformers; i++)
	{
		FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer( i, FbxDeformer::eSkin );
		if (!skin)
			continue;

		// 본의 정보를 얻어오는 행위
		int boneCount = skin->GetClusterCount( );		// 본의 개수		 54
		for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			// cluster == bone
			FbxCluster *cluster = skin->GetCluster( boneIndex );
			FbxNode *bone = cluster->GetLink( );		// 본과 연결되어있는 노드
			std::string pName = bone->GetName( );

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

			tempBoneOffsets.push_back( tempOffsetMatl );

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
	}	// end of deformer for
}

std::map<std::string, AnimationClip> FBXManager::LoadBoneInfomation( FbxNode* pNode )
{
	bool isAnimated = false;
	std::map<std::string, AnimationClip> tempAnimations;
	AnimationClip animation;
	std::string animName;

	// 애니메이션클립을 찾는 행위
	int numAnimations = m_pfbxScene->GetSrcObjectCount( FbxAnimStack::ClassId );
	for (int animIndex = 0; animIndex < numAnimations; animIndex++)
	{
		FbxAnimStack *animStack = ( FbxAnimStack* )this->m_pfbxScene->GetSrcObject( FbxAnimStack::ClassId, animIndex );
		animName = animStack->GetName( );	// 이름

		// 본애니메이션을 찾는 행위
		int numLayers = animStack->GetMemberCount( );
		for (int layerIndex = 0; layerIndex < numLayers; layerIndex++)
		{
			FbxAnimLayer *animLayer = (FbxAnimLayer*)animStack->GetMember( layerIndex );
			BoneAnimation tempBone;
			tempBone.BoneName = animLayer->GetName( );
			tempBone.BoneIndex = layerIndex;
			
			FbxAnimCurve *translationCurve = pNode->LclTranslation.GetCurve( animLayer );
			FbxAnimCurve *rotationCurve = pNode->LclRotation.GetCurve( animLayer );
			FbxAnimCurve *scalingCurve = pNode->LclScaling.GetCurve( animLayer );

			// 키프레임 찾는 행위
			if (scalingCurve)
			{
				tempBone.Keyframes.resize( scalingCurve->KeyGetCount( ) );

				int numKeys = scalingCurve->KeyGetCount( );
				for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				{
					FbxTime frameTime = scalingCurve->KeyGetTime( keyIndex );
					FbxDouble3 scalingVector = pNode->EvaluateLocalScaling( frameTime );
					// 스케일값에 저장
					// 좌표계 변환을 위해 y와 z를 바꿈
					XMFLOAT3 vScaling( (float)scalingVector[0], (float)scalingVector[2], (float)scalingVector[1] );
					tempBone.Keyframes[keyIndex].Scale = vScaling;

					// 시간 저장
					tempBone.Keyframes[keyIndex].TimePos = (float)frameTime.GetSecondDouble( );
				}
			}	// end of scaling if
			if (rotationCurve)
			{
				// 키프레임 찾는 행위
				tempBone.Keyframes.resize( rotationCurve->KeyGetCount( ) );

				int numKeys = rotationCurve->KeyGetCount( );
				for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				{
					FbxTime frameTime = rotationCurve->KeyGetTime( keyIndex );
					FbxDouble3 rotationVector = pNode->EvaluateLocalRotation( frameTime );
					// 회전값에 저장
					// 좌표계 변환을 위해 y와 z를 바꿈
					XMFLOAT3 vRotation( (float)rotationVector[0], (float)rotationVector[2], (float)rotationVector[1] );
					tempBone.Keyframes[keyIndex].RotationQuat = XMFLOAT4( vRotation.x, vRotation.y, vRotation.z, 1 );	// XMFLOAT3을 쿼터니언으로 변환해야함

					// 시간 저장
					tempBone.Keyframes[keyIndex].TimePos = (float)frameTime.GetSecondDouble( );
				}
			}	// end of rotation if
			if (translationCurve)
			{
				// 키프레임 찾는 행위
				tempBone.Keyframes.resize( translationCurve->KeyGetCount( ) );

				int numKeys = translationCurve->KeyGetCount( );
				for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				{
					FbxTime frameTime = translationCurve->KeyGetTime( keyIndex );
					FbxDouble3 translationVector = pNode->EvaluateLocalTranslation( frameTime );
					// 트렌스레이션값에 저장
					XMFLOAT3 vTranslation( (float)translationVector[0], (float)translationVector[2], (float)translationVector[1] );
					tempBone.Keyframes[keyIndex].Translation = vTranslation;

					// 시간 저장
					tempBone.Keyframes[keyIndex].TimePos = (float)frameTime.GetSecondDouble( );
				}
			}	// end of translation if
			animation.BoneAnimations.push_back( tempBone );
		}	// end of layer for
		tempAnimations.insert( { animName, animation } );
	}	// end of animation for
	return tempAnimations;
}

void FBXManager::LoadFBXMeshData( FbxMesh* pMesh, int Layer, int Type )
{
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 정점 좌표, 인덱스 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	vector<XMFLOAT3> tempVertex;
	vector<UINT> tempIndex;
	LoadVertexAndIndexInfomation( pMesh, &tempVertex, &tempIndex );

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ normal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	std::vector<XMFLOAT3> tempNormal;
	LoadNormallnfomation( pMesh, &tempNormal );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ tangent 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	std::vector<XMFLOAT4> tempTangent;
	LoadTangentInfomation( pMesh, &tempTangent );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ binormal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	std::vector<XMFLOAT3> tempBinormal;
	LoadBinormalInfomation( pMesh, &tempBinormal );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ UV 좌표 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	std::vector<XMFLOAT2> tempUVVector;
	LoadUVInformation( pMesh, &tempUVVector );
	// 정점의 개수를 줄이려 했으나 UV 좌표 에러로 인해 막음
	// 최적화 단계에서 수정요망
	//			std::vector<XMFLOAT2> UVVectorByControlPoint( tempVertex.size( ) );
	//			for (int idx = 0; idx < tempIndex.size( ); idx++)
	//			{
	//				UVVectorByControlPoint[tempIndex[idx]] = tempUVVector[idx];
	//			}

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ  가중치와 뼈대 정보가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	SkinnedData tempSkinnedData;
	std::vector<int> tempBoneHierarachy;
	std::vector<XMFLOAT4X4> tempBoneOffsets;
	std::vector<CFbxVertex> tempVertices( tempVertex.size( ) );

	LoadBoneHierarachy( pMesh, &tempVertices );

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 자료들 저장하기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	SaveData( tempVertex, tempIndex, tempUVVector, tempVertices, Layer, Type );
}

void FBXManager::LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<XMFLOAT3> *pVertex, std::vector<UINT> *pIndex )
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
			int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

			XMFLOAT3 temp;
			temp.x = (float)pVertices[iControlPointIndex].mData[0];
			temp.y = (float)pVertices[iControlPointIndex].mData[2];
			temp.z = (float)pVertices[iControlPointIndex].mData[1];

			pVertex->push_back( temp );
			//			tempIndex.push_back( iControlPointIndex );
			pIndex->push_back( count++ );
		}
	}
}
void FBXManager::LoadTangentInfomation( FbxMesh *pMesh, std::vector<XMFLOAT4> *pTangents )
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
		pTangents->push_back( tempTangent );
	}
}

void FBXManager::LoadBinormalInfomation( FbxMesh *pMesh, std::vector<XMFLOAT3> *pBinormals )
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
		pBinormals->push_back( tempBinomal );
	}
}

void FBXManager::LoadNormallnfomation( FbxMesh *pMesh, std::vector<XMFLOAT3> *pNormals )
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
		pNormals->push_back( tempNormal );
	}
}

void FBXManager::LoadUVInformation( FbxMesh* pMesh, std::vector<XMFLOAT2> *pVector )
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
					pVector->push_back( temp );
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
						pVector->push_back( temp );

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}

void FBXManager::SaveData( std::vector<XMFLOAT3> Vertex, std::vector<UINT> Index, std::vector<XMFLOAT2> UVVector, std::vector<CFbxVertex> weights, int iLayer, int iType )
{
	CFbxMesh tempMesh;

	tempMesh.m_pVertices.resize( Vertex.size( ) );

	for (int i = 0; i < Vertex.size( ); i++)
	{
		// 정점 저장
		tempMesh.m_pVertices[i].m_position = Vertex[i];
		// UV좌표 저장
		tempMesh.m_pVertices[i].m_textureUV = UVVector[i];
		// 가중치 저장
		tempMesh.m_pVertices[i].m_weights = weights[i].m_weights;
		// 영향주는 뼈대 저장
		tempMesh.m_pVertices[i].m_boneIndices = weights[i].m_boneIndices;
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


	m_pMeshes.push_back( tempMesh );
}

bool FBXManager::LoadM3D( const std::string& fileName, int layer, int type, CM3dMesh& pOutMesh)
{
	//	#include <fstream> 필요
	std::ifstream fin( fileName );

	UINT nMaterials = 0;
	UINT nVertices = 0;
	UINT nTriangles = 0;
	UINT nBones = 0;
	UINT nAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore;	// 파일 시작부 문구
		fin >> ignore >> nMaterials;
		fin >> ignore >> nVertices;
		fin >> ignore >> nTriangles;
		fin >> ignore >> nBones;
		fin >> ignore >> nAnimationClips;

		// 반환할 정보들
		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::map<std::string, AnimationClip> animations;
		SkinnedData skinInfo;

		std::vector<M3dMaterial> mats;
		std::vector<Subset> subsets;
		std::vector<M3dVertex> vertices;
		std::vector<USHORT> indices;

		ReadMaterial( fin, nMaterials, mats );
		ReadSubsetTable( fin, nMaterials, subsets );
		ReadSkinnedVertices( fin, nVertices, vertices );
		ReadTriangles( fin, nTriangles, indices );
		ReadBoneOffsets( fin, nBones, boneOffsets );
		ReadBoneHierarchy( fin, nBones, boneIndexToParentIndex );
		ReadAnimationClips( fin, nBones, nAnimationClips, animations );

		skinInfo.Set( boneIndexToParentIndex, boneOffsets, animations );

		// return data
		pOutMesh.m_iLayer = layer;
		pOutMesh.m_iType = type;
		pOutMesh.m_nIndexCount = indices.size( );
		pOutMesh.m_nVertexCount = vertices.size( );
		pOutMesh.m_pVertexes = vertices;
		pOutMesh.m_pvIndices = indices;
		pOutMesh.m_mats = mats;
		pOutMesh.m_subsets = subsets;
//		pOutMesh.m_skinnedData = skinInfo;

		return true;
	}
	return false;
}
void FBXManager::ReadAnimationClips( std::ifstream& fin, UINT nBones, UINT nAnimationClips, std::map<std::string, AnimationClip>& animations )
{
	std::string ignore;

	fin >> ignore;	// 애니메이션 시작부 문구
	for (UINT clipIndex = 0; clipIndex < nAnimationClips; clipIndex++)
	{
		std::string clipName;
		fin >> ignore >> clipName;
		fin >> ignore;

		AnimationClip clip;
		clip.BoneAnimations.resize( nBones );

		for (UINT boneIndex = 0; boneIndex < nBones; boneIndex++)
		{
			ReadBoneKeyFrames( fin, nBones, clip.BoneAnimations[boneIndex] );
		}

		fin >> ignore;

		animations[clipName] = clip;
	}
}

void FBXManager::ReadBoneKeyFrames( std::ifstream& fin, UINT nBones, BoneAnimation& boneAnimation )
{
	std::string ignore;
	UINT nKeyFrames = 0;
	fin >> ignore >> ignore >> nKeyFrames;
	fin >> ignore;
	
	boneAnimation.Keyframes.resize( nKeyFrames );
	for (UINT i = 0; i < nKeyFrames; i++)
	{
		float t = 0.0f;
		XMFLOAT3 p( 0.0f, 0.0f, 0.0f );
		XMFLOAT3 s( 0.0f, 0.0f, 0.0f );
		XMFLOAT4 q( 0.0f, 0.0f, 0.0f, 1.0f );

		fin >> ignore >> t;
		fin >> ignore >> p.x >> p.y >> p.z;
		fin >> ignore >> s.x >> s.y >> s.z;
		fin >> ignore >> q.x >> q.y >> q.z >> q.w;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = s;
		boneAnimation.Keyframes[i].Scale = s;
		boneAnimation.Keyframes[i].RotationQuat = q;
	}
	fin >> ignore;
}

void FBXManager::ReadBoneOffsets( std::ifstream& fin, UINT nBones, std::vector<XMFLOAT4X4>& boneOffsets )
{
	std::string ignore;
	boneOffsets.resize( nBones );
	for (UINT i = 0; i < nBones; i++)
	{
		fin >> ignore >>
			boneOffsets[i]( 0, 0 ) >> boneOffsets[i]( 0, 1 ) >>
			boneOffsets[i]( 0, 2 ) >> boneOffsets[i]( 0, 3 ) >>
			boneOffsets[i]( 1, 0 ) >> boneOffsets[i]( 1, 1 ) >>
			boneOffsets[i]( 1, 2 ) >> boneOffsets[i]( 1, 3 ) >>
			boneOffsets[i]( 2, 0 ) >> boneOffsets[i]( 2, 1 ) >>
			boneOffsets[i]( 2, 2 ) >> boneOffsets[i]( 2, 3 ) >>
			boneOffsets[i]( 3, 0 ) >> boneOffsets[i]( 3, 1 ) >>
			boneOffsets[i]( 3, 2 ) >> boneOffsets[i]( 3, 3 );
	}
}

void FBXManager::ReadSkinnedVertices( std::ifstream& fin, UINT nVertices, std::vector<M3dVertex>& vertices )
{
	std::string ignore;
	vertices.resize( nVertices );

	fin >> ignore; // 첫 줄의 파일 시작부 문구
	int boneIndices[4];
	float weights[4];
	for (UINT i = 0; i < nVertices; i++)
	{
		fin >> ignore >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> vertices[i].TangentU.w;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].Tex.x >> vertices[i].Tex.y;
		fin >> ignore >> weights[0] >> weights[1] >> weights[2] >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].weights.x = weights[0];
		vertices[i].weights.y = weights[1];
		vertices[i].weights.z = weights[2];

		vertices[i].boneIndices[0] = (BYTE)boneIndices[0];
		vertices[i].boneIndices[1] = (BYTE)boneIndices[1];
		vertices[i].boneIndices[2] = (BYTE)boneIndices[2];
		vertices[i].boneIndices[3] = (BYTE)boneIndices[3];
	}
}

void FBXManager::ReadMaterial( std::ifstream& fin, UINT nMaterials, std::vector<M3dMaterial>& mats )
{
	std::string ignore;
	mats.resize( nMaterials );

	std::string diffuseMapName;
	std::string normalMapName;

	fin >> ignore; // 재질 부분 시작부 문구
	for (UINT i = 0; i < nMaterials; i++)
	{
		fin >> ignore >> mats[i].Mat.Ambient.x >> mats[i].Mat.Ambient.y >> mats[i].Mat.Ambient.z;
		fin >> ignore >> mats[i].Mat.Diffuse.x >> mats[i].Mat.Diffuse.y >> mats[i].Mat.Diffuse.z;
		fin >> ignore >> mats[i].Mat.Specular.x >> mats[i].Mat.Specular.y >> mats[i].Mat.Specular.z;
		fin >> ignore >> mats[i].Mat.Reflect.x >> mats[i].Mat.Reflect.y >> mats[i].Mat.Reflect.z;

		fin >> ignore >> mats[i].AlphaClip;
		fin >> ignore >> mats[i].EffectTypeName;
		fin >> ignore >> diffuseMapName;
		fin >> ignore >> normalMapName;

		mats[i].DiffuseMapName.resize( diffuseMapName.size( ), ' ' );
		mats[i].NormalMapName.resize( normalMapName.size( ), ' ' );

		//wstring 으로 변환
		std::copy( diffuseMapName.begin( ), diffuseMapName.end( ), mats[i].DiffuseMapName.begin( ) );
		std::copy( normalMapName.begin( ), normalMapName.end( ), mats[i].NormalMapName.begin( ) );
	}
}

void FBXManager::ReadSubsetTable( std::ifstream& fin, UINT nSubsets, std::vector<Subset>& subsets )
{
	std::string ignore;
	subsets.resize( nSubsets );

	fin >> ignore;
	for (UINT i = 0; i < nSubsets; i++)
	{
		fin >> ignore >> subsets[i].Id;
		fin >> ignore >> subsets[i].VertexStart;
		fin >> ignore >> subsets[i].VertexCount;
		fin >> ignore >> subsets[i].FaceStart;
		fin >> ignore >> subsets[i].FaceCount;
	}
}

void FBXManager::ReadTriangles( std::ifstream& fin, UINT nTriangles, std::vector<USHORT>& indices )
{
	std::string ignore;
	indices.resize( nTriangles * 3 );

	fin >> ignore;
	for (UINT i = 0; i < nTriangles; i++)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
}

void FBXManager::ReadBoneHierarchy( std::ifstream& fin, UINT nBones, std::vector<int>& boneIndexToParentIndex )
{
	std::string ignore;
	boneIndexToParentIndex.resize( nBones );

	fin >> ignore;
	for (UINT i = 0; i < nBones; i++)
	{
		fin >> ignore >> boneIndexToParentIndex[i];
	}
}