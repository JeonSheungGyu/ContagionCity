#pragma once
#include "stdafx.h"

// state
struct ObjectStatus
{
	DWORD lv;		// 레벨
	int max_hp;
	int hp;		// 체력
	int max_ap;
	int ap;		// 행동력
	DWORD damage;	// 공격력
	DWORD defense;	// 방어력
	DWORD exp;		// 경험치
	DWORD requestEXP; // 필요한 경험치

	ObjectStatus() : lv(1), max_hp(100), hp(100), max_ap(100), ap(100), damage(10), defense(20), exp(0), requestEXP(100) {}
	void reset() {
		lv = 1;
		max_hp = 100;
		hp = 100;
		max_ap = 100;
		ap = 100;
		damage = 10;
		defense = 20;
		exp = 0;
		requestEXP = 100;
	}
	ObjectStatus& operator*(const WORD up)
	{
		max_hp *= up;
		hp = max_hp;
		max_ap *= up;
		ap = max_ap;
		damage *= up;
		defense *= up;
		requestEXP *= up;
		return *this;
	}

	ObjectStatus& operator*=(const WORD up)
	{
		max_hp *= up;
		hp = max_hp;
		max_ap *= up;
		ap = max_ap;
		damage *= up;
		defense *= up;
		requestEXP *= up;
		return *this;
	}
};