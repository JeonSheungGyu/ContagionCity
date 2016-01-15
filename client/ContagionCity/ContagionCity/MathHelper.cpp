#include "stdafx.h"
#include "MathHelper.h"


MathHelper::MathHelper( )
{
}


MathHelper::~MathHelper( )
{
}


float MathHelper::Vector3ToLength( XMVECTOR vec )
{
	XMFLOAT3 fLength;
	XMVECTOR fTemp = XMVector3Length( vec );
	XMStoreFloat3( &fLength, fTemp );

	return fLength.x;
}

float MathHelper::Vector3ToDot( XMVECTOR v1, XMVECTOR v2 )
{
	XMVECTOR vDot = XMVector3Dot( v1, v2 );
	XMFLOAT3 fDot = VectorToFloat3( vDot );
	
	return fDot.x;
}

float MathHelper::Float3ToLength( XMFLOAT3 vec )
{
	XMVECTOR tmp = XMLoadFloat3( &vec );
	tmp = XMVector3Length( tmp );

	XMFLOAT3 fLength;
	XMStoreFloat3( &fLength, tmp );

	return fLength.x;
}

float MathHelper::Float3ToDot( XMFLOAT3& v1, XMFLOAT3& v2 )
{
	XMVECTOR vec1 = XMLoadFloat3( &v1 );
	XMVECTOR vec2 = XMLoadFloat3( &v2 );

	XMVECTOR vDot = XMVector3Dot( vec1, vec2 );
	XMFLOAT3 fDot = VectorToFloat3( vDot );

	return fDot.x;
}

XMFLOAT3 MathHelper::VectorToFloat3( XMVECTOR vec3 )
{
	XMFLOAT3 tmp;

	XMStoreFloat3( &tmp, vec3 );

	return tmp;
}

XMVECTOR MathHelper::Float3ToVector( XMFLOAT3 vec )
{
	return XMLoadFloat3( &vec );
}

XMVECTOR MathHelper::Float3ToVector( float x, float y, float z )
{
	return XMLoadFloat3( &XMFLOAT3( x, y, z ) );
}

float MathHelper::Clamp( float &n, float minN, float maxN )
{
	if (n > maxN)
	{
		return maxN;
	}
	if (n < minN)
	{
		return minN;
	}

	return n;
}

void MathHelper::ChangeVector3Data( XMVECTOR& vec, float x, float y, float z )
{
	XMFLOAT3 tmp = VectorToFloat3( vec );

	if (x != GARBAGE)
	{
		tmp.x = x;
	}
	if (y != GARBAGE)
	{
		tmp.y = y;
	}
	if (z != GARBAGE)
	{
		tmp.z = z;
	}

	Float3ToVector( tmp );
}

XMVECTOR MathHelper::Float4ToVector( XMFLOAT4 vec )
{
	return XMLoadFloat4( &vec );
}

XMVECTOR MathHelper::Float4ToVector( float x, float y, float z, float w )
{
	return XMLoadFloat4( &XMFLOAT4( x, y, z, w ) );
}

XMVECTOR MathHelper::Float4ToVector( XMVECTOR vec, float w )
{
	XMFLOAT3 tmp;
	XMStoreFloat3( &tmp, vec );

	return XMLoadFloat4( &XMFLOAT4(tmp.x, tmp.y, tmp.z, w ) );
}

XMVECTOR MathHelper::Float2ToVector( float x, float y )
{
	return XMLoadFloat2( &XMFLOAT2( x, y ) );
}

XMFLOAT3 MathHelper::Float3PlusFloat3( XMFLOAT3& vec1, XMFLOAT3& vec2 )
{
	XMVECTOR temp1 = XMLoadFloat3( &vec1 );
	XMVECTOR temp2 = XMLoadFloat3( &vec2 );
	
	XMFLOAT3 tmp;
	XMStoreFloat3( &tmp, temp1 + temp2 );

	return tmp;
}

// vec1 - vec2 
XMFLOAT3 MathHelper::Float3MinusFloat3( XMFLOAT3& vec1, XMFLOAT3& vec2 )
{
	XMVECTOR temp1 = XMLoadFloat3( &vec1 );
	XMVECTOR temp2 = XMLoadFloat3( &vec2 );

	XMFLOAT3 tmp;
	XMStoreFloat3( &tmp, temp1 - temp2 );

	return tmp;
}

XMFLOAT3 MathHelper::Float3MulFloat( XMFLOAT3& vec, float num )
{
	XMVECTOR vTmp = XMLoadFloat3( &vec );
	vTmp *= num;

	XMFLOAT3 returnValue;
	XMStoreFloat3( &returnValue, vTmp );

	return returnValue;
}

XMFLOAT4X4 MathHelper::GetMatrixIdentity( )
{
	XMFLOAT4X4 iden;
	XMMATRIX tmp = XMMatrixIdentity( );
	XMStoreFloat4x4( &iden, tmp );

	return iden;
}

XMFLOAT3 MathHelper::NormalizeFloat( XMFLOAT3& vec )
{
	XMVECTOR tmp = XMLoadFloat3( &vec );	
	tmp = XMVector3Normalize( tmp );

	XMStoreFloat3( &vec, tmp );
	return vec;
}

XMFLOAT4 MathHelper::NormalizeFloat( XMFLOAT4& vec )
{
	XMVECTOR tmp = XMLoadFloat4( &vec );
	tmp = XMVector3Normalize( tmp );

	XMStoreFloat4( &vec, tmp );
	return vec;
}

XMFLOAT3 MathHelper::CrossFloat3( XMFLOAT3 vec1, XMFLOAT3 vec2 )
{
	XMVECTOR v1 = XMLoadFloat3( &vec1 );
	XMVECTOR v2 = XMLoadFloat3( &vec2 );

	XMVECTOR vec = XMVector3Cross( v1, v2 );
	XMFLOAT3 tmp;
	XMStoreFloat3( &tmp, vec );

	return tmp;
}

XMFLOAT4X4 MathHelper::TransposeFloat4x4( XMFLOAT4X4 mtx )
{
	XMMATRIX tmp = XMLoadFloat4x4( &mtx );
	
	tmp = XMMatrixTranspose( tmp );
	XMFLOAT4X4 returnMtx;
	XMStoreFloat4x4( &returnMtx, tmp );

	return returnMtx;
}

XMFLOAT4X4 MathHelper::MatrixRotationAxis( XMFLOAT3 vec, float Angle )
{
	XMMATRIX tmp = XMMatrixRotationAxis( MathHelper::GetInstance( )->Float3ToVector( vec ), (float)XMConvertToRadians( Angle ) );
	XMFLOAT4X4 returnValue;
	XMStoreFloat4x4( &returnValue, tmp );

	return returnValue;
}

XMFLOAT3 MathHelper::Vector3TransformNormal( XMFLOAT3 vec, XMFLOAT4X4 mtx )
{
	XMMATRIX tmpMtx = XMLoadFloat4x4( &mtx );
	return VectorToFloat3( XMVector3TransformNormal( Float3ToVector( vec ), tmpMtx ) );
}

XMFLOAT3 MathHelper::Vector3TransformCoord( XMFLOAT3 vec, XMFLOAT4X4 mtx )
{
	XMMATRIX tmpMtx = XMLoadFloat4x4( &mtx );
	return VectorToFloat3( XMVector3TransformCoord( Float3ToVector( vec ), tmpMtx ) );
}

XMFLOAT4X4 MathHelper::MatrixLookAtLH( XMFLOAT3 EyePosition, XMFLOAT3 FocusPosition, XMFLOAT3 UpDirection )
{
	XMVECTOR vEyePosition, vFocusPosition, vUpDirection;
	vEyePosition = XMLoadFloat3( &EyePosition );
	vFocusPosition = XMLoadFloat3( &FocusPosition );
	vUpDirection = XMLoadFloat3( &UpDirection );

	XMMATRIX tmp = XMMatrixLookAtLH( vEyePosition, vFocusPosition, vUpDirection );
	XMFLOAT4X4 returnValue;
	XMStoreFloat4x4( &returnValue, tmp );

	return returnValue;
}

XMFLOAT4X4 MathHelper::Float4x4MulFloat4x4( XMFLOAT4X4& mtx1, XMFLOAT4X4& mtx2 )
{
	XMMATRIX tmp1 = XMLoadFloat4x4( &mtx1 );
	XMMATRIX tmp2 = XMLoadFloat4x4( &mtx2 );

	XMMATRIX dest = tmp1 * tmp2;

	XMFLOAT4X4 returnValue;
	XMStoreFloat4x4( &returnValue, dest );

	return returnValue;
}

XMFLOAT4 MathHelper::MakeFloat4( XMFLOAT3 vec, float w )
{
	return MakeFloat4( vec.x, vec.y, vec.z, w );
}

XMFLOAT4 MathHelper::MakeFloat4( float x, float y, float z, float w )
{
	return XMFLOAT4( x, y, z, w );
}
