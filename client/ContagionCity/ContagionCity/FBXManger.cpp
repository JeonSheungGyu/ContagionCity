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

// FBX ������ Scene�� ��忡 �߰��ϴ� �Լ�, �ؽ�ó�� �������ڷ� ����
bool FBXManager::LoadFBX( const char* pstrFileName, int Layer, int Type, int textureCount, ...)
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

		// �ؽ�ó �̸� ����
		va_list ap;
		va_start( ap, textureCount );
		for (int i = 0; i < textureCount; i++)
		{
			tempTextures.push_back( va_arg( ap, TCHAR* ) );
		}
		va_end( ap);

		// i�� 0���� �����ϸ� �ݵ�� mesh�� ���� ���۵����� i�� ChildCount ���� ���ҽ�Ű�鼭 ���� �ݵ�� skeleton�� ���� ���۵ȴ�
		for (int i = ChildCount - 1; i >= 0; i--)
	//	for (int i = 0; i < ChildCount; i++)
		{
			// ��Ű�� ������ �ε�
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
					// �Ű������� std;;Vector<Bone> �ѱ�� �� ���� ���� ã�� �� �˻��ؼ� �ε��� �����Ұ�
					LoadFBXMeshData( pMesh, &tempVertices, &tempIndex, &tempUV, &tempBoneOffsets, tempBoneHierarachy );
					break;
				}
				case FbxNodeAttribute::eSkeleton:
				{
					LoadBoneHierarachy( pfbxChildNode, &tempBoneHierarachy );
					tempAnimations = LoadBoneInfomation( pfbxChildNode, tempBoneHierarachy );
					break;
				}
			}
		}
		//�ѤѤѤѤѤѤѤѤѤѤ� �ڷ�� �����ϱ� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
		SaveData( tempVertices, tempIndex, tempUV, tempBoneHierarachy, tempBoneOffsets, tempAnimations, Layer, Type, tempTextures );

		// ���ؽ� �������� �ű� �� ���� ����
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
	// �� ��������, �����º�ȯ, ����ġ, ����޴� ���� �� ���ؾ���
	int numDeformers = pMesh->GetDeformerCount( FbxDeformer::eSkin);

	for (int i = 0; i < numDeformers; i++)
	{
		FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer( i, FbxDeformer::eSkin );
		if (!skin)
			continue;

		pBoneOffsets->resize( BoneHierarchy.size( ) );
		// ���� ������ ������ ����
		int boneCount = skin->GetClusterCount( );		// ���� ����		 54
		for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			// cluster == bone
			FbxCluster *cluster = skin->GetCluster( boneIndex );
			FbxNode *bone = cluster->GetLink( );		// ���� ����Ǿ��ִ� ���, �θ��尡 �ƴ�
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
		
			double *boneVertexWeights = cluster->GetControlPointWeights( );		// �ش� ���� ���� ������ ����ġ
			int *boneVertexIndices = cluster->GetControlPointIndices( );					// �ش� ���� ������ �޴� ������
			// �ش� ���� ������ �޴� ��� ������ �ϳ��� ������	
			int numBoneVertexIndices = cluster->GetControlPointIndicesCount( );		

			//std::vector<int> boneIndices;
			//for (int i = 0; i < numBoneVertexIndices; i++)
			//{
			//	boneIndices.push_back( boneVertexIndices[i]);
			//}

			for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
			{
				float tempBoneWeight = (float)boneVertexWeights[boneVertexIndex];	// ������ �޴� ������ ����ġ ����

				// ����ġ�� 0�̸� �����ɷ� �ѱ�
				if (!tempBoneWeight)
					continue;

				int tempBoneVertexIndex = boneVertexIndices[boneVertexIndex];			// ������ �޴� ������ �ε���

				// ����ġ �� x�� 0�̸� ù��° �ε���
				if (pVertices->data()[tempBoneVertexIndex].m_weights.x == 0)		
				{
					pVertices->data( )[tempBoneVertexIndex].m_weights.x = tempBoneWeight;
					pVertices->data( )[tempBoneVertexIndex].m_boneIndices.x = boneHierIdx;
				}
				// ����ġ �� x�� 0�� �ƴϰ� y�� 0�̸� �ι�° �ε���
				else if (pVertices->data( )[tempBoneVertexIndex].m_weights.x != 0 && pVertices->data( )[tempBoneVertexIndex].m_weights.y == 0)
				{
					pVertices->data( )[tempBoneVertexIndex].m_weights.y = tempBoneWeight;
					pVertices->data( )[tempBoneVertexIndex].m_boneIndices.y = boneHierIdx;
				}
				// ����ġ �� x�� 0�� �ƴϰ� y�� 0�� �ƴϰ� z�� 0�̸� ����° �ε���
				else if (pVertices->data( )[tempBoneVertexIndex].m_weights.x != 0 && pVertices->data( )[tempBoneVertexIndex].m_weights.y != 0 && pVertices->data( )[tempBoneVertexIndex].m_weights.z == 0)
				{
					pVertices->data( )[tempBoneVertexIndex].m_weights.z = tempBoneWeight;
					pVertices->data( )[tempBoneVertexIndex].m_boneIndices.z = boneHierIdx;
				}
				// ��� 0�� �ƴϸ� 4��° �ε���, ����ġ�� 1���� xyz ���� ������ ���� �� ����
				else if (pVertices->data( )[tempBoneVertexIndex].m_weights.x != 0 && pVertices->data( )[tempBoneVertexIndex].m_weights.y != 0 && pVertices->data( )[tempBoneVertexIndex].m_weights.z != 0)
				{
					pVertices->data( )[tempBoneVertexIndex].m_boneIndices.w = boneHierIdx;
				}
			}	
		}	// end of bonecount for
	}	// end of deformer for
}

std::map<int, AnimationClip> FBXManager::LoadBoneInfomation( FbxNode* pNode, std::vector<Bone> BoneHierarchy )
{
	bool isAnimated = false;
	std::map<int, AnimationClip> tempAnimations;
	AnimationClip animation;
	std::string animName;

	animation.BoneAnimations.resize( BoneHierarchy.size( ) );
	// �ִϸ��̼�Ŭ���� ã�� ����
	int numAnimations = m_pfbxScene->GetSrcObjectCount<FbxAnimStack>();
	for (int animIndex = 0; animIndex < numAnimations; animIndex++)
	{
		FbxAnimStack *animStack = ( FbxAnimStack* )this->m_pfbxScene->GetSrcObject<FbxAnimStack>(animIndex);
		animName = animStack->GetName( );	// �̸�

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

	// BoneIdx �� -1�� ���� ����, 0���� �����ϱ� ����
	// ���� for�� �˻��ϰ� BoneIdx�� -1�̶�� �Ҹ��� ������������ ���� ����� �ǹ�
	// ���� ���� �˻縦 �� �ʿ䰡 ����
	if (BoneIdx == -1)
		return;

	FbxAnimLayer *pAnimLayer = (FbxAnimLayer*)pAnimStack->GetMember( );
	FbxAnimCurve *pfbxAnimCurve = pNode->LclRotation.GetCurve( pAnimLayer, "X" );

	long long mtxSize = pfbxAnimCurve->KeyGetCount( );

	if (mtxSize != 0)
	{
		std::vector<Keyframe> _mtx( mtxSize );
		BoneAnimation tempBoneAnim;

		for (long long i = 0; i < mtxSize; i++)
		{
			FbxTime nTime = pfbxAnimCurve->KeyGetTime( i );

			if (pNode)
			{	
				// �ش� ���� ������ǥ�迡���� ���
				_mtx[i].TimePos = (float)nTime.GetSecondDouble( );		// �ð�����

				FbxAMatrix aniMatrix = pNode->EvaluateLocalTransform( nTime );
				FbxQuaternion Q = aniMatrix.GetQ( );
				FbxVector4 S =  aniMatrix.GetS( );
				FbxVector4 T =  aniMatrix.GetT( );

				_mtx[i].RotationQuat.x = Q[0]; _mtx[i].RotationQuat.y = Q[1]; _mtx[i].RotationQuat.z = Q[2]; _mtx[i].RotationQuat.w = Q[3];
				_mtx[i].Scale.x = S[0]; _mtx[i].Scale.y = S[1]; _mtx[i].Scale.z = S[2];
				_mtx[i].Translation.x = T[0]; _mtx[i].Translation.y = T[1]; _mtx[i].Translation.z = T[2];
			}
		}

		// �� ������ �ִϸ��̼� ��Ʈ����
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

void FBXManager::LoadFBXMeshData( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertices, std::vector<UINT> *pIndices,
	std::vector<XMFLOAT2> *pUVs, std::vector<XMFLOAT4X4> *pBoneOffsets, std::vector<Bone> BoneHierarchy )
{
	//�ѤѤѤѤѤѤѤѤѤѤ� ���� ��ǥ, �ε��� ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	LoadVertexAndIndexInfomation( pMesh, pVertices, pIndices );

	// �ѤѤѤѤѤѤѤѤѤѤ� normal ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
//	LoadNormallnfomation( pMesh, pVertices );

	//�ѤѤѤѤѤѤѤѤѤѤ� tangent ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
//	LoadTangentInfomation( pMesh, pVertices );

	//�ѤѤѤѤѤѤѤѤѤѤ� binormal ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
//	LoadBinormalInfomation( pMesh, pVertices );

	//�ѤѤѤѤѤѤѤѤѤѤ� UV ��ǥ �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	std::vector<XMFLOAT2> tempUVs;
	LoadUVInformation( pMesh, &tempUVs );

	//�ѤѤѤѤѤѤѤѤѤѤ� ��ǥ���� �ߺ��� ���� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	pUVs->resize( pVertices->size( ) );
	for (int i = 0; i < pIndices->size( ); i++)
	{
		int idx = pIndices->data( )[i];
		pUVs->data( )[idx] = tempUVs[i];
	}	

	// �ѤѤѤѤѤѤѤѤѤѤ�  ����ġ�� ���� ������������ �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
	LoadInfluenceWeight( pMesh, pVertices, pBoneOffsets, BoneHierarchy );
}

void FBXManager::SaveData( std::vector<CFbxVertex> Vertex, std::vector<UINT> Index, std::vector<XMFLOAT2> UVVector, 
	std::vector<Bone> BoneHierarchy, std::vector<XMFLOAT4X4> BoneOffsets, std::map<int, AnimationClip> Animations, int iLayer, int iType, std::vector<TCHAR*> textures )
{
	CFbxMesh tempMesh;

	tempMesh.m_pVertices.resize( Vertex.size( ) );
	tempMesh.m_pVertices = Vertex;

	for (int i = 0; i < Vertex.size( ); i++)
	{
		// UV��ǥ ����
		tempMesh.m_pVertices[i].m_textureUV = UVVector[i];
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

	// ��Ű�� ������
	tempMesh.m_skinnedData.Set( BoneHierarchy, BoneOffsets, Animations );
	// �ؽ�ó�̸�
	tempMesh.m_pTextures = textures;

	m_pMeshes.push_back( tempMesh );
}

void FBXManager::LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertex, std::vector<UINT> *pIndex )
{
	FbxVector4* pVertices = pMesh->GetControlPoints( );

	// ���� ��ǥ���� ������ ����
	int polygonCount = pMesh->GetPolygonCount( );		// �������� ����
	int vertexCount = pMesh->GetControlPointsCount( );

		for (int polyCount = 0; polyCount < vertexCount; polyCount++)
		{
			CFbxVertex tempVertex;
			tempVertex.m_position = XMFLOAT3( pVertices[polyCount].mData[0], pVertices[polyCount].mData[1], pVertices[polyCount].mData[2] );
			pVertex->push_back( tempVertex );
		}

	int count = 0;

	for (int j = 0; j < polygonCount; j++)
	{
		int iNumVertices = pMesh->GetPolygonSize( j );	// �������� �����ϴ� ������ ����
		if (iNumVertices != 3)
		{
		//	pMesh->Destroy( );
		//	return;
		}

		for (int k = 0; k < iNumVertices; k++)
		{
			int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

			pIndex->push_back( iControlPointIndex );
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