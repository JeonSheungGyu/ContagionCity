#pragma once

#include "SyncObject.h"
#include "Sector.h"

class Object : public SyncObject
{
public:
	DWORD id;
	DWORD x, y;
	Sector *sector;
	std::set<Sector*> nearSectors;
	std::set<DWORD> nearList;


	Object() {
		sector = nullptr;
	}
	void updateNearList();
};
