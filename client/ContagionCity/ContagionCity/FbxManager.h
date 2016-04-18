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

	// fbx용
	bool LoadFBX( const char* pstrFileName, int Layer, int Type );
	void ClearMeshes( ){ m_pMeshes.clear( ); }

	// m3d용
	bool LoadM3D( const std::string& fileName, int layer, int type, CM3dMesh& pOutMesh );

private:
	// fbx용
	void LoadFBXMeshData( FbxMesh* pMesh, int Layer, int Type );
	void LoadUVInformation( FbxMesh* pMesh, std::vector<XMFLOAT2> *pVector );
	void LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<XMFLOAT3> *pVertex, std::vector<UINT> *pIndex );
	void LoadNormallnfomation( FbxMesh *pMesh, std::vector<XMFLOAT3> *pNormals );
	void LoadTangentInfomation( FbxMesh *pMesh, std::vector<XMFLOAT4> *pTangents );
	void LoadBinormalInfomation( FbxMesh *pMesh, std::vector<XMFLOAT3> *pBinormals );
	void SaveData( std::vector<XMFLOAT3> pVertex, std::vector<UINT> pIndex, std::vector<XMFLOAT2> UVVector, std::vector<CFbxVertex>weights, int iLayer, int iType );

	std::map<std::string, AnimationClip> LoadBoneInfomation( FbxNode* pNode );
	void LoadBoneHierarachy( FbxMesh *pMesh, std::vector<CFbxVertex> *pVertices, std::vector<int> *pBoneHierachy, std::vector<XMFLOAT4X4> *pBoneOffsets );

	// m3d용
	void ReadAnimationClips( std::ifstream& fin, UINT nBones, UINT nAnimationClips, std::map<std::string, AnimationClip>& animationClips );
	void ReadBoneKeyFrames( std::ifstream& fin, UINT nBones, BoneAnimation& boneAnimation );
	void ReadBoneOffsets( std::ifstream& fin, UINT nBones, std::vector<XMFLOAT4X4>& boneOffsets );
	void ReadSkinnedVertices( std::ifstream& fin, UINT nVertices, std::vector<M3dVertex>& vertices );
	void ReadMaterial( std::ifstream& fin, UINT nMaterials, std::vector<M3dMaterial>& mats );
	void ReadSubsetTable( std::ifstream& fin, UINT nSubsets, std::vector<Subset>& subsets );
	void ReadTriangles( std::ifstream& fin, UINT nTriangles, std::vector<USHORT>& indices );
	void ReadBoneHierarchy( std::ifstream& fin, UINT nBones, std::vector<int>& boneIndexToParentIndex );

};