#include "PacketDispatcher.h"
#include "Protocol.h"
#include "Login.h"

void PacketDispatcher_DB_Server::PermisionLogin(char*buf,const unsigned short id)
{
	dl_packet_permit_login rPacket;
	lc_packet_permit_login sPacket;

	memcpy(reinterpret_cast<char*>(&rPacket), buf, *buf);


	sPacket.permit_check = rPacket.check;
	sPacket.type = LC_PERMISION_LOGIN;
	sPacket.size = sizeof(sPacket);

	Login::sendPacket(rPacket.login_id, &sPacket);
}



//로그인정보 받기
void PacketDispatcher_Client::RequestLogin(char*buf,const unsigned short id)
{
	cl_packet_request_login rPacket;

	memcpy(reinterpret_cast<char*>(&rPacket), buf, *buf);

	cout << "id: " << rPacket.id << endl;
	cout << "password: " << rPacket.password << endl;

	DB_QUERY q;
	q.accountid = id;
	strcpy_s(q.id, rPacket.id);
	strcpy_s(q.pw, rPacket.password);
	q.type = ID_PW_CHECK;


	//DB.큐에 푸시푸시
	EnterCriticalSection(&Login::dbCS);
	Login::getDBQ().push(q);
	LeaveCriticalSection(&Login::dbCS);
}