#pragma once
#include "Protocol.h"
#include "Object.h"
#include "UserViewList.h"
#include <atomic>
// �÷��̾ �� �� �ִ� �ൿ
enum
{
	action_move,
	action_combat,
	action_stop
};

typedef struct NetworkSession {
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	Overlap_ex recv_overlap;
	int	packet_size;
	int previous_size;
	unsigned char packet_buff[BUFSIZE];
	
	NetworkSession() {
		hClntSock = NULL;
		memset(&clntAddr, 0, sizeof(clntAddr));
		memset(packet_buff, 0, sizeof(packet_buff));
		recv_overlap.operation = OP_RECV;
		recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(recv_overlap.iocp_buffer);
		recv_overlap.wsabuf.len = sizeof(recv_overlap.iocp_buffer);
		packet_size = 0;
		previous_size = 0;
	}
	void reset() {
		hClntSock = NULL;
		memset(&clntAddr, 0, sizeof(clntAddr));
		memset(packet_buff, 0, sizeof(packet_buff));
		recv_overlap.operation = OP_RECV;
		recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(recv_overlap.iocp_buffer);
		recv_overlap.wsabuf.len = sizeof(recv_overlap.iocp_buffer);
		packet_size = 0;
		previous_size = 0;
	}
}NetworkSession;


class User : public Object
{
private:
	char userID[ID_LEN];
	bool blnConnected;
	NetworkSession session;
	UserViewList viewList;

	//�̵�, ����, ����
	BYTE			 action;
public:

	User() : viewList(this), Object () {
		blnConnected = false;
	}
	void reset() {
		Object::reset();
		session.reset();
		blnConnected = false;
		viewList.getView().clear();
	}
	NetworkSession& getSession() { return session; }
	UserViewList& getViewList() { return viewList; }

	//�������̵�
	void					setUserID(const char* buf) { memcpy(userID, buf, ID_LEN); }
	char*					getUserID() { return userID; }
	//�������ӿ���
	bool			isConnected() const { return blnConnected; }
	void			setConnected(const bool blnTemp) { blnConnected = blnTemp; }

	void updateViewList() {
		viewList.updateViewList(nearList);
	}

	//action
	void					setAction(const BYTE ac) { action = ac; }
	const BYTE				getAction()const { return action; }

	//����ġ �÷���
	void plusExp(const WORD exp)
	{
		obStatus.exp += exp;

		// ���� �������� �ʿ��� ����ġ ����
		if (obStatus.exp >= obStatus.requestEXP) {
			obStatus.lv += 1;
			obStatus.exp = 0;
			obStatus *= 2;
		}
	}
};
