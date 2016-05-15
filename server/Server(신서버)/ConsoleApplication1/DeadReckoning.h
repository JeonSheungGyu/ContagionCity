#pragma once
#include "stdafx.h"
#include <atomic>
#include <mutex>

class DeadReckoning
{
private:
	std::vector<WORD>							ObjectList;
	std::atomic<WORD>							is_exist[MAX_USER];

	static DeadReckoning*						dInstance;
	static std::mutex							dMutex;


	DeadReckoning();
	DeadReckoning(const DeadReckoning& copy) = delete;
	DeadReckoning(const DeadReckoning&& copy) = delete;
	DeadReckoning& operator=(const DeadReckoning& other) = delete;
	DeadReckoning& operator=(const DeadReckoning&& other) = delete;

public:
	static DeadReckoning&						Instance();

	void										setList(const WORD id);
	void										Execute();

	// �̷��� �ȵ�.. ��������
	// ���� DeadReckoning����� ���� ���� �ð����� ���ϱ� ���� Ÿ������Ʈ 
	std::chrono::system_clock::time_point			preTimePoint;
	std::atomic<bool>								is_process;
};