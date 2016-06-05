#pragma once
#pragma warning(disable : 4244)
#include "Object.h"
class Zone
{
	Sector **innerSector;// ���弽�� ����
	int m_width;
	int m_height;
	XMFLOAT3 m_startPos; // ZONE ���� ������ġ
public:
	int getWidth() { return m_width; }
	int getHeight() { return m_height; }
	XMFLOAT3 getStartPos() { return m_startPos; }
	void SectorUpdateOfPlayer(const unsigned id);
	void SectorUpdateOfMonster(const unsigned id);

	Zone(XMFLOAT3 startPos, int width, int height) : m_startPos(startPos), m_width(width), m_height(height){
		//������ �����Ҵ�
		innerSector = new Sector*[height / SECTOR_HEIGHT];
		for (int i = 0; i < height / SECTOR_HEIGHT; i++) {
			innerSector[i] = new Sector[width / SECTOR_WIDTH];
		}
	}

	~Zone() {
		//����������
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
		x = x - m_startPos.x;
		z = z - m_startPos.z;

		if (z < 0) z = 0;
		if (z >= m_height) z = m_height - 1;
		if (x < 0) x = 0;
		if (x >= m_width) x = m_width - 1;

		//printf("%f %f\n", x ,z);
		return &innerSector[(int)(z / SECTOR_HEIGHT)][(int)(x / SECTOR_WIDTH)];
	}


	static void insertNearSector( Sector* sector, std::set<Sector*>&nearSector)
	{
		try{
			//set�̱⶧���� �ߺ���� �ȵȴ�.
				nearSector.insert(sector);
		}catch (std::exception& e) {
			printf("Object::updateNearList %s\n", e.what());
			return;
		}

	}
};