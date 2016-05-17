#include "stdafx.h"
#include "NPCShader.h"


CNPCShader::CNPCShader( )
{
}


CNPCShader::~CNPCShader( )
{
}

void CNPCShader::BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes )
{
	for (int i = 0; i < meshes.size( ); i++)
	{
		switch (meshes[i].m_iType)
		{
			case NPC_PORTAL:
			{
				AnimatedObjectInfo *pRortal = new AnimatedObjectInfo( meshes[i] );
				pRortal->m_iType = meshes[i].m_iType;
				pRortal->m_iLayer = meshes[i].m_iLayer;

				int textureCount = meshes[i].m_pTextures.size( );
				CAnimatedMesh *pPortalMesh = new CAnimatedMesh( pd3dDevice, meshes[i], textureCount );
				pPortalMesh->FindMinMax( );		// AABB 상자 값 세팅
				for (int j = 0; j < textureCount; j++)
					pPortalMesh->OnChangeTexture( pd3dDevice, meshes[i].m_pTextures[j], j );
				pRortal->SetMesh( pPortalMesh, 0 );
				pRortal->CreateShaderVariables( pd3dDevice );

				CMaterial *pPortalMaterial = new CMaterial;
				pRortal->SetMaterial( pPortalMaterial );

				m_ppObjects.push_back( pRortal );

				break;
			}
		}
	}
	m_nObjects = m_ppObjects.size( );

}

void CNPCShader::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::Render( pd3dDeviceContext, pCamera );
}