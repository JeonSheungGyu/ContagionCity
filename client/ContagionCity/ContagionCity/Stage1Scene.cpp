#include "stdafx.h"
#include "Stage1Scene.h"


CStage1Scene::CStage1Scene( )
{
	CScene::CScene( );
}


CStage1Scene::~CStage1Scene( )
{
}

void CStage1Scene::LoadFBXs( )
{
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_base_mod.FBX", LAYER_BACKGROUND, BACK_GROUND, 2, _T( "./res/Stage1/Stage1_1f_base_texture.bmp" ), _T( "./res/Stage1/Stage1_1f_baseNormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_wall.FBX", LAYER_BACKGROUND, BACK_WALL, 1, _T( "./res/Stage1/Stage1_wall_texture.bmp" ) );
//	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_ceiling.FBX", LAYER_BACKGROUND, BACK_GROUND, 1, _T( "./res/city_base_0314_texture.dds" ) );

	FBXManager::GetInstance( )->LoadFBX( "res/animation example.FBX", LAYER_ENEMY, ENEMY_ZOMBIE, 1, _T( "./res/city_base_0314_texture.dds" ) );

	m_nFbxCount = FBXManager::GetInstance( )->m_pMeshes.size( );
}

void CStage1Scene::BuildObjects( ID3D11Device *pd3dDevice )
{
	// ���� �ε� �� ���
	LoadingSoundResource( );
	SoundManager::GetInstance( )->Play( BGM );

	// �ؽ�ó ���ο� ����� ���÷� ���� ��ü�� ����
	ID3D11SamplerState *pd3dSamplerState = NULL;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory( &d3dSamplerDesc, sizeof( D3D11_SAMPLER_DESC ) );
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState( &d3dSamplerDesc, &pd3dSamplerState );

	// fbx loader�� ���� fbx�ε�
	LoadFBXs( );
	std::vector<CFbxMesh> background;
	std::vector<CFbxMesh> enemy;
	std::vector<CFbxMesh> npc;

	for (int i = 0; i < m_nFbxCount; i++)
	{
		CFbxMesh tempMesh = FBXManager::GetInstance( )->m_pMeshes[i];
		int layer = tempMesh.m_iLayer;
		switch (layer)
		{
			case ObjectLayer::LAYER_BACKGROUND:
			{
				background.push_back( tempMesh );
				break;
			}
			case ObjectLayer::LAYER_ENEMY:
			{
				enemy.push_back( tempMesh );
				break;
			}
			case ObjectLayer::LAYER_NPC:
			{
				npc.push_back( tempMesh );
				break;
			}
			default:
				break;
		}
	}
	FBXManager::GetInstance( )->ClearMeshes( );

	// ���� ���̴� ������ fbx������ ���� �� ���̴��� �����Ͽ� ����
	m_nShaders = 3;		// 0�� ��ī�̹ڽ� , 1�� ���, 2�� ��, 3�� ���ǽ�
	m_ppShaders = new CShader*[m_nShaders];

	// ù��°�� �׸� ��ü�� ��ī�̹ڽ�
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	// �ι�°�� ���
	m_ppShaders[1] = new CBackgroundShader( );
	m_ppShaders[1]->CreateShader( pd3dDevice );
	( (CBackgroundShader*)m_ppShaders[1] )->BuildObjects( pd3dDevice, background );

	// ����°�� ����
	m_ppShaders[2] = new CEnemyShader( );
	m_ppShaders[2]->CreateShader( pd3dDevice );
	( (CEnemyShader*)m_ppShaders[2] )->BuildObjects( pd3dDevice, enemy );

	CreateShaderVariables( pd3dDevice );
}

bool CStage1Scene::LoadingSoundResource( )
{
	SoundManager::GetInstance( )->Loading( "Sound/stage1bgm2.mp3", FMOD_LOOP_NORMAL, BGM );
	SoundManager::GetInstance( )->Loading( "Sound/attack.wav", FMOD_DEFAULT, ATTACK );

	return false;
}
