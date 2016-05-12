#pragma once
#include "stdafx.h"
#include <mutex>

// 서버로 보낼 packet처리 메서드
class PacketSender
{
public:
	static PacketSender& instance();
	void requestLogin(SOCKET sock, char *id, char *password);

	//void playerMove(const float x, const float y);

	//void positionUpdate();

	//void staticUpdate();

	//// 플레이어가 스킬 or 기본공격 시 호출
	//// 매개변수: 플레이어가 어떤 키를 눌렀는지 action BYTE값,스킬사용한 x,y
	//void playerCombat(const char action, const float x, const float y);

private:
	PacketSender() {}
	virtual ~PacketSender();
	PacketSender(const PacketSender& copy) {}
	PacketSender& operator=(const PacketSender& copy) {}

	static PacketSender* sInstance;

	static std::mutex sMutex;
};