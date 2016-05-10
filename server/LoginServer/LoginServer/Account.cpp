#include "Account.h"

Account::Account() : sock(0),recvQue(this), is_using(false)
{
	ZeroMemory(id, sizeof(id));
	ZeroMemory(password, sizeof(password));
}

Account::~Account() { }