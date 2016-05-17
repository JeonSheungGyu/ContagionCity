#include "stdafx.h"
#include "VilligeScene.h"
#include "NPCShader.h"

VilligeScene::VilligeScene( )
{
	CScene::CScene( );
	m_vPlayerStartPos = XMFLOAT3( 0.0f, 2.0f, 0.0f );
}


VilligeScene::~VilligeScene( )
{
}

void VilligeScene::CreateShaderVariables( ID3D11Device *pd3dDevice )
{
	m_pLights = new LIGHTS;
	::ZeroMemory( m_pLights, sizeof( LIGHTS ) );
	// 월드 전체를 비추는 주변 조명을 설정
	m_pLights->m_cGlobalAmbient = D3DXCOLOR( 0.1f, 0.1f, 0.1f, 1.0f );
	//	m_pLights->m_FogColor = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
	//	m_pLights->m_FogStart = 100.0;
	//	m_pLights->m_FogRange = 110.0;

	// 태양
	m_pLights->m_pLights[1].m_bEnable = true;
	m_pLights->m_pLights[1].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[1].m_cAmbient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights->m_pLights[1].m_cDiffuse = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_cSpecular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[1].m_vDirection = XMFLOAT3( 0.0f, -1.0f, 0.0f );

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

void VilligeScene::ReleaseShaderVariables( )
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release( );
}

void VilligeScene::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights )
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map( m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource );
	LIGHTS *pcbLights = (LIGHTS *)d3dMappedResource.pData;
	memcpy( pcbLights, pLights, sizeof( LIGHTS ) );
	pd3dDeviceContext->Unmap( m_pd3dcbLights, 0 );
	pd3dDeviceContext->PSSetConstantBuffers( PS_SLOT_LIGHT, 1, &m_pd3dcbLights );
}

void VilligeScene::LoadFBXs( )
{
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/city_base.FBX", LAYER_BACKGROUND, BACK_GROUND, 2,
		_T( "./res/StageVillige/city_base_0314_texture.dds" ), _T( "./res/StageVillige/city_base_0314_normal.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/city_hospital.FBX", LAYER_BACKGROUND, BACK_HOSPITAL, 1,
		_T( "./res/StageVillige/city_hospital_texture.dds" ));
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Shelter.FBX", LAYER_BACKGROUND, BACK_SHELTER, 1,
		_T( "./res/StageVillige/City_Shelter_texture.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Protective_Wall_0223.FBX", LAYER_BACKGROUND, BACK_PROTECTWALL, 1,
		_T( "./res/StageVillige/city_protective_wall_texture.jpg" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Store.FBX", LAYER_BACKGROUND, BACK_STORE, 1,
		_T( "./res/StageVillige/City_Store_texture.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Structure.FBX", LAYER_BACKGROUND, BACK_STRUCTURE, 1,
		_T( "./res/StageVillige/City_Structure_texture.dds" ));
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/city_wall_0411.FBX", LAYER_BACKGROUND, BACK_WALL, 1,
		_T( "./res/StageVillige/city_wall_texture.dds" ) );

	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Object1.FBX", LAYER_BACKGROUND, BACK_RUIN_1, 1,
		_T( "./res/StageVillige/City_Object1_texture.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Object2.FBX", LAYER_BACKGROUND, BACK_RUIN_2, 2,
		_T( "./res/StageVillige/City_Object2_texture.dds" ), _T( "./res/StageVillige/City_Object2_NormalMap.dds" ) );
	FBXManager::GetInstance( )->LoadFBX( "res/StageVillige/City_Object3.FBX", LAYER_BACKGROUND, BACK_RUIN_3, 2,
		_T( "./res/StageVillige/City_Object3_texture.dds" ), _T( "./res/StageVillige/City_Object3_NormalMap.dds" ) );

//	FBXManager::GetInstance( )->LoadFBX( "res/Effect/Portal.FBX", LAYER_NPC, NPC_PORTAL, 1,
//		_T( "./res/Effect/portal_texture.dds" ) );

	m_nFbxCount = FBXManager::GetInstance( )->m_pMeshes.size( );
}

void VilligeScene::BuildObjects( ID3D11Device *pd3dDevice )
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

	m_nShaders = 2;		// 0은 스카이박스 , 1은 배경, 2는 적, 3은 엔피시
	m_ppShaders = new CShader*[m_nShaders];

	// 첫번째로 그릴 객체는 스카이박스
	m_ppShaders[0] = new CSkyBoxShader( );
	m_ppShaders[0]->CreateShader( pd3dDevice );
	m_ppShaders[0]->BuildObjects( pd3dDevice );

	// 두번째는 배경
	m_ppShaders[1] = new CVilligeBackgroundShader( );
	m_ppShaders[1]->CreateShader( pd3dDevice );
	( (CVilligeBackgroundShader*)m_ppShaders[1] )->BuildObjects( pd3dDevice, background );

//	m_ppShaders[2] = new CNPCShader( );
//	m_ppShaders[2]->CreateShader( pd3dDevice );
//	( (CNPCShader*)m_ppShaders[2] )->BuildObjects( pd3dDevice, npc );

	CTrigger stage1Trigger( STAGE_1 );
	stage1Trigger.SetPosition( XMFLOAT3( -6000.0f, 50, -5700.0f ) );
	m_pTriggers.push_back( stage1Trigger );

//	m_ppShaders[2]->getObjects( )[0]->SetPosition( stage1Trigger.GetPosition( ) );

	m_nEffects = 1;
	m_ppEffectShaders = new CShader*[m_nEffects];
	m_ppEffectShaders[0] = new CTexture2DShader( pd3dDevice, _T( "res/portal.png" ), 100, 100 );
	m_ppEffectShaders[0]->getObjects( )[0]->SetPosition( m_pTriggers[0].GetPosition() );

	CreateShaderVariables( pd3dDevice );
}

bool VilligeScene::LoadingSoundResource( )
{
	SoundManager::GetInstance( )->Loading( "Sound/stage1bgm2.mp3", FMOD_LOOP_NORMAL, SOUND_BGM );
	SoundManager::GetInstance( )->Loading( "Sound/attack.wav", FMOD_DEFAULT, SOUND_ATTACK );

	return false;
}

void VilligeScene::AnimateObjects( float fTimeElapsed )
{
	if (m_pLights && m_pd3dcbLights)
	{
		// 현재 카메라의 위치 벡터를 조명을 나타내는 상수 버퍼에 설정
		XMFLOAT3 vCameraPosition = m_pCamera->GetPosition( );
		m_pLights->m_vCameraPosition = MathHelper::GetInstance( )->MakeFloat4( vCameraPosition, 1.0f );
		// 플레이어를 가리키는 조명
		m_pLights->m_pLights[0].m_vPosition = XMFLOAT3( m_pPlayer->GetPosition( ).x, 300, m_pPlayer->GetPosition( ).z );
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects( fTimeElapsed );
	}

	for (int i = 0; i < m_nEffects; i++)
	{
		( (CTexture2DShader*)( m_ppEffectShaders[i] ) )->MakeTransformToCamera( m_pCamera, m_ppEffectShaders[i]->getObjects( )[0]->GetPosition( ) );
	}
}