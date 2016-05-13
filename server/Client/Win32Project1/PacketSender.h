#pragma once
#include "stdafx.h"
#include <mutex>

// 서버로 보낼 packet처리 메서드
class PacketSender
{
public:
	static PacketSender& instance();
	void requestLogin(char *id, char *password);
	void PlayerMove(BYTE dir);
	void PlayerCombat(const char combatCollision, const float x, const float y);
	//void positionUpdate();

	//void staticUpdate();

	

private:
	PacketSender() {}
	virtual ~PacketSender();
	PacketSender(const PacketSender& copy) {}
	PacketSender& operator=(const PacketSender& copy) {}

	static PacketSender* sInstance;

	static std::mutex sMutex;
};