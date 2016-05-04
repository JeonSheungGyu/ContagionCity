#pragma once
#define GARBAGE -55555

class MathHelper
{
private:
	MathHelper( );

public:
	static MathHelper *Instance;
	static MathHelper *GetInstance( ) { if (Instance == NULL) Instance = new MathHelper; return Instance; }
	~MathHelper( );

	float DistanceVector3ToVector3( XMFLOAT3 v1, XMFLOAT3 v2 );
	float Vector3ToLength( XMVECTOR vec );
	float Vector3ToDot( XMVECTOR v1, XMVECTOR v2 );
	float Float3ToLength( XMFLOAT3 vec );
	float Float3ToDot( XMFLOAT3& v1, XMFLOAT3& v2 );
	XMFLOAT3 CrossFloat3( XMFLOAT3 vec1, XMFLOAT3 vec2 );
	XMFLOAT3 NormalizeFloat( XMFLOAT3& vec );
	XMFLOAT4 NormalizeFloat( XMFLOAT4& vec );

	XMFLOAT3 VectorToFloat3( XMVECTOR vec3 );
	XMVECTOR Float4ToVector( XMFLOAT4 vec );
	XMVECTOR Float4ToVector( XMVECTOR vec, float w );
	XMVECTOR Float4ToVector( float x, float y, float z, float w );
	XMVECTOR Float3ToVector( XMFLOAT3 vec );
	XMVECTOR Float3ToVector( float x, float y, float z );
	XMVECTOR Float2ToVector( float x, float y );
	XMFLOAT3 Float3PlusFloat3( XMFLOAT3& vec1, XMFLOAT3& vec2 );
	XMFLOAT3 Float3MinusFloat3( XMFLOAT3& vec1, XMFLOAT3& vec2 );
	XMFLOAT3 Float3MulFloat( XMFLOAT3& vec, float num );
	XMFLOAT4 MakeFloat4( XMFLOAT3 vec, float w );
	XMFLOAT4 MakeFloat4( float x, float y, float z, float w );

	// 행렬 변환
	XMFLOAT3 Vector3TransformNormal( XMFLOAT3 vec, XMFLOAT4X4 mtx );
	// 점 변환
	XMFLOAT3 Vector3TransformCoord( XMFLOAT3 vec, XMFLOAT4X4 mtx );
	
	XMFLOAT4X4 MatrixRotationAxis( XMFLOAT3 vec, float Angle );
	XMFLOAT4X4 MatrixLookAtLH( XMFLOAT3 EyePosition, XMFLOAT3 FocusPosition, XMFLOAT3 UpDirection );
	XMFLOAT4X4 GetMatrixIdentity( );
	XMFLOAT4X4 TransposeFloat4x4( XMFLOAT4X4 mtx );
	XMFLOAT4X4 Float4x4MulFloat4x4( XMFLOAT4X4& mtx1, XMFLOAT4X4& mtx2 );

	void ChangeVector3Data( XMVECTOR& vec, float x = GARBAGE, float = GARBAGE, float z = GARBAGE );
	float Clamp( float &n, float minN, float maxN );
	template<typename T>
	static T Min( const T& a, const T& b )
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max( const T& a, const T& b )
	{
		return a > b ? a : b;
	}
};