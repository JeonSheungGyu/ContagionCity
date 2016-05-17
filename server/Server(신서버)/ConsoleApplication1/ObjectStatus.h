#pragma once
#include "stdafx.h"

// state
struct ObjectStatus
{
	DWORD lv;		// ����
	int hp;		// ü��
	int ap;		// �ൿ��
	DWORD damage;	// ���ݷ�
	DWORD defense;	// ����
	DWORD exp;		// ����ġ
	DWORD requestEXP; // �ʿ��� ����ġ

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