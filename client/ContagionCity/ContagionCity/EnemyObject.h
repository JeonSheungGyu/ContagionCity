#pragma once
class CEnemyObject : public AnimatedObjectInfo
{
public:
	CEnemyObject( CFbxMesh vertex, int nMeshes );
	~CEnemyObject( );

	void Move( DWORD dwDirection, float fDistance, bool bUpdateVelocity );
	void Move( XMFLOAT3& vShift, bool bVelocity = false );
	void Rotate( float x, float y, float z );
	virtual void Animate( float fTimeElapsed );

	float m_fSpeed;
	XMFLOAT3 m_vVelocity;
	float m_fMaxVelocityXZ;
	float m_fMaxVelocityY;
	float m_fFriction;
};

