#pragma once


//캐릭터 장착 정의
enum _EQUIP_INDEX{
	E_HEAD = 0, // 머리
	E_CHEST,
	E_PANTS,
	E_GLOVES,
	E_SOCKS,
	E_SHOES,
	E_ACCESSORY,
	E_SKILL1,
	E_SKILL2,
	E_SKILL3,
	E_END
};


class CCharacter{
public:
	CCharacter(VOID);
	virtual ~CCharacter(VOID);
private:
	TCHAR mName[32]; // 캐릭터 이름
	INT64 mIndex; // 캐릭터 고유 번호
	INT mJob; // 캐릭터 직업 ( )
	INT mType;
	INT64 mExp; // 경험치
	INT mEquips[E_END]; // 장비 배열
public:
	//캐릭터를 초기화하는 함수
	VOID Initialize(VOID);

	//각각의 필드를 읽고 쓰는 함수들
	inline VOID SetIndex(INT64 index){ mIndex = index; }
	inline INT64 GetIndex(VOID){ return mIndex; }
	inline VOID SetJob(INT job) { mJob = job; }
	inline INT GetJob(VOID){ return mJob; }
	inline VOID SetExp(INT64 exp){ mExp = exp; }
	inline INT64 GetExp(VOID){ return mExp; }
	inline VOID SetEquip(USHORT equipLocation, INT type) { mEquips[equipLocation] = type; }
	inline INT GetEquip(USHORT equipLocation){ return mEquips[equipLocation]; }
	inline BOOL SetName(LPTSTR name) { if (!name)return FALSE; _tcsncpy(mName, name, 32); return TRUE; }
	inline LPTSTR GetName(VOID){ return mName; }
	inline VOID SetType(INT type) { mType = type; }
	inline INT GetType(VOID){ return mType; }
};