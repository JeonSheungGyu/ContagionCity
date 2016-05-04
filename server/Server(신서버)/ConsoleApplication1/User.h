#pragma once

#include "Object.h"
#include "UserViewList.h"


class User : public Object //소켓정보를구조체화.
{
public:
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	bool isConnected;
	Overlap_ex recv_overlap;
	int	packet_size;
	int previous_size;
	unsigned char packet_buff[BUFSIZE];
	UserViewList viewList;

	User() : viewList(this) {
		x = 0, y = 0;
		recv_overlap.operation = OP_RECV;
		recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(recv_overlap.iocp_buffer);
		recv_overlap.wsabuf.len = sizeof(recv_overlap.iocp_buffer);
		isConnected = false;
	}
	void updateViewList() {
		viewList.updateViewList(nearList);
	}
};
