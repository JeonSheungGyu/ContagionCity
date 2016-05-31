#include "stdafx.h"
#include "Monster.h"
#include "User.h"
#include "PacketDispatcher.h"
#include "Server.h"

extern std::vector<User> users;
extern std::vector<Monster> monsters;

using namespace DirectX;
// �α��μ��� <-> Ŭ���̾�Ʈ
void PacketDispatcher::PermisionLogin(char* buf)
{
	lc_packet_permit_login packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *buf);
	
	Server::login_success = packet.permit_check;
}
// ���Ӽ��� <-> Ŭ���̾�Ʈ
void PacketDispatcher::Login(char* buf)
{
	sc_packet_login packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *(buf));

	Server::setClientID(packet.id);

	auto& user = users[packet.id];

	user.setID(packet.id);
	user.setPos(XMFLOAT2(packet.x, packet.z));
	user.setStatus(ObjectStatus(packet.lv, packet.hp, packet.ap, packet.damage, packet.defense, packet.exp, packet.request_exp));
	user.setSpeed(packet.speed);
	user.is_using = true;


	//��ġ������Ʈ
	Server::updateThread = new thread{ Server::UpdateThread };
}

// �÷��̾� or ���� �߰� ( �þ߹��� �� )
void PacketDispatcher::PutObject(char* buf)
{
	printf("Recv :SC_PUT_PLAYER \n");
	sc_packet_put_object packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *(buf));

	if (packet.id < MAX_USER)	// User
	{
		auto& user = users[packet.id];
		user.setID(packet.id);
		user.setPos(XMFLOAT2(packet.x, packet.z));
		user.setStatus(ObjectStatus(packet.lv,packet.hp, packet.ap, packet.damage, packet.defense, packet.exp,packet.request_exp));
		user.setSpeed(packet.speed);
		//user.setSpeed(240);
		user.setDir(XMFLOAT2(0,0));
		user.is_using = true;
	} 
	else  // Monster
	{
		auto& monster = monsters.at(packet.id - MAX_USER);
		monster.setID(packet.id);
		monster.setPos(XMFLOAT2(packet.x, packet.z));
		monster.setStatus(ObjectStatus(packet.lv, packet.hp, packet.ap, packet.damage, packet.defense, packet.exp, packet.request_exp));
		monster.setSpeed(packet.speed+10);
		monster.setDir(XMFLOAT2(0, 0));
		monster.is_using = true;
	}
}
// �÷��̾� or ���� ���� ( �þ߹��� �� )
void PacketDispatcher::RemoveObject(char* buf)
{
	printf("Recv :SC_REMOVE_PLAYER \n");
	sc_packet_remove_object packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *(buf));

	if (packet.id < MAX_USER) users[packet.id].is_using = false;
	else monsters[packet.id - MAX_USER].is_using = false;
}

void PacketDispatcher::MoveObject(char* buf)
{
	sc_packet_move_object packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *buf);

	//if (packet.tf == 1){
		if (packet.id < MAX_USER){
			auto& user = users[packet.id];
			// ���⺤��
			XMVECTOR dir = XMVector2Normalize(XMLoadFloat2(&XMFLOAT2(
				packet.tx - user.getPos().x,
				packet.tz - user.getPos().y)));

			// ��ǥ�� ���� �÷��̾� ��ġ���� �Ÿ�
			float dist = sqrt((packet.tx - user.getPos().x)*
				(packet.tx - user.getPos().x) +
				(packet.tz - user.getPos().y)*
				(packet.tz - user.getPos().y));

			// �÷��̾� ��ġ�� ���⺤�� ����
			user.setDir(XMFLOAT2(XMVectorGetX(dir), XMVectorGetY(dir)));
			user.setDist(dist);
			user.is_move = true;
		} else{

			auto& monster = monsters.at(packet.id - MAX_USER);
			// ���⺤��
			XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(
				packet.tx - monster.getPos().x,
				packet.ty - 0,
				packet.tz - monster.getPos().y)));

			// ��ǥ�� ���� �÷��̾� ��ġ���� �Ÿ�
			float dist = sqrt((packet.tx - monster.getPos().x)*
				(packet.tx - monster.getPos().x) +
				(packet.tz - monster.getPos().y)*
				(packet.tz - monster.getPos().y));

			// �÷��̾� ��ġ�� ���⺤�� ����
			monster.setDir(XMFLOAT2(XMVectorGetX(dir), XMVectorGetZ(dir)));
			monster.setDist(dist);

			monster.is_move = true;
		}
	//}
	//else if (packet.tf == 2)
	//{
		// A* ����ؼ� �����ſ� �°� �߰�
	//}
	//else // �� ���� ��
	//{

	//}
}

void PacketDispatcher::ObjectCombat(char* buf)
{
	sc_packet_combat packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *buf);

	// packet.id�� packet.action�� ���� �ִϸ��̼� ��ȭ

	// player -> monster ����
	for (int i = 0; i < packet.ListSize; ++i){
		monsters[packet.InfoList[i].first - MAX_USER].setHp(packet.InfoList[i].second);
	}
}

void PacketDispatcher::MonsterAttack(char* buf)
{
	sc_packet_monster_attack packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *buf);

	// packet.id�� �ִϸ��̼� 

	// monster -> player ����
	//if (users[packet.target_id].getHp() > 0 )
	users[packet.target_id].minusHp(packet.damage);
	monsters[packet.mon_id - MAX_USER].setServerPos(XMFLOAT2(packet.x, packet.z));
	
	//cout << "[���� -> �÷��̾� ����] Player ID: " << packet.target_id
	//	<< " HP: " << GameEngine::getUser()[packet.target_id].getHp() << endl;
}

void PacketDispatcher::MonsterChase(char* buf)
{
	//cout << "MonsterChase" << endl;
	sc_packet_monster_chase packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *buf);

	auto& monster = monsters.at(packet.id - MAX_USER);

	// ���⺤��
	XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(
		packet.tx - monster.getPos().x,
		packet.ty - 0,
		packet.tz - monster.getPos().y)));

	// ��ǥ�� ���� �÷��̾� ��ġ���� �Ÿ�
	float dist = sqrt((packet.tx - monster.getPos().x)*
		(packet.tx - monster.getPos().x) +
		(packet.tz - monster.getPos().y)*
		(packet.tz - monster.getPos().y));

	// �÷��̾� ��ġ�� ���⺤�� ����
	monster.setDir(XMFLOAT2(XMVectorGetX(dir), XMVectorGetZ(dir)));
	monster.setDist(dist);
	monster.setTargetPos(XMFLOAT2(packet.tx, packet.tz));
	monster.setServerPos(XMFLOAT2(packet.x, packet.z));
	monster.is_move = true;
}

void PacketDispatcher::MonsterDie(char* buf)
{
	sc_packet_monster_die packet;
	memcpy(reinterpret_cast<char*>(&packet), buf, *buf);
	monsters[packet.mon_id - MAX_USER].is_using = false;
	users[packet.player_id].plusEXP(packet.EXP);
}


