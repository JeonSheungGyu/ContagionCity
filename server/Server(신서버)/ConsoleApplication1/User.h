#pragma once

#include "Object.h"
#include "UserViewList.h"
#include <atomic>
// 플레이어가 할 수 있는 행동
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
	//계정 아이디
	char userID[ID_LEN];

	bool blnConnected;
	NetworkSession session;
	UserViewList viewList;

	//이동, 공격, 정지
	BYTE			 action;

	//파티
	WORD PartyNumber;
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

	//userID
	void					setUserID(const char *ui) { memcpy(userID, ui, ID_LEN); }
	const char*				getUserID()const { return userID; }

	//유저접속여부
	bool			isConnected() const { return blnConnected; }
	void			setConnected(const bool blnTemp) { blnConnected = blnTemp; }
	//뷰리스트 업데이트
	void updateViewList() {
		viewList.updateViewList(nearList);
	}
	//action
	void					setAction(const BYTE ac) { action = ac; }
	const BYTE				getAction()const { return action; }
	// 소속 파티번호 accessor
	const WORD					getPartyNum()const { return PartyNumber; }
	void						setPartyNum(const WORD num) { PartyNumber = num; }


	//경험치 플러스
	void plusExp(const WORD exp)
	{
		obStatus.exp += exp;

		// 다음 레벨까지 필요한 경험치 충족
		if (obStatus.exp >= obStatus.requestEXP) {
			obStatus.lv += 1;
			obStatus.exp = 0;
			obStatus *= 2;
		}
	}
};
