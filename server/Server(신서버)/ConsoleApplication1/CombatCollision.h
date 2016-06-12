#pragma once
#include "stdafx.h"
const int COLLISION_FUNC_TYPE = 5;

enum
{
	CC_CircleAround = 0,
	CC_CircleFront,
	CC_Eraser,
	CC_PointCircle,
	ETC_CheckUser
};

// 함수포인터 구조체
struct CollisionFuncArray
{
	using FuncType = void(*)(const WORD, std::vector< std::pair<WORD, int>>&, const FLOAT, const FLOAT);
	FuncType Func;
	CollisionFuncArray() { Func = nullptr; }
};

class CombatCollision
{
public:
	// 캐릭터 주변
	static void CircleAround(const WORD id, std::vector< std::pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z);

	// 캐릭터 앞
	static void CircleFront(const WORD id, std::vector< std::pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z);

	// 일직선
	static void Eraser(const WORD id, std::vector< std::pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z);

	// 지정된 포인트
	static void PointCircle(const WORD id, std::vector< std::pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z);

	// 일직선
	static void CheckUser(const WORD id, std::vector< std::pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z);
	//RayCast
	static BOOL RayCast(const XMFLOAT3& Pos, const XMFLOAT3& Dir, FLOAT dist, const BoundingSphere object)
	{
		FXMVECTOR pos(XMLoadFloat3(&Pos));
		FXMVECTOR dir(XMLoadFloat3(&Dir));

		return object.Intersects(pos, dir, dist);
	}

};