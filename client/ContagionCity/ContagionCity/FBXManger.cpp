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
		for (int i = 0; i < m_pfbxScene->GetRootNode( )->GetChildCount( ); i++)
		{
			FbxNode* pfbxChildNode = pfbxRootNode->GetChild( i );

			if (pfbxChildNode->GetNodeAttribute( ) == NULL)
				continue;

			FbxMesh* pMesh = (FbxMesh*)pfbxChildNode->GetNodeAttribute( );
			
			//�ѤѤѤѤѤѤѤѤѤѤ� ���� ��ǥ ��������ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
			FbxVector4* pVertices = pMesh->GetControlPoints( );

			// ���� ��ǥ���� ������ ����
			int polygonCount = pMesh->GetPolygonCount( );		// �������� ����
			int vertexCount = pMesh->GetControlPointsCount( );

			vector<XMFLOAT3> tempVector;
			for (int polyCount = 0; polyCount < vertexCount; polyCount++)
			{
				tempVector.push_back( XMFLOAT3(pVertices[polyCount].mData[0], pVertices[polyCount].mData[2], pVertices[polyCount].mData[1]) );
			}

			//�ѤѤѤѤѤѤѤѤѤѤ� �ε��� ���� ��������ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
			vector<UINT> tempIndex;

			for (int j = 0; j < polygonCount; j++)
			{
				int iNumVertices = pMesh->GetPolygonSize( j );	// �������� �����ϴ� ������ ����
				if (iNumVertices != 3)
				{
					pMesh->Destroy( );
					return false;
				}

				for (int k = 0; k < iNumVertices; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

					tempIndex.push_back( iControlPointIndex );
				}
			}

			//�ѤѤѤѤѤѤѤѤѤѤ� UV ��ǥ ��������ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
			std::vector<XMFLOAT2> tempUVVector;
			LoadUVInformation( pMesh, &tempUVVector );

			std::vector<XMFLOAT2> UVVectorByControlPoint( tempVector.size( ) );
			for (int idx = 0; idx < tempIndex.size( ); idx++)
			{
				UVVectorByControlPoint[tempIndex[idx]] = tempUVVector[idx];
			}

			//�ѤѤѤѤѤѤѤѤѤѤ� �ڷ�� �����ϱ�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�
			CFbxMesh tempMesh;

			// ���� ��ǥ���� ����
			tempMesh.m_pvPositions = tempVector;
			// vertex ����
			tempMesh.m_nVertexCount = tempVector.size( );
			// �ε������� ����
			tempMesh.m_pvIndices = tempIndex;
			// index ����
			tempMesh.m_nIndexCount = tempIndex.size( );
			// ���̾�
			tempMesh.m_iLayer = Layer;
			// Ÿ��
			tempMesh.m_iType = Type;
			// UV ��ǥ
			tempMesh.m_vTextureUV = UVVectorByControlPoint;

			m_pMeshes.push_back( tempMesh );
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
