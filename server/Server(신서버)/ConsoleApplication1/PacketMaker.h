#pragma once
#include <mutex>
#include "Object.h"

// 패킷 생성과정이 코드마다 분산되어 있으면 찾기도 힘들고 가독성 떨어짐
// 패킷 생성하는 부분 클래스로 캡슐화

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