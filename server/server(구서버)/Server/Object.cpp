#include "stdafx.h"


//#include "Zone.h"
//#include "Server.h"
//#include "protocol.h"
//#include "Sector.h"
//#include "ViewList.h"
//#include <WinSock.h>
#include "Object.h"


Object::Object() //: currentSector(nullptr), id(0), speed(120), viewList(this)
{
	//this->overlapped.is_send = false;
}


//Object::Object(const XMFLOAT3& pos, const FLOAT speed) : currentSector(nullptr), id(0), speed(speed), obVector(pos), prePos(pos), viewList(this), sock(0), targetPos(0, 0, 0), action(0)
//{
//	this->overlapped.is_send = false;
//	nearList.reserve(1000);
//}
//
//
//void Object::updateNearList()
//{
//	Zone* zone = Server::getZone();
//
//	RECT rt;
//	nearSectors.clear();
//	// 현재 시야 사각형의 네 꼭짓점을 구한다
//	rt.left = getPos().x - VIEW_RANGE;
//	rt.right = getPos().x + VIEW_RANGE + 1;
//	rt.top = getPos().z - VIEW_RANGE;
//	rt.bottom = getPos().z + VIEW_RANGE + 1;
//
//	correctRect(rt); // 좌표 경계검사
//
//	Zone::insertNearSector(zone->getSectorWithPoint(rt.left, rt.top), nearSectors);
//	Zone::insertNearSector(zone->getSectorWithPoint(rt.right, rt.top), nearSectors);
//	Zone::insertNearSector(zone->getSectorWithPoint(rt.left, rt.bottom), nearSectors);
//	Zone::insertNearSector(zone->getSectorWithPoint(rt.right, rt.bottom), nearSectors);
//
//	// nearSector에 있는 유저들 중 시야 내에 있는 유저만 nearList에 추가
//	nearList.clear();
//
//	for (auto&sector : nearSectors)
//	{
//		if (!sector) continue;
//
//		EnterCriticalSection(&sector->sCS);
//		auto vec = sector->getPlayers();
//		LeaveCriticalSection(&sector->sCS);
//
//		for (auto&id : vec)
//		{
//			if (id == this->id) continue;
//
//			try{
//				if (id < MAX_USER)	// ID가 유저ID
//				{
//					if (Sector::isinViewRadius(this->getPos(), Server::getPlayers().at(id)->getPos())){
//						nearList.push_back(id);
//					}
//				}
//				else if (id >= MAX_USER){
//					if (Sector::isinViewRadius(this->getPos(), Server::getMonsters().at(id - MAX_USER)->getPos()))
//					{
//						if (Server::getMonsters().at(id - MAX_USER)->is_alive == true)
//							nearList.push_back(id);
//					}
//				}
//			}
//			catch (exception& e){
//				cout << "Object::updateNearList " << e.what() << endl;
//				continue;
//			}
//		}
//	}
//}
//
//
////경계 체크
//void Object::correctRect(RECT& rt)
//{
//	//// left check
//	//if (rt.left<0) rt.left = 0;
//	//if ((rt.top<400 && rt.left<400) || (rt.top>800 && rt.left<400)) rt.left = 400;
//
//	//// right check
//	//if (rt.right>=WORLD_WIDTH) rt.right = WORLD_WIDTH-1;
//	//if ((rt.top<400 && rt.right>800) || (rt.bottom>800 && rt.right>800)) rt.right = 800 - 1;
//
//	//// top check
//	//if (rt.top<0) rt.top = 0;
//	//if ((rt.left < 400 && rt.top < 400) || (rt.right>800 && rt.top<400)) rt.top = 400;
//
//	//// bottom check
//	//if (rt.bottom>=WORLD_HEIGHT) rt.bottom = WORLD_HEIGHT-1;
//	//if ((rt.left<400 && rt.bottom>800) || (rt.right>800 && rt.bottom>800)) rt.bottom = 800 - 1;
//
//
//	// [TEST] 실제 게임에 적용할 땐 위에 코드로 변경
//	// 800x800용 경계검사
//
//	if (rt.left < 0) rt.left = 0;
//	if (rt.right > WORLD_WIDTH) rt.right = WORLD_WIDTH - 1;
//	if (rt.top < 0) rt.top = 0;
//	if (rt.bottom > WORLD_HEIGHT) rt.bottom = WORLD_HEIGHT - 1;
//
//
//	//// 중간발표용 임시맵 2칸만 사용
//	//if (rt.left < 400) rt.left = 400 - 1;
//	//if (rt.right > 800) rt.right = 800 - 1;
//	//if (rt.top < 400) rt.top = 400 - 1;
//	//if (rt.bottom > 1200) rt.bottom = 1200 - 1;
//}
//
///*
//	새로운 지점 도착할때까지 계산을 하지않고,
//	도착하면 새로운지점의 좌표를 전송한다.
//	아마도 이것이 데드레커닝인듯?
//*/
//
///*
//void Object::ObjectDeadReckoning(const float elapsed)
//{
//	NewSpeed = speed*(elapsed / 1000.0);
//	obVector.position.x += obVector.direction.x*NewSpeed;
//	obVector.position.z += obVector.direction.z*NewSpeed;
//	obVector.dist -= NewSpeed;
//	//obVector.position.y += dir.y*speed;
//	//cout << "obVector.dist: " << obVector.dist << endl;
//
//	if (obVector.dist <= 0)
//		is_DeadReckoning = false;
//}
//*/
Object::~Object() {}