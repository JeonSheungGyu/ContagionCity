#pragma once

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
class CCharacter
{
public:
	CCharacter(VOID);
	virtual ~CCharacter(VOID);

private:
	//ĳ�����̸�
	TCHAR			mName[32];
	//�ε���?
	INT64			mIndex;
	//����
	INT				mJob;
	//Ÿ�� ���� ����
	INT				mType;
	//����ġ
	INT64			mExp;
	//���ӸӴ�
	INT64			mGameMoney;
	//���
	//��� �������ο� ���� �׼� �� ��ų ��� ����
	INT				mEquips[E_END];

	// ĳ���� ����
	DWORD				mHP;
	DWORD				mAP;
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

	inline BOOL			InitializeForGameStart(VOID){ mHP = DEFAULT_MAX_HP; mAP = DEFAULT_MAX_AP;  return TRUE; }
	inline DWORD		GetHP(VOID){return mHP; }
	inline DWORD		GetAP(VOID){ return mAP; }
	inline BOOL			SetHP(DWORD hp){ mHP = hp; return TRUE; }
	inline BOOL			SetAP(DWORD ap){ mAP = ap; return TRUE; }
};
