#pragma once
#pragma warning(disable : 4244)

class Zone
{
	Sector innerSector[WORLDSIZE / SECTOR_HEIGHT][WORLDSIZE / SECTOR_WIDTH];	// 월드섹터 저장

public:
	void SectorUpdateOfPlayer(const unsigned int id);
	void SectorUpdateOfMonster(const unsigned int id);
	// get nearList
	// 어떤 오브젝트가 매개변수로 오던지 Zone에서 Sector를 구할 수 있도록 template 사용
	template<class object_type>
	Sector* getSectorWithPlayer(const object_type* player)
	{
		return getSectorWithPoint(player->x, player->y);
	}

	Sector* getSectorWithPoint( float x,  float y)
	{

		if (y < 0) y = 0;
		if (y >= WORLDSIZE) y = WORLDSIZE - 1;
		if (x < 0) x = 0;
		if (x >= WORLDSIZE) x = WORLDSIZE - 1;
	
		return &innerSector[(int)(y / SECTOR_HEIGHT)][(int)(x / SECTOR_WIDTH)];
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