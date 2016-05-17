#include "stdafx.h"
#include "Stage1Scene.h"


CStage1Scene::CStage1Scene( )
{
	CScene::CScene( );
	m_vPlayerStartPos = XMFLOAT3( 5290.0f, 2.0f, 565.0f );
}


CStage1Scene::~CStage1Scene( )
{
}

void CStage1Scene::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	m_pLights = new LIGHTS;
	::ZeroMemory( m_pLights, sizeof( LIGHTS ) );
	// 월드 전체를 비추는 주변 조명을 설정
	m_pLights->m_cGlobalAmbient = D3DXCOLOR( 0.3f, 0.3f, 0.3f, 1.0f );
//	m_pLights->m_FogColor = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
//	m_pLights->m_FogStart = 100.0;
//	m_pLights->m_FogRange = 110.0;s

	// 플레이어를 가리킬 조명
	m_pLights->m_pLights[0].m_bEnable = 1.0f;
	m_pLights->m_pLights[0].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 350.0f;
	m_pLights->m_pLights[0].m_cAmbient = D3DXCOLOR( 0.5f, 0.5f, 0.5f, 1.0f );
	m_pLights->m_pLights[0].m_cDiffuse = D3DXCOLOR( 0.4f, 0.4f, 0.4f, 1.0f );
	m_pLights->m_pLights[0].m_cSpecular = D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.0f );
	m_pLights->m_pLights[0].m_vPosition = XMFLOAT3( 0, 120, 2500.0f );
	m_pLights->m_pLights[0].m_vDirection = XMFLOAT3( 0.0f, -1.0f, 0.0f );
	m_pLights->m_pLights[0].m_vAttenuation = XMFLOAT3( 1.0f, 0.11f, 0.011f );
	m_pLights->m_pLights[0].m_fFalloff = 70.0f;
	m_pLights->m_pLights[0].m_fPhi = (float)cos( D3DXToRadian( 80.0f ) );
	m_pLights->m_pLights[0].m_fTheta = (float)cos( D3DXToRadian( 20.0f ) );

	for (int i = 1; i < 4; i++)
	{
		m_pLights->m_pLights[i].m_bEnable = 1.0f;
		m_pLights->m_pLights[i].m_nType = SPOT_LIGHT;
		m_pLights->m_pLights[i].m_fRange = 1500.0f;
		m_pLights->m_pLights[i].m_cAmbient = D3DXCOLOR( 0.6f, 0.6f, 0.6f, 1.0f );
		m_pLights->m_pLights[i].m_cDiffuse = D3DXCOLOR( 0.3f, 0.3f, 0.3f, 1.0f );
		m_pLights->m_pLights[i].m_cSpecular = D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.0f );
		m_pLights->m_pLights[i].m_vPosition = XMFLOAT3( - 4000 + 3000.0f * i, 440.0f, 2500.0f );
		m_pLights->m_pLights[i].m_vDirection = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		m_pLights->m_pLights[i].m_vAttenuation = XMFLOAT3( 1.0f, 0.41f, 0.041f );
		m_pLights->m_pLights[i].m_fFalloff = 10.0f;
		m_pLights->m_pLights[i].m_fPhi = (float)cos( D3DXToRadian( 80.0f ) );
		m_pLights->m_pLights[i].m_fTheta = (float)cos( D3DXToRadian( 60.0f ) );
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory( &d3dBufferDesc, sizeof( d3dBufferDesc ) );
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof( LIGHTS );
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory( &d3dBufferData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	d3dBufferData.pSysMem = m_pLights;
	pd3dDevice->CreateBuffer( &d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights );
}

void CStage1Scene::ReleaseShaderVariables( )
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release( );
}

void CStage1Scene::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights )
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	LIGHTS *pcbLights = (LIGHTS *)d3dMappedResource.pData;
	memcpy( pcbLights, pLights, sizeof( LIGHTS ) );
	pd3dDeviceContext->Unmap( m_pd3dcbLights, 0 );
	pd3dDeviceContext->PSSetConstantBuffers( PS_SLOT_LIGHT, 1, &m_pd3dcbLights );
}

void CStage1Scene::LoadFBXs( )
{
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_base_mod.FBX", LAYER_BACKGROUND, BACK_GROUND, 2,
		_T( "./res/Stage1/Stage1_1f_base_texture.bmp" ), _T( "./res/Stage1/Stage1_1f_baseNormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_wall.FBX", LAYER_BACKGROUND, BACK_WALL, 2,
		_T( "./res/Stage1/Stage1_wall_texture.bmp" ), _T( "./res/Stage1/Stage1_wallNormalsMap.DDS" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_ceiling.FBX", LAYER_BACKGROUND, BACK_CEIL, 2,
		_T( "./res/Stage1/Stage1_1f_ceiling_texture.bmp" ), _T( "./res/Stage1/Stage1_1f_ceiling_NormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_bench.FBX", LAYER_BACKGROUND, BACK_BENCH, 2,
		_T( "./res/Stage1/Stage1_1f_bench_texture_mod.bmp" ), _T( "./res/Stage1/Stage1_1f_benchNormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_chair.FBX", LAYER_BACKGROUND, BACK_CHAIR, 2,
		_T( "./res/Stage1/Stage1_1f_chair_texture_mod.bmp" ), _T( "./res/Stage1/Stage1_1f_chairNormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_frontdesk.FBX", LAYER_BACKGROUND, BACK_FRONTDESK, 2,
		_T( "./res/Stage1/Stage1_1f_frontdesk_texture.bmp" ), _T( "./res/Stage1/Stage1_1f_frontdeskNormalsMap.dds" ) );
//	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_2f_door.FBX", LAYER_BACKGROUND, BACK_DOOR, 2, 
//		_T( "./res/Stage1/Stage1_2f_door_texture.bmp" ), _T( "./res/Stage1/Stage1_2f_doorNormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_2f_bed.FBX", LAYER_BACKGROUND, BACK_BED, 2,
		_T( "./res/Stage1/Stage1_2f_bed_texture.bmp" ), _T( "./res/Stage1/Stage1_2f_bedNormalsMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_1f_entrance.FBX", LAYER_BACKGROUND, BACK_ENTERANCE, 1,
		_T( "./res/Stage1/Stage1_1f_entrance_texture.dds" ) );
//	FBXManager::GetInstance( )->LoadFBX( "res/Stage1/Stage1_2f_doorwall.FBX", LAYER_BACKGROUND, BACK_DOORWALL, 1,
//		_T( "./res/Stage1/Stage1_2f_doorwall_texture.dds" ));

	FBXManager::GetInstance( )->LoadFBX( "res/monster_man/Monster_man_animation.FBX", LAYER_ENEMY, ENEMY_ZOMBIE_MAN, 2,
		_T( "./res/monster_man/monster_man_texture.dds" ), _T( "./res/monster_man/monster_man_NormalsMap.dds" )	 );
	FBXManager::GetInstance( )->LoadFBX( "res/monster_woman/Monster_woman_animation.FBX", LAYER_ENEMY, ENEMY_ZOMBIE_WOMAN, 2,
		_T( "./res/monster_woman/Monster_woman_texture.dds" ), _T( "./res/monster_woman/Monster_woman_NormalsMap.dds" ) );

	m_nFbxCount = FBXManager::GetInstance( )->m_pMeshes.size( );
}

void CStage1Scene::BuildObjects( ID3D11Device *pd3dDevice )
{
	// 사운드 로딩 및 출력
	LoadingSoundResource( );
	SoundManager::GetInstance( )->Play( SOUND_BGM );

	// 텍스처 맵핑에 사용할 샘플러 상태 객체를 생성
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

	// fbx loader를 통한 fbx로딩
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

	m_nShaders = 3;		// 0은 스카이박스 , 1은 배경, 2는 적, 3은 엔피시
	m_ppShaders = new CShader*[m_nShaders];

	// 첫번째로 그릴 객체는 스카이박스
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	// 두번째는 배경
	m_ppShaders[1] = new CStage1BackgroundShader( );
	m_ppShaders[1]->CreateShader( pd3dDevice );
	( (CStage1BackgroundShader*)m_ppShaders[1] )->BuildObjects( pd3dDevice, background );

	// 세번째는 몬스터
	m_ppShaders[2] = new CEnemyShader( );
	m_ppShaders[2]->CreateShader( pd3dDevice );
	( (CEnemyShader*)m_ppShaders[2] )->BuildObjects( pd3dDevice, enemy );

	CreateShaderVariables( pd3dDevice );
}

bool CStage1Scene::LoadingSoundResource( )
{
	SoundManager::GetInstance( )->Loading( "Sound/stage1bgm2.mp3", FMOD_LOOP_NORMAL, SOUND_BGM );
	SoundManager::GetInstance( )->Loading( "Sound/attack.wav", FMOD_DEFAULT, SOUND_ATTACK );

	return false;
}

void CStage1Scene::AnimateObjects( float fTimeElapsed )
{
	if (m_pLights && m_pd3dcbLights)
	{
		// 현재 카메라의 위치 벡터를 조명을 나타내는 상수 버퍼에 설정
		XMFLOAT3 vCameraPosition = m_pCamera->GetPosition( );
		m_pLights->m_vCameraPosition = MathHelper::GetInstance( )->MakeFloat4( vCameraPosition, 1.0f );
		m_pLights->m_pLights[0].m_vPosition = XMFLOAT3( m_pPlayer->GetPosition( ).x, 300, m_pPlayer->GetPosition( ).z );
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects( fTimeElapsed );
	}
}