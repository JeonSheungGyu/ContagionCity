#include "stdafx.h"
#include "Object.h"
#include "Character.h"

CCharacter::CCharacter(VOID) : Object()
{
	mIndex = 0;
	mJob = 0;
	mType = 0;
	mExp = 0;
	mHP = 0;
	mAP = 0;
	mGameMoney = 0;
	mLevel = 1;
	mDefense = 20;
	mAttack = 10;
	mRequestEXP = 100;

	memset(mEquips, 0, sizeof(mEquips));
	memset(mName, 0, sizeof(mName));
}

CCharacter::~CCharacter(VOID)
{
}

VOID CCharacter::Initialize(VOID)
{
	mIndex = 0;
	mJob = 0;
	mType = 0;
	mExp = 0;
	mHP = 0;
	mAP = 0;
	mGameMoney = 0;
	mLevel = 1;
	mDefense = 20;
	mAttack = 10;
	mRequestEXP = 100;

	memset(mEquips, 0, sizeof(mEquips));
	memset(mName, 0, sizeof(mName));
}