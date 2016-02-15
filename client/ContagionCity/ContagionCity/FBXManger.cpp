#include "stdafx.h"
#include "FBXManager.h"
#include <vector>

FBXManager::FBXManager( )
{
	m_nMeshCount = 0;

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
bool FBXManager::LoadFBX( std::vector<CFbxMesh> *pOutMeshes, const char* pstrFileName, int Layer, int Type )
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

	m_nMeshCount += m_pfbxScene->GetRootNode( )->GetChildCount( );

	if (pfbxRootNode)
	{
		for (int i = 0; i < m_pfbxScene->GetRootNode( )->GetChildCount( ); i++)
		{
			FbxNode* pfbxChildNode = pfbxRootNode->GetChild( i );

			if (pfbxChildNode->GetNodeAttribute( ) == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pfbxChildNode->GetNodeAttribute( )->GetAttributeType( );

			if (AttributeType == FbxNodeAttribute::eMesh)
			{
				FbxMesh* pMesh = (FbxMesh*)pfbxChildNode->GetNodeAttribute( );

				FbxVector4* pVertices = pMesh->GetControlPoints( );

				// ���� ��ǥ���� ������ ����
				int polygonCount = pMesh->GetPolygonCount( );		// �������� ����
				int vertexCount = pMesh->GetControlPointsCount( );

				vector<XMFLOAT3> tempVector( vertexCount );
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

						// �� �κп��� �� ���ؽ��� �޽ÿ� �����ϸ� ��
						XMFLOAT3 temp;

						temp.x = (float)pVertices[iControlPointIndex].mData[0];
						temp.y = (float)pVertices[iControlPointIndex].mData[2];
						temp.z = (float)pVertices[iControlPointIndex].mData[1];

						tempVector[iControlPointIndex] = temp;
					}
				}
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

				pOutMeshes->push_back( tempMesh );
			}
			else
			{
				m_nMeshCount--;
				continue;
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
