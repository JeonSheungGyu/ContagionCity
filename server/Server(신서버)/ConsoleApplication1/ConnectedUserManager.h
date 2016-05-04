#pragma once

class CConnectedUserManager {
private:
	std::vector<User*>	mConnectedUserVector;
public:

	bool PushConnectedUser(User* user) {
		mConnectedUserVector.push_back(user);
		return true;
	}

	bool DeleteConnectedUser(User* user) {
		for (auto iter = mConnectedUserVector.begin(); iter != mConnectedUserVector.end(); ) {
			if ((*iter) == user) {
				delete (*iter);
				iter = mConnectedUserVector.erase(iter);
				return true;
			}
			else
				++iter;
		}
		return false;
	}

	int GetKey() {
		bool Check;

		for (int j = 0; j < MAX_USER; j++) {
			Check = false;
			for (DWORD i = 0; i < mConnectedUserVector.size(); i++)
			{
				if (mConnectedUserVector[i]->id == j)
					Check = TRUE;
			}
			if (!Check)
				return j;
		}
		return -1;
	}

	std::vector<User*>::iterator GetIterator() {
		return mConnectedUserVector.begin();
	}

	DWORD GetSize() {
		return mConnectedUserVector.size();
	}
	User* GetUser(DWORD id) {
		for (auto iter = mConnectedUserVector.begin(); iter != mConnectedUserVector.end(); iter++) {
			if ((*iter)->id == id) {
				return *iter;
			}
		}
		return nullptr;
	}

};