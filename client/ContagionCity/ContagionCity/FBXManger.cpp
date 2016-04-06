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

// FBX ������ Scene�� ��忡 �߰��ϴ� �Լ�
bool FBXManager::LoadFBX( const char* pstrFileName, int Layer, int Type )
{
	// ���� �̸��� �ε�
	bool bResult = m_pfbxImporter->Initialize( pstrFileName, -1, m_pfbxManager->GetIOSettings( ) );
	if (!bResult)
		return false;

	// Scene�� �߰�, Scene�� ��Ʈ����� ������ ���Ե�
	bResult = m_pfbxImporter->Import( m_pfbxScene );
	if (!bResult)
		return false;

	FbxNode *pfbxRootNode = m_pfbxScene->GetRootNode( );
	// ���� Scene�� ����Ǿ��ִ� �޽õ��� ������ ����

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
					// �޽õ����� �ε�
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
		// ���ؽ� �������� �ű� �� ���� ����
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
	// �� ��������, �����º�ȯ, ����ġ, ����޴� ���� �� ���ؾ���
	std::vector<XMFLOAT4X4> tempBoneOffsets;		// i�� ���밡 �θ�� ���� �����º�ȯ
	std::vector<int> tempBoneHierarachy;			// ������ ��ü ��������

	int numDeformers = pMesh->GetDeformerCount( FbxDeformer::eSkin);

	for (int i = 0; i < numDeformers; i++)
	{
		FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer( i, FbxDeformer::eSkin );
		if (!skin)
			continue;

		// ���� ������ ������ ����
		int boneCount = skin->GetClusterCount( );		// ���� ����		 54
		for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			// cluster == bone
			FbxCluster *cluster = skin->GetCluster( boneIndex );
			FbxNode *bone = cluster->GetLink( );		// ���� ����Ǿ��ִ� ���
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

			int *boneVertexIndices = cluster->GetControlPointIndices( );			// �ش� ���� ������ �޴� ������
			double *boneVertexWeights = cluster->GetControlPointWeights( );		// �ش� ���� ���� ������ ����ġ

			// �ش� ���� ������ �޴� ��� ������ �ϳ��� ������	
			int numBoneVertexIndices = cluster->GetControlPointIndicesCount( );		
			for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
			{
				int tempBoneVertexIndex = boneVertexIndices[boneVertexIndex];			// ������ �޴� ������ �ε���
				float tempBoneWeight = (float)boneVertexWeights[boneVertexIndex];	// ������ �޴� ������ ����ġ ����

				// ����ġ �� x�� 0�̸� ù��° �ε���
				if (( *pVertices )[tempBoneVertexIndex].m_weights.x == 0)		
				{
					( *pVertices )[tempBoneVertexIndex].m_weights.x = tempBoneWeight;				
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.x = boneIndex;
				}
				// ����ġ �� x�� 0�� �ƴϰ� y�� 0�̸� �ι�° �ε���
				else if (( *pVertices )[tempBoneVertexIndex].m_weights.x != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.y == 0)
				{
					( *pVertices )[tempBoneVertexIndex].m_weights.y = tempBoneWeight;				
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.y = boneIndex;
				}
				// ����ġ �� x�� 0�� �ƴϰ� y�� 0�� �ƴϰ� z�� 0�̸� ����° �ε���
				else if (( *pVertices )[tempBoneVertexIndex].m_weights.x != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.y != 0 && ( *pVertices )[tempBoneVertexIndex].m_weights.z == 0)
				{
					( *pVertices )[tempBoneVertexIndex].m_weights.z = tempBoneWeight;				
					( *pVertices )[tempBoneVertexIndex].m_boneIndices.z = boneIndex;
				}
				// ��� 0�� �ƴϸ� 4��° �ε���, ����ġ�� 1���� xyz ���� ������ ���� �� ����
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

	// �ִϸ��̼�Ŭ���� ã�� ����
	int numAnimations = m_pfbxScene->GetSrcObjectCount( FbxAnimStack::ClassId );
	for (int animIndex = 0; animIndex < numAnimations; animIndex++)
	{
		FbxAnimStack *animStack = ( FbxAnimStack* )this->m_pfbxScene->GetSrcObject( FbxAnimStack::ClassId, animIndex );
		animName = animStack->GetName( );	// �̸�

		// ���ִϸ��̼��� ã�� ����
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

			// Ű������ ã�� ����
			if (scalingCurve)
			{
				tempBone.Keyframes.resize( scalingCurve->KeyGetCount( ) );

				int numKeys = scalingCurve->KeyGetCount( );
				for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				{
					FbxTime frameTime = scalingCurve->KeyGetTime( keyIndex );
					FbxDouble3 scalingVector = pNode->EvaluateLocalScaling( frameTime );
					// �����ϰ��� ����
					// ��ǥ�� ��ȯ�� ���� y�� z�� �ٲ�
					XMFLOAT3 vScaling( (float)scalingVector[0], (float)scalingVector[2], (float)scalingVector[1] );
					tempBone.Keyframes[keyIndex].Scale = vScaling;

					// �ð� ����
					tempBone.Keyframes[keyIndex].TimePos = (float)frameTime.GetSecondDouble( );
				}
			}	// end of scaling if
			if (rotationCurve)
			{
				// Ű������ ã�� ����
				tempBone.Keyframes.resize( rotationCurve->KeyGetCount( ) );

				int numKeys = rotationCurve->KeyGetCount( );
				for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				{
					FbxTime frameTime = rotationCurve->KeyGetTime( keyIndex );
					FbxDouble3 rotationVector = pNode->EvaluateLocalRotation( frameTime );
					// ȸ������ ����
					// ��ǥ�� ��ȯ�� ���� y�� z�� �ٲ�
					XMFLOAT3 vRotation( (float)rotationVector[0], (float)rotationVector[2], (float)rotationVector[1] );
					tempBone.Keyframes[keyIndex].RotationQuat = XMFLOAT4( vRotation.x, vRotation.y, vRotation.z, 1 );	// XMFLOAT3�� ���ʹϾ����� ��ȯ�ؾ���

					// �ð� ����
					tempBone.Keyframes[keyIndex].TimePos = (float)frameTime.GetSecondDouble( );
				}
			}	// end of rotation if
			if (translationCurve)
			{
				// Ű������ ã�� ����
				tempBone.Keyframes.resize( translationCurve->KeyGetCount( ) );

				int numKeys = translationCurve->KeyGetCount( );
				for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				{
					FbxTime frameTime = translationCurve->KeyGetTime( keyIndex );
					FbxDouble3 translationVector = pNode->EvaluateLocalTranslation( frameTime );
					// Ʈ�������̼ǰ��� ����
					XMFLOAT3 vTranslation( (float)translationVector[0], (float)translationVector[2], (float)translationVector[1] );
					tempBone.Keyframes[keyIndex].Translation = vTranslation;

					// �ð� ����
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
	//�ѤѤѤѤѤѤѤѤѤѤ� ���� ��ǥ, �ε��� ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	vector<XMFLOAT3> tempVertex;
	vector<UINT> tempIndex;
	LoadVertexAndIndexInfomation( pMesh, &tempVertex, &tempIndex );

	// �ѤѤѤѤѤѤѤѤѤѤ� normal ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	std::vector<XMFLOAT3> tempNormal;
	LoadNormallnfomation( pMesh, &tempNormal );

	//�ѤѤѤѤѤѤѤѤѤѤ� tangent ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	std::vector<XMFLOAT4> tempTangent;
	LoadTangentInfomation( pMesh, &tempTangent );

	//�ѤѤѤѤѤѤѤѤѤѤ� binormal ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	std::vector<XMFLOAT3> tempBinormal;
	LoadBinormalInfomation( pMesh, &tempBinormal );

	//�ѤѤѤѤѤѤѤѤѤѤ� UV ��ǥ �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	std::vector<XMFLOAT2> tempUVVector;
	LoadUVInformation( pMesh, &tempUVVector );
	// ������ ������ ���̷� ������ UV ��ǥ ������ ���� ����
	// ����ȭ �ܰ迡�� �������
	//			std::vector<XMFLOAT2> UVVectorByControlPoint( tempVertex.size( ) );
	//			for (int idx = 0; idx < tempIndex.size( ); idx++)
	//			{
	//				UVVectorByControlPoint[tempIndex[idx]] = tempUVVector[idx];
	//			}

	// �ѤѤѤѤѤѤѤѤѤѤ�  ����ġ�� ���� ������������ �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	SkinnedData tempSkinnedData;
	std::vector<int> tempBoneHierarachy;
	std::vector<XMFLOAT4X4> tempBoneOffsets;
	std::vector<CFbxVertex> tempVertices( tempVertex.size( ) );

	LoadBoneHierarachy( pMesh, &tempVertices );

	//�ѤѤѤѤѤѤѤѤѤѤ� �ڷ�� �����ϱ� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	SaveData( tempVertex, tempIndex, tempUVVector, tempVertices, Layer, Type );
}

void FBXManager::LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<XMFLOAT3> *pVertex, std::vector<UINT> *pIndex )
{
	FbxVector4* pVertices = pMesh->GetControlPoints( );

	// ���� ��ǥ���� ������ ����
	int polygonCount = pMesh->GetPolygonCount( );		// �������� ����
	//	int vertexCount = pMesh->GetControlPointsCount( );

	// ������ ������ ���̷� ������ UV ��ǥ ������ ���� ����
	// ����ȭ �ܰ迡�� �������
	//	for (int polyCount = 0; polyCount < vertexCount; polyCount++)
	//	{
	//		tempVector.push_back( XMFLOAT3(pVertices[polyCount].mData[0], pVertices[polyCount].mData[2], pVertices[polyCount].mData[1]) );
	//	}

	int count = 0;

	for (int j = 0; j < polygonCount; j++)
	{
		int iNumVertices = pMesh->GetPolygonSize( j );	// �������� �����ϴ� ������ ����
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
		// ���� ����
		tempMesh.m_pVertices[i].m_position = Vertex[i];
		// UV��ǥ ����
		tempMesh.m_pVertices[i].m_textureUV = UVVector[i];
		// ����ġ ����
		tempMesh.m_pVertices[i].m_weights = weights[i].m_weights;
		// �����ִ� ���� ����
		tempMesh.m_pVertices[i].m_boneIndices = weights[i].m_boneIndices;
	}

	// vertex ����
	tempMesh.m_nVertexCount = Vertex.size( );
	// �ε������� ����
	tempMesh.m_pvIndices = Index;
	// index ����
	tempMesh.m_nIndexCount = Index.size( );
	// ���̾�
	tempMesh.m_iLayer = iLayer;
	// Ÿ��
	tempMesh.m_iType = iType;


	m_pMeshes.push_back( tempMesh );
}

bool FBXManager::LoadM3D( const std::string& fileName, int layer, int type, CM3dMesh& pOutMesh)
{
	//	#include <fstream> �ʿ�
	std::ifstream fin( fileName );

	UINT nMaterials = 0;
	UINT nVertices = 0;
	UINT nTriangles = 0;
	UINT nBones = 0;
	UINT nAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore;	// ���� ���ۺ� ����
		fin >> ignore >> nMaterials;
		fin >> ignore >> nVertices;
		fin >> ignore >> nTriangles;
		fin >> ignore >> nBones;
		fin >> ignore >> nAnimationClips;

		// ��ȯ�� ������
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

	fin >> ignore;	// �ִϸ��̼� ���ۺ� ����
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

	fin >> ignore; // ù ���� ���� ���ۺ� ����
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

	fin >> ignore; // ���� �κ� ���ۺ� ����
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

		//wstring ���� ��ȯ
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