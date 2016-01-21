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

	bool LoadFBX( const char* pstrFileName );
	bool LoadVertex( CMesh *pOutMeshes, int *pOutVertexCount );

	int getMeshCount( ){ return m_nMeshCount; }
};

