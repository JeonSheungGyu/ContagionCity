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

// FBX 파일을 Scene의 노드에 추가하는 함수
bool FBXManager::LoadFBX( std::vector<CFbxMesh> *pOutMeshes, const char* pstrFileName, int Layer, int Type )
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

				// 정점 좌표들을 저장할 공간
				int polygonCount = pMesh->GetPolygonCount( );		// 폴리곤의 개수
				int vertexCount = pMesh->GetControlPointsCount( );

				vector<XMFLOAT3> tempVector( vertexCount );
				vector<UINT> tempIndex;

				for (int j = 0; j < polygonCount; j++)
				{
					int iNumVertices = pMesh->GetPolygonSize( j );	// 폴리곤을 구성하는 정점의 개수
					if (iNumVertices != 3)
					{
						pMesh->Destroy( );
						return false;
					}

					for (int k = 0; k < iNumVertices; k++)
					{
						int iControlPointIndex = pMesh->GetPolygonVertex( j, k );

						tempIndex.push_back( iControlPointIndex );

						// 이 부분에서 각 버텍스를 메시에 저장하면 됌
						XMFLOAT3 temp;

						temp.x = (float)pVertices[iControlPointIndex].mData[0];
						temp.y = (float)pVertices[iControlPointIndex].mData[2];
						temp.z = (float)pVertices[iControlPointIndex].mData[1];

						tempVector[iControlPointIndex] = temp;
					}
				}
				CFbxMesh tempMesh;

				// 정점 좌표들의 모임
				tempMesh.m_pvPositions = tempVector;
				// vertex 개수
				tempMesh.m_nVertexCount = tempVector.size( );
				// 인덱스들의 모임
				tempMesh.m_pvIndices = tempIndex;
				// index 개수
				tempMesh.m_nIndexCount = tempIndex.size( );
				// 레이어
				tempMesh.m_iLayer = Layer;
				// 타입
				tempMesh.m_iType = Type;

				pOutMeshes->push_back( tempMesh );
			}
			else
			{
				m_nMeshCount--;
				continue;
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
