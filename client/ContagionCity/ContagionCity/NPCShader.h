#pragma once

class CNPCShader : public CAnimatedObjShader
{
public:
	CNPCShader( );
	~CNPCShader( );

	virtual void BuildObjects( ID3D11Device *pd3dDevice, std::vector<CFbxMesh> meshes );
	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera = NULL );
};

