#pragma once
#include "stdafx.h"
#include <fbxsdk.h>
#include "Mesh.h"

class FBXManager
{
private:
	FbxManager* m_pfbxManager;
	FbxImporter* m_pfbxImporter;
	FbxScene* m_pfbxScene;
	int m_nMeshCount;

public:
	FBXManager( );
	~FBXManager( );

	// 파일 경로에 있는 FBX를 로딩하여 scene에 추가
	bool LoadFBX( const char* pstrFileName );
	// scene에 있는 fbx노드들의 정점들을 CMesh의 정점들로 변환하여 반환
	bool LoadVertex( CMesh *pOutMeshes, int *pOutVertexCount );

	int getMeshCount( ){ return m_nMeshCount; }
};

