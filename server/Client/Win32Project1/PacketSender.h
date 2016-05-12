#pragma once
#include "stdafx.h"
#include <mutex>

// ������ ���� packetó�� �޼���
class PacketSender
{
public:
	static PacketSender& instance();
	void requestLogin(SOCKET sock, char *id, char *password);

	//void playerMove(const float x, const float y);

	//void positionUpdate();

	//void staticUpdate();

	//// �÷��̾ ��ų or �⺻���� �� ȣ��
	//// �Ű�����: �÷��̾ � Ű�� �������� action BYTE��,��ų����� x,y
	//void playerCombat(const char action, const float x, const float y);

private:
	PacketSender() {}
	virtual ~PacketSender();
	PacketSender(const PacketSender& copy) {}
	PacketSender& operator=(const PacketSender& copy) {}

	static PacketSender* sInstance;

	static std::mutex sMutex;
};