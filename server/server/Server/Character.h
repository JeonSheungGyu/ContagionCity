#pragma once


#include "Object.h"

enum _EQUIP_INDEX
{
	E_HEAD = 0,
	E_CHEST,
	E_PANTS,
	E_GLOVES,
	E_SOCKS,
	E_SHOES,
	E_ACCESSORY,
	E_WEAPON1,
	E_WEAPON2,
	E_END
};

extern DWORD	DEFAULT_MAX_HP;
extern DWORD	DEFAULT_MAX_AP;
class CCharacter : public Object
{
public:
	CCharacter(VOID);
	virtual ~CCharacter(VOID);

private:
	//캐릭터이름
	TCHAR			mName[32];
	//인덱스?
	INT64			mIndex;
	//직업
	INT				mJob;
	//타입 아직 미정
	INT				mType;
	//게임머니
	INT64			mGameMoney;

	////////// 캐릭터 정보
	DWORD mLevel;
	DWORD mHP;
	DWORD mAP;
	DWORD mAttack;
	DWORD mDefense;

	//필요한 경험치
	DWORD mRequestEXP;
	DWORD mExp;

	//장비
	//장비 장착여부에 따른 액션 및 스킬 사용 분할
	INT				mEquips[E_END];
	//아직 모르겠음
	BYTE ElementType;


public:
	VOID			Initialize(VOID);

	inline VOID		SetIndex(INT64 index){ mIndex = index; }
	inline INT64	GetIndex(VOID){ return mIndex; }
	inline VOID		SetJob(INT job){ mJob = job; }
	inline INT		GetJob(VOID){ return mJob; }
	inline VOID		SetType(INT type){ mType = type; }
	inline INT		GetType(VOID){ return mType; }
	inline VOID		SetExp(INT64 exp){ mExp = exp; }
	inline INT64	GetExp(VOID){ return mExp; }
	inline VOID		SetEquip(USHORT equipLocation, INT type){ mEquips[equipLocation] = type; }
	inline INT		GetEquip(USHORT equipLocation){ return mEquips[equipLocation]; }
	inline BOOL		SetName(LPTSTR name){ if (!name)return FALSE; _tcsncpy(mName, name, 32); return TRUE; }
	inline LPTSTR	GetName(VOID){ return mName; }
	inline INT64	GetGameMoney(VOID){ return mGameMoney; }
	inline VOID		SetGameMoney(INT64 gameMoney){ mGameMoney = gameMoney; }

	inline BOOL		InitializeForGameStart(VOID){ mHP = DEFAULT_MAX_HP; mAP = DEFAULT_MAX_AP;  return TRUE; }
	inline DWORD	GetHP(VOID){return mHP; }
	inline DWORD	GetAP(VOID){ return mAP; }
	inline BOOL		SetHP(DWORD hp){ mHP = hp; return TRUE; }
	inline BOOL		SetAP(DWORD ap){ mAP = ap; return TRUE; }

	inline VOID		LevelUp(DWORD up){
		mHP *= up;
		mAP *= up;
		mDefense *= up;
		mAttack *= up;
		mRequestEXP *= up;
	}

	inline bool minusAP(const int ap)
	{
		mAP -= ap;
		return 0;
	}
	inline bool minusHP(const int hp)
	{
		mHP -= hp;
		return 0;
	}
	inline void plusHP(const DWORD hp) { mHP += hp; }
};
