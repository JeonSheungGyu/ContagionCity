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
bool FBXManager::LoadFBX( const char* pstrFileName, int Type )
{
	// ���� �̸��� �ε�
	bool bResult = m_pfbxImporter->Initialize( pstrFileName, -1, m_pfbxManager->GetIOSettings( ) );
	if (!bResult)
		return false;

	// Scene�� �߰�, Scene�� ��Ʈ����� ������ ���Ե�
	bResult = m_pfbxImporter->Import( m_pfbxScene );
	if (!bResult)
		return false;

	m_vTypes.push_back( Type );
	// ���� Scene�� ����Ǿ��ִ� �޽õ��� ������ ����
	m_nMeshCount = m_pfbxScene->GetRootNode( )->GetChildCount( );
	return true;
}

// Scene�� ������ ���� ��ǥ���� ���� ����� �������� ��ȯ�ϴ� �Լ�
bool FBXManager::LoadVertex( std::vector<CFbxVertex> *pOutMeshes )
{
	FbxNode *pfbxRootNode = m_pfbxScene->GetRootNode( );

	if (pfbxRootNode)
	{
		int ChildCount = pfbxRootNode->GetChildCount( );
		
		for (int i = 0; i < ChildCount; i++)
		{
			FbxNode* pfbxChildNode = pfbxRootNode->GetChild( i );

			if (pfbxChildNode->GetNodeAttribute( ) == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pfbxChildNode->GetNodeAttribute( )->GetAttributeType( );

			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			FbxMesh* pMesh = (FbxMesh*)pfbxChildNode->GetNodeAttribute( );

			FbxVector4* pVertices = pMesh->GetControlPoints( );

			// ���� ��ǥ���� ������ ����
			int polygonCount = pMesh->GetPolygonCount( );
			vector<XMFLOAT3> temp( polygonCount * 3 );

			for (int j = 0; j < polygonCount; j++)
			{
				int iNumVertices = pMesh->GetPolygonSize( j );
				if (iNumVertices != 3)
					return false;

				for (int k = 0; k < iNumVertices; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

					// �� �κп��� �� ���ؽ��� �޽ÿ� �����ϸ� ��
					temp[j * 3 + k].x = (float)pVertices[iControlPointIndex].mData[0];
					temp[j * 3 + k].y = (float)pVertices[iControlPointIndex].mData[1];
					temp[j * 3 + k].z = (float)pVertices[iControlPointIndex].mData[2];
					
				}
			}
			// ���� ��ǥ���� ����
			( *pOutMeshes )[i].m_pvPositions = temp;
			// vertex ����
			( *pOutMeshes )[i].m_nVertexCount = polygonCount * 3;
			// Ÿ��
			( *pOutMeshes )[i].m_iType = m_vTypes[i];
		}
	}
	// ���ؽ� �������� �ű� �� ���� ����
	for (int i = 0; i < m_pfbxScene->GetRootNode( )->GetChildCount( ); i++)
	{
		FbxNode* temp = pfbxRootNode->GetChild( i );
		m_pfbxScene->RemoveNode( temp );
	}
	m_nMeshCount = 0;
	return true;
}