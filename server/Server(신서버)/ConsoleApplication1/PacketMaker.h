#pragma once
#include <mutex>
#include "Object.h"

// ��Ŷ ���������� �ڵ帶�� �л�Ǿ� ������ ã�⵵ ����� ������ ������
// ��Ŷ �����ϴ� �κ� Ŭ������ ĸ��ȭ

class PacketMaker
{
public:
	static PacketMaker& instance();

	void PutObject(Object *owner, const DWORD id);

	void RemoveObject(Object *owner, const DWORD id);

	void MoveObject(Object *owner, const DWORD id);



private:
	static PacketMaker* pInstance;
	static std::mutex pMutex;

	PacketMaker() {}
	PacketMaker(const PacketMaker& copy) = delete;
	PacketMaker(const PacketMaker&& copy) = delete;
	PacketMaker& operator=(const PacketMaker& copy) = delete;
	PacketMaker& operator=(const PacketMaker&& copy) = delete;
	virtual ~PacketMaker();
};