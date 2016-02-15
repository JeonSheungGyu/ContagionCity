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
	int m_nMeshCount;
	vector<int> m_vTypes;
	vector<int> m_vLayers;
public:
	FBXManager( );
	~FBXManager( );

	// ���� ��ο� �ִ� FBX�� �ε��Ͽ� scene�� �߰�
	bool LoadFBX( std::vector<CFbxMesh> *pOutMeshes, const char* pstrFileName, int Layer, int Type );

	int getMeshCount( ){ return m_nMeshCount; }
	vector<int> getTypes( ){ return m_vTypes; }
	vector<int> getLayers( ) { return m_vLayers; }

};

