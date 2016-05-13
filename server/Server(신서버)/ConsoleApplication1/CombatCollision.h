#pragma once
#include "stdafx.h"
const int COLLISION_FUNC_TYPE = 4;

enum
{
	CC_CircleAround = 0,
	CC_CircleFront,
	CC_Eraser,
	CC_PointCircle,
};

// �Լ������� ����ü
struct CollisionFuncArray
{
	using FuncType = void(*)(const WORD, std::vector< std::pair<WORD, WORD>>&, const FLOAT, const FLOAT);
	FuncType Func;
	CollisionFuncArray() { Func = nullptr; }
};

class CombatCollision
{
public:
	// ĳ���� �ֺ�
	static void CircleAround(const WORD id, std::vector< std::pair<WORD, WORD>>& InfoList, const FLOAT x, const FLOAT z);

	// ĳ���� ��
	static void CircleFront(const WORD id, std::vector< std::pair<WORD, WORD>>& InfoList, const FLOAT x, const FLOAT z);

	// ������
	static void Eraser(const WORD id, std::vector< std::pair<WORD, WORD>>& InfoList, const FLOAT x, const FLOAT z);

	// ������ ����Ʈ
	static void PointCircle(const WORD id, std::vector< std::pair<WORD, WORD>>& InfoList, const FLOAT x, const FLOAT z);

	//RayCast
	static BOOL RayCast(const XMFLOAT3& Pos, const XMFLOAT3& Dir, FLOAT dist, const BoundingSphere object)
	{
		FXMVECTOR pos(XMLoadFloat3(&Pos));
		FXMVECTOR dir(XMLoadFloat3(&Dir));

		return object.Intersects(pos, dir, dist);
	}

};