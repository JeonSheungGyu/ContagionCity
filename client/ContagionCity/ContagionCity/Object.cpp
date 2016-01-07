#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

CMaterial::CMaterial( )
{
	m_nReferences = 0;
}

CMaterial::~CMaterial( )
{

}

CGameObject::CGameObject( int nMeshes )
{
	m_mtxWorld = MathHelper::GetInstance( )->GetMatrixIdentity( );
	m_pMaterial = NULL;
	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	// 메시를 여러개 갖는다.
	if (m_nMeshes > 0)
		m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)
		m_ppMeshes[i] = NULL;

	m_bcMeshBoundingCube = AABB( );
	m_nReferences = 0;
	m_pTexture = NULL;
}

CGameObject::~CGameObject( )
{
	if (m_pMaterial) m_pMaterial->Release( );
	if (m_pTexture) m_pTexture->Release( );
	if (m_ppMeshes)
	{
		for (int i = 0; i++; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Release( );
			m_ppMeshes[i] = NULL;
		}
		delete[ ] m_ppMeshes;
	}
}

void CGameObject::SetTexture( CTexture *pTexture )
{
	if (m_pTexture) m_pTexture->Release( );
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef( );
}

bool CGameObject::IsVisible( CCamera *pCamera )
{
	OnPrepareRender( );

	bool bIsVisible = false;

	AABB bcBoundingCube = m_bcMeshBoundingCube;
	bcBoundingCube.Update( &m_mtxWorld );
	if (pCamera) bIsVisible = pCamera->IsInFrustum( &bcBoundingCube );

	return bIsVisible;
}

void CGameObject::SetMaterial( CMaterial *pMaterial )
{
	if (m_pMaterial) m_pMaterial->Release( );
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef( );
}

void CGameObject::SetMesh( CMesh *pMesh, int nIndex )
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex])
			m_ppMeshes[nIndex]->Release( );
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) 
			pMesh->AddRef( );
	}

	if (pMesh)
	{
		AABB bcBoundingCube = pMesh->GetBoundingCube( );
		// 게임 객체는 항상 AABB를 유지하도록 한다.
		m_bcMeshBoundingCube.Union( &bcBoundingCube );
	}
}


// 월드 변환
void CGameObject::SetPosition( float x, float y, float z )
{
	m_mtxWorld._41 = x;
	m_mtxWorld._42 = y;
	m_mtxWorld._43 = z;
}

void CGameObject::SetPosition( XMFLOAT3 vPosition )
{
	m_mtxWorld._41 = vPosition.x;
	m_mtxWorld._42 = vPosition.y;
	m_mtxWorld._43 = vPosition.z;
}

XMFLOAT3 CGameObject::GetPosition( )
{
	return XMFLOAT3( m_mtxWorld._41, m_mtxWorld._42, m_mtxWorld._43 );
}

void CGameObject::Move( XMFLOAT3& vDirection, float fSpeed )
{
	SetPosition( m_mtxWorld._41 + vDirection.x * fSpeed, m_mtxWorld._42 + vDirection.y * fSpeed, m_mtxWorld._43 + vDirection.z * fSpeed );
}

void CGameObject::Animate( float fTimeElapsed )
{
	if (m_fRotationSpeed != 0.0f)
		Rotate( &m_vRotationAxis, m_fRotationSpeed*fTimeElapsed );
	if (m_fMovingSpeed != 0.0f)
	{
		XMFLOAT3 vPosition = GetPosition( );
		Move( m_vMovingDirection, m_fMovingSpeed*fTimeElapsed );
	}
}

XMFLOAT3 CGameObject::GetLookAt( )
{
	// 게임 객체를 로컬 z축 벡터를 반환
	XMFLOAT3 vLookAt( m_mtxWorld._31, m_mtxWorld._32, m_mtxWorld._33 );
	vLookAt = MathHelper::GetInstance( )->NormalizeFloat3( vLookAt );

	return vLookAt;
}

XMFLOAT3 CGameObject::GetUp( )
{
	// 게임 객체를 로컬 y축 벡터를 반환
	XMFLOAT3 vUp( m_mtxWorld._21, m_mtxWorld._22, m_mtxWorld._23 );
	vUp = MathHelper::GetInstance( )->NormalizeFloat3( vUp );

	return vUp;
}

XMFLOAT3 CGameObject::GetRight( )
{
	// 게임 객체를 로컬 x축 벡터를 반환
	XMFLOAT3 vRight( m_mtxWorld._11, m_mtxWorld._12, m_mtxWorld._13 );
	vRight = MathHelper::GetInstance( )->NormalizeFloat3( vRight );

	return vRight;
}

void CGameObject::MoveStrafe( float fDistance )
{
	// 게임 객체를 로컬 x축 방향으로 이동
	XMFLOAT3 vPosition = GetPosition( );
	XMFLOAT3 vRight = MathHelper::GetInstance( )->Float3MulFloat( GetRight( ), fDistance );

	vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( vPosition, vRight );
	CGameObject::SetPosition( vPosition );
}

void CGameObject::MoveUp( float fDistance )
{
	// 게임 객체를 로컬 y축 방향으로 이동
	XMFLOAT3 vPosition = GetPosition( );
	XMFLOAT3 vUp = MathHelper::GetInstance( )->Float3MulFloat( GetUp( ), fDistance );

	vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( vPosition, vUp );
	CGameObject::SetPosition( vPosition );
}

void CGameObject::MoveForward( float fDistance )
{
	// 게임 객체를 로컬 z축 방향으로 이동
	XMFLOAT3 vPosition = GetPosition( );
	XMFLOAT3 vLookAt = MathHelper::GetInstance( )->Float3MulFloat( GetLookAt( ), fDistance );

	vPosition = MathHelper::GetInstance( )->Float3PlusFloat3( vPosition, vLookAt );
	CGameObject::SetPosition( vPosition );
}

void CGameObject::Rotate( float fPitch, float fYaw, float fRoll )
{
	// 게임 객체를 주어진 각도로 회전
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw( (float)XMConvertToRadians( fPitch ), (float)XMConvertToRadians( fYaw ), (float)XMConvertToRadians( fRoll ) );
	XMFLOAT4X4 f4x4Rotate;
	XMStoreFloat4x4( &f4x4Rotate, mtxRotate );

	m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( f4x4Rotate, m_mtxWorld );
}

void CGameObject::Rotate( XMFLOAT3 *pvAxis, float fAngle )
{
	XMVECTOR tmp = XMLoadFloat3( pvAxis );
	XMMATRIX mtxRotate = XMMatrixRotationAxis( tmp, (float)XMConvertToRadians(fAngle) );
	XMFLOAT4X4 f4x4Rotate;
	XMStoreFloat4x4( &f4x4Rotate, mtxRotate );

	m_mtxWorld = MathHelper::GetInstance( )->Float4x4MulFloat4x4( f4x4Rotate, m_mtxWorld );
}

void CGameObject::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	CShader::UpdateShaderVariable( pd3dDeviceContext, &m_mtxWorld );
	if (m_pMaterial)
		CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
	// 객체의 텍스처를 셰이더 변수에 연결
	if (m_pTexture)
		m_pTexture->UpdateShaderVariable( pd3dDeviceContext );

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
			{
				bool bIsVisible = true;
				if (pCamera)
				{
					AABB bcBoundingCube = m_ppMeshes[i]->GetBoundingCube( );
					bcBoundingCube.Update( &m_mtxWorld );
					bIsVisible = pCamera->IsInFrustum( &bcBoundingCube );
				}
				if (bIsVisible)
					m_ppMeshes[i]->Render( pd3dDeviceContext );
			}
		}
	}
}

CTexture::CTexture( int nTextures, int nSamplers, int nTextureStartSlot, int nSamplerStartSlot )
{
	m_nReferences = 0;

	m_nTextures = nTextures;
	m_ppd3dsrvTextures = new ID3D11ShaderResourceView*[m_nTextures];
	for (int i = 0; i < m_nTextures; i++)
		m_ppd3dsrvTextures[i] = NULL;

	m_nTextureStartSlot = nTextureStartSlot;
	m_nSamplers = nSamplers;
	m_ppd3dSamplerStates = new ID3D11SamplerState*[m_nSamplers];
	for (int i = 0; i < m_nSamplers; i++)
		m_ppd3dSamplerStates[i] = NULL;
	m_nSamplerStartSlot = nSamplerStartSlot;
}

CTexture::~CTexture( )
{
	for (int i = 0; i < m_nTextures; i++)
		if (m_ppd3dsrvTextures[i])
			m_ppd3dsrvTextures[i]->Release( );
	for (int i = 0; i < m_nSamplers; i++)
		if (m_ppd3dSamplerStates[i])
			m_ppd3dSamplerStates[i]->Release( );

	if (m_ppd3dsrvTextures) 
		delete[ ] m_ppd3dsrvTextures;
	if (m_ppd3dSamplerStates) 
		delete[ ] m_ppd3dSamplerStates;
}

void CTexture::SetTexture( int nIndex, ID3D11ShaderResourceView *pd3dsrvTexture )
{
	if (m_ppd3dsrvTextures[nIndex])
		m_ppd3dsrvTextures[nIndex]->Release( );
	m_ppd3dsrvTextures[nIndex] = pd3dsrvTexture;
	if (pd3dsrvTexture) pd3dsrvTexture->AddRef( );
}

void CTexture::SetSampler( int nIndex, ID3D11SamplerState *pd3dSamplerState )
{
	if (m_ppd3dSamplerStates[nIndex])
		m_ppd3dSamplerStates[nIndex]->Release( );
	m_ppd3dSamplerStates[nIndex] = pd3dSamplerState;
	if (pd3dSamplerState)
		pd3dSamplerState->AddRef( );
}

void CTexture::UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext )
{
	pd3dDeviceContext->PSSetShaderResources( m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures );
	pd3dDeviceContext->PSSetSamplers( m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates );
}

void CTexture::UpdateTextureShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, int nIndex, int nSlot )
{
	pd3dDeviceContext->PSSetShaderResources( nSlot, 1, &m_ppd3dsrvTextures[nIndex] );
}

void CTexture::UpdateSamplerShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, int nIndex, int nSlot )
{
	pd3dDeviceContext->PSSetSamplers( nSlot, 1, &m_ppd3dSamplerStates[nIndex] );
}


CSkyBox::CSkyBox( ID3D11Device *pd3dDevice ) : CGameObject( 1 )
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh( pd3dDevice, 20.0f, 20.0f, 20.0f );
	SetMesh( pSkyBoxMesh, 0 );
}

CSkyBox::~CSkyBox( )
{

}

void CSkyBox::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	XMFLOAT3 vCameraPos = pCamera->GetPosition( );
	SetPosition( vCameraPos );
	CShader::UpdateShaderVariable( pd3dDeviceContext, &m_mtxWorld );

	// 스카이박스 메시를 렌더링
	if (m_ppMeshes && m_ppMeshes[0])
		m_ppMeshes[0]->Render( pd3dDeviceContext );
}