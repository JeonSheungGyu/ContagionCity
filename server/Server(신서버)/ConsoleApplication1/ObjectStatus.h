#pragma once
#include "stdafx.h"

// state
struct ObjectStatus
{
	DWORD lv;		// 레벨
	int hp;		// 체력
	int ap;		// 행동력
	DWORD damage;	// 공격력
	DWORD defense;	// 방어력
	DWORD exp;		// 경험치
	DWORD requestEXP; // 필요한 경험치

	ObjectStatus() : lv(1), hp(100), ap(100), damage(10), defense(20), exp(0), requestEXP(100) {}

	ObjectStatus& operator*(const WORD up)
	{
		hp *= up;
		ap *= up;
		damage *= up;
		defense *= up;
		requestEXP *= up;
		return *this;
	}

	ObjectStatus& operator*=(const WORD up)
	{
		hp *= up;
		ap *= up;
		damage *= up;
		defense *= up;
		requestEXP *= up;
		return *this;
	}
};