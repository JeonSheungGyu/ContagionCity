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
	void LoadUVInformation( FbxMesh* pMesh, std::vector<XMFLOAT2> *pVector );
	void LoadVertexAndIndexInfomation( FbxMesh* pMesh, std::vector<XMFLOAT3> *pVertex, std::vector<UINT> *pIndex );
	void LoadNormalInfoamtion( FbxMesh *pMesh, std::vector<XMFLOAT3> *pOut );
	std::map<std::string, AnimationClip> LoadBoneInfomation( FbxNode* pNode );
	void LoadBoneHierarachy( FbxMesh *pMesh );
	void SaveData( std::vector<XMFLOAT3> pVertex, std::vector<UINT> pIndex, std::vector<XMFLOAT2> UVVector, int iLayer, int iType );

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