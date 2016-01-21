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
	m_pfbxManager->Destroy( );
}

// FBX ������ Scene�� ��忡 �߰��ϴ� �Լ�
bool FBXManager::LoadFBX( const char* pstrFileName )
{
	// ���� �̸��� �ε�
	bool bResult = m_pfbxImporter->Initialize( pstrFileName, -1, m_pfbxManager->GetIOSettings( ) );
	if (!bResult)
		return false;

	// Scene�� �߰�, Scene�� ��Ʈ����� ������ ���Ե�
	bResult = m_pfbxImporter->Import( m_pfbxScene );
	if (!bResult)
		return false;

	m_pfbxImporter->Destroy( );

	// ���� Scene�� ����Ǿ��ִ� �޽õ��� ������ ����
	m_nMeshCount = m_pfbxScene->GetRootNode( )->GetChildCount( );
}

// Scene�� ������ ���� ��ǥ���� ���� ����� �������� ��ȯ�ϴ� �Լ�
bool FBXManager::LoadVertex( CMesh *pOutMeshes, int *pOutVertexCount )
{
	FbxNode *pfbxRootNode = m_pfbxScene->GetRootNode( );

	if (pfbxRootNode)
	{
		int ChildCount = pfbxRootNode->GetChildCount( );
		pOutMeshes = new CMesh[ChildCount];
		pOutVertexCount = new int[ChildCount];

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

			for (int j = 0; j < pMesh->GetPolygonCount( ); j++)
			{
				int iNumVertices = pMesh->GetPolygonSize( j );
				if (iNumVertices != 3)
					return false;
			
				// ���� ��ǥ���� ������ ����
				XMFLOAT3* temp = new XMFLOAT3[pMesh->GetPolygonCount()*3];

				for (int k = 0; k < iNumVertices; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

					// �� �κп��� �� ���ؽ��� �޽ÿ� �����ϸ� ��
					temp[j * 3 + k].x = (float)pVertices[iControlPointIndex].mData[0];
					temp[j * 3 + k].y = (float)pVertices[iControlPointIndex].mData[1];
					temp[j * 3 + k].z = (float)pVertices[iControlPointIndex].mData[2];
					
				}
				// ���� ��ǥ���� ����
				pOutMeshes[i].m_pvPositions = temp;
				// vertex ����
				pOutVertexCount[i] = j * 3;
			}
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