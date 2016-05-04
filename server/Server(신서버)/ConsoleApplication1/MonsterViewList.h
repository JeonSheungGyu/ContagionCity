#pragma once
#pragma once
#include "SyncObject.h"


class Monster;

typedef std::pair<unsigned int, bool> ViewData;

class MonsterViewList : public SyncObject
{
protected:
	// concurrent_vector�� erase ����X 
	std::vector<ViewData> view;	// ���� �� ����Ʈ
	std::vector<ViewData> tmpList; // view�� ���� list

	Monster				 *owner;

public:
	MonsterViewList(Monster* owner);
	virtual					~MonsterViewList();

	bool					insertUserInTmp(const DWORD id);
	bool					eraseUserInTmp(const DWORD id);

	void					updateViewList(const std::set<DWORD>& nearLIst);
	bool					isInViewList(const DWORD id);
	// List accessor
	std::vector<ViewData>&	getView() { return view; }
	std::vector<ViewData>&	getTmpList() { return tmpList; }
};