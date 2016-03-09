#include "stdafx.h"
#include "FBXManager.h"
#include <vector>

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

			FbxNodeAttribute *nodeAttribute = pfbxChildNode->GetNodeAttributeByIndex( i );
			if (nodeAttribute == NULL)
				continue;
			FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType( );

			FbxMesh* pMesh = (FbxMesh*)pfbxChildNode->GetNodeAttribute( );
			switch (attributeType)
			{
				case FbxNodeAttribute::eMesh:
				{
					FbxMesh* pMesh = (FbxMesh*)pfbxChildNode->GetNodeAttribute( );

					//�ѤѤѤѤѤѤѤѤѤѤ� ���� ��ǥ, �ε��� ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤ�
					vector<XMFLOAT3> tempVertex;
					vector<UINT> tempIndex;
					LoadVertexAndIndexInfomation( pMesh, &tempVertex, &tempIndex );

					// �ѤѤѤѤѤѤѤѤѤѤ� normal ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
					std::vector<XMFLOAT3> tempNormal;
					LoadNormalInfoamtion( pMesh, &tempNormal );

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

					//�ѤѤѤѤѤѤѤѤѤѤ� �ִϸ��̼� ���� �������� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
					LoadBoneInfomation( pfbxChildNode );

					//�ѤѤѤѤѤѤѤѤѤѤ� �ڷ�� �����ϱ� �ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
					SaveData( tempVertex, tempIndex, tempUVVector, Layer, Type );
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
void FBXManager::LoadNormalInfoamtion( FbxMesh *pMesh, std::vector<XMFLOAT3> *pOut )
{
	for (int i = 0; i < pMesh->GetPolygonVertexCount( ); i++)
	{
		XMFLOAT3 tempNormal;
		FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal( 0 );

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
		pOut->push_back( tempNormal );
	}
}

void FBXManager::LoadBoneInfomation( FbxNode* pNode )
{
	int numStack = m_pfbxScene->GetSrcObjectCount<FbxAnimStack>( );

	for (int i = 0; i < numStack; i++)
	{
		FbxAnimStack* lAnimStack = m_pfbxScene->GetSrcObject<FbxAnimStack>( i );
		int numAnimLayer = lAnimStack->GetMemberCount<FbxAnimLayer>( );

		for (int j = 0; j < numAnimLayer; j++)
		{
			FbxAnimLayer* lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>( );
			//	FbxAnimCurve* lAnimCurve = pNode->LclTranslation.GetCurve( lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );

		}
	}
}

void FBXManager::SaveData( std::vector<XMFLOAT3> Vertex, std::vector<UINT> Index, std::vector<XMFLOAT2> UVVector, int iLayer, int iType )
{
	CFbxMesh tempMesh;

	// ���� ��ǥ���� ����
	tempMesh.m_pvPositions = Vertex;
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
	// UV ��ǥ
	tempMesh.m_vTextureUV = UVVector;

	m_pMeshes.push_back( tempMesh );
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

