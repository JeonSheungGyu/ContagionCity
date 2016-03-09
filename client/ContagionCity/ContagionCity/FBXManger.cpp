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

					//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 정점 좌표, 인덱스 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
					vector<XMFLOAT3> tempVertex;
					vector<UINT> tempIndex;
					LoadVertexAndIndexInfomation( pMesh, &tempVertex, &tempIndex );

					// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ normal 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
					std::vector<XMFLOAT3> tempNormal;
					LoadNormalInfoamtion( pMesh, &tempNormal );

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

					//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 애니메이션 정보 가져오기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
					LoadBoneInfomation( pfbxChildNode );

					//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 자료들 저장하기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
					SaveData( tempVertex, tempIndex, tempUVVector, Layer, Type );
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

	// 정점 좌표들의 모임
	tempMesh.m_pvPositions = Vertex;
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
	// UV 좌표
	tempMesh.m_vTextureUV = UVVector;

	m_pMeshes.push_back( tempMesh );
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

