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

// FBX 파일을 Scene의 노드에 추가하는 함수
bool FBXManager::LoadFBX( const char* pstrFileName )
{
	// 파일 이름을 로딩
	bool bResult = m_pfbxImporter->Initialize( pstrFileName, -1, m_pfbxManager->GetIOSettings( ) );
	if (!bResult)
		return false;

	// Scene에 추가, Scene의 루트노드의 하위로 들어가게됨
	bResult = m_pfbxImporter->Import( m_pfbxScene );
	if (!bResult)
		return false;

	m_pfbxImporter->Destroy( );

	// 현재 Scene에 저장되어있는 메시들의 개수를 저장
	m_nMeshCount = m_pfbxScene->GetRootNode( )->GetChildCount( );
}

// Scene의 노드들의 정점 좌표들을 내가 사용할 정점으로 변환하는 함수
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
			
				// 정점 좌표들을 저장할 공간
				XMFLOAT3* temp = new XMFLOAT3[pMesh->GetPolygonCount()*3];

				for (int k = 0; k < iNumVertices; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

					// 이 부분에서 각 버텍스를 메시에 저장하면 됌
					temp[j * 3 + k].x = (float)pVertices[iControlPointIndex].mData[0];
					temp[j * 3 + k].y = (float)pVertices[iControlPointIndex].mData[1];
					temp[j * 3 + k].z = (float)pVertices[iControlPointIndex].mData[2];
					
				}
				// 정점 좌표들의 모임
				pOutMeshes[i].m_pvPositions = temp;
				// vertex 개수
				pOutVertexCount[i] = j * 3;
			}
		}
	}
	// 버텍스 정보들을 옮긴 뒤 노드들 제거
	for (int i = 0; i < m_pfbxScene->GetRootNode( )->GetChildCount( ); i++)
	{
		FbxNode* temp = pfbxRootNode->GetChild( i );
		m_pfbxScene->RemoveNode( temp );
	}
	m_nMeshCount = 0;
	return true;
}