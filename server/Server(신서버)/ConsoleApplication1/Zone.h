#pragma once
#pragma warning(disable : 4244)
#include "Object.h"
class Zone
{
	Sector **innerSector;	// 월드섹터 저장
	int m_width;
	int m_height;
public:
	int getWidth() { return m_width; }
	int getHeight() { return m_height; }
	void SectorUpdateOfPlayer(const unsigned id);
	void SectorUpdateOfMonster(const unsigned id);

	Zone(int width, int height) {
		m_width = width;
		m_height = height;
		//존영역 동적할당
		innerSector = new Sector*[height / SECTOR_HEIGHT];
		for (int i = 0; i < height / SECTOR_HEIGHT; i++) {
			innerSector[i] = new Sector[width / SECTOR_WIDTH];
		}
	}

	~Zone() {
		//존영역해제
		for (int i = 0; i < m_height / SECTOR_HEIGHT; i++) {
			delete[] innerSector[i];
		}
		delete[] innerSector;
	}

	Sector* getSectorWithPlayer(const Object* object)
	{
		return getSectorWithPoint(object->getPos().x, object->getPos().z);
	}

	Sector* getSectorWithPoint( float x,  float z)
	{

		if (z < 0) z = 0;
		if (z >= m_height) z = m_height - 1;
		if (x < 0) x = 0;
		if (x >= m_width) x = m_width - 1;
	
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
};