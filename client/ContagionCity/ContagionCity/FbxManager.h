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

	// ���� ��ο� �ִ� FBX�� �ε��Ͽ� scene�� �߰�
	bool LoadFBX( const char* pstrFileName );
	// scene�� �ִ� fbx������ �������� CMesh�� ������� ��ȯ�Ͽ� ��ȯ
	bool LoadVertex( CMesh *pOutMeshes, int *pOutVertexCount );

	int getMeshCount( ){ return m_nMeshCount; }
};

