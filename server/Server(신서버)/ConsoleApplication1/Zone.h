#pragma once
#pragma warning(disable : 4244)
#include "Object.h"
class Zone
{
	Sector innerSector[WORLDSIZE / SECTOR_HEIGHT][WORLDSIZE / SECTOR_WIDTH];	// 월드섹터 저장

public:
	void SectorUpdateOfPlayer(const unsigned int id);
	void SectorUpdateOfMonster(const unsigned int id);


	Sector* getSectorWithPlayer(const Object* object)
	{
		return getSectorWithPoint(object->getPos().x, object->getPos().z);
	}

	Sector* getSectorWithPoint( float x,  float z)
	{

		if (z < 0) z = 0;
		if (z >= WORLDSIZE) z = WORLDSIZE - 1;
		if (x < 0) x = 0;
		if (x >= WORLDSIZE) x = WORLDSIZE - 1;
	
		return &innerSector[(int)(z / SECTOR_HEIGHT)][(int)(x / SECTOR_WIDTH)];
	}


	static void insertNearSector( Sector* sector, std::set<Sector*>&nearSector)
	{
		try{
			//set이기때문에 중복허용 안된다.
				nearSector.insert(sector);
		}catch (std::exception& e) {
			printf("Object::updateNearList %s\n", e.what());
			return;
		}

	}


	Zone()
	{}
	~Zone() {}
};