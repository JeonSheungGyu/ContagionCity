#pragma once
#include "stdafx.h"
#include <fbxsdk.h>
#include "Mesh.h"
#include <vector>
using namespace std;

class FBXManager
{
private:
	FbxManager* m_pfbxManager;
	FbxImporter* m_pfbxImporter;
	FbxScene* m_pfbxScene;

	FBXManager( );

public:
	static FBXManager *Instance;
	static FBXManager *GetInstance( ) {	if (Instance == NULL) Instance = new FBXManager; return Instance; }
	~FBXManager( );

	std::vector<CFbxMesh> m_pMeshes;

	bool LoadFBX( const char* pstrFileName, int Layer, int Type );

	void ClearMeshes( ){ m_pMeshes.clear( ); }
	
};