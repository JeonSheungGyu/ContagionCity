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
	static FBXManager *GetInstance( ) { if (Instance == NULL) Instance = new FBXManager; return Instance; }
	~FBXManager( );

	std::vector<CFbxMesh> m_pMeshes;

	// fbx¿ë
	bool LoadFBX( const char* pstrFileName, int Layer, int Type, int textureCount, ... );
	void ClearMeshes( ){ m_pMeshes.clear( ); }

private:
	bool FindStringInString( std::string dstString, std::string Findword );
	// fbx¿ë
	void SaveData( std::vector<CFbxVertex> Vertex, std::vector<UINT> Index, std::vector<XMFLOAT2> UVVector,
		std::vector<Bone> BoneHierarchy, std::vector<XMFLOAT4X4> BoneOffsets, std::map<int, AnimationClip>Animations, int iLayer, int iType, std::vector<TCHAR*> textures );
	void LoadFBXMeshData( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertices, std::vector<UINT> *pIndices, std::vector<XMFLOAT2> *pUVs,
		std::vector<XMFLOAT4X4> *pBoneOffsets, std::vector<Bone> BoneHierarchy );
	void LoadUVInformation( FbxMesh* pMesh, std::vector<XMFLOAT2> *pVertices );
	void LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertices, std::vector<UINT> *pIndex );
	void LoadGeometry( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertex );
	void LoadNormallnfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices );
	void LoadTangentInfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices );
	void LoadBinormalInfomation( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices );
	std::map<int, AnimationClip> LoadBoneInfomation( FbxNode* pNode, std::vector<Bone> pBoneHierarchy );
	void LoadInfluenceWeight( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices, std::vector<XMFLOAT4X4> *pBoneOffsets, std::vector<Bone> BoneHierarchy );
	void LoadBoneHierarachy( FbxNode *pNode, std::vector<Bone> *pBoneHierarchy );
	void LoadKeyframesByTime( FbxAnimStack *pAnimStack, FbxNode *pNode, std::vector<BoneAnimation>* pvAnimations, std::vector<Bone> BoneHierarchy );


	void LoadMeshData( FbxMesh* pMesh, std::vector<CFbxVertex> *pVertex, std::vector<UINT> *pIndex );

};