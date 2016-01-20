#pragma once
#include "stdafx.h"
#include <fbxsdk.h>

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
	bool SetVertex( ID3D11Device *pd3dDevice, CMesh *pOutMeshes );

	int getMeshCount( ){ return m_nMeshCount; }
};

