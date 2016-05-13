#pragma once
#include "SyncObject.h"


class User;

typedef std::pair<unsigned int, bool> ViewData;

class UserViewList : public SyncObject
{
protected:
	// concurrent_vector�� erase ����X 
	std::vector<ViewData> view;	// ���� �� ����Ʈ
	std::vector<ViewData> tmpList; // view�� ���� list

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

	// �÷��̾� �� ���� �߰�,�̵�,����,����(����)
	void					putObject(const DWORD id);
	void					moveObject(const DWORD id);
	void					removeObject(const DWORD id);
	void					combatObject(const DWORD id);

	// List accessor
	std::vector<ViewData>&	getView() { return view; }
	std::vector<ViewData>&	getTmpList() { return tmpList; }
};