#pragma once


//ĳ���� ���� ����
enum _EQUIP_INDEX{
	E_HEAD = 0, // �Ӹ�
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
	TCHAR mName[32]; // ĳ���� �̸�
	INT64 mIndex; // ĳ���� ���� ��ȣ
	INT mJob; // ĳ���� ���� ( )
	INT mType;
	INT64 mExp; // ����ġ
	INT mEquips[E_END]; // ��� �迭
public:
	//ĳ���͸� �ʱ�ȭ�ϴ� �Լ�
	VOID Initialize(VOID);

	//������ �ʵ带 �а� ���� �Լ���
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