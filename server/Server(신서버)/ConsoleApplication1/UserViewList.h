#pragma once
#include "SyncObject.h"


class User;

typedef std::pair<unsigned int, bool> ViewData;

class UserViewList : public SyncObject
{
protected:
	// concurrent_vector는 erase 지원X 
	std::vector<ViewData> view;	// 이전 뷰 리스트
	std::vector<ViewData> tmpList; // view와 비교할 list

	User				 *owner;

public:
	UserViewList(User* owner);
	virtual					~UserViewList();

	bool					insertUserInTmp(const DWORD id);
	bool					eraseUserInTmp(const DWORD id);
	bool					insertUser(const DWORD id);
	bool					eraseUser(const DWORD id);

	void					updateViewList(const std::set<DWORD>& nearLIst);
	bool					isInViewList(const DWORD id);

	// 플레이어 및 몬스터 추가,삭제,이동
	void					putObject(const DWORD id);
	void					moveObject(const DWORD id);
	void					removeObject(const DWORD id);


	//몬스터

	void MonsterWander(const DWORD id);
	void MonsterChase(const DWORD id);
	void MonsterAttack(const DWORD id);
	void MonsterDie(const DWORD id);

	// List accessor
	std::vector<ViewData>&	getView() { return view; }
	std::vector<ViewData>&	getTmpList() { return tmpList; }
};